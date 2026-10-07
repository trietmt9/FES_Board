// End-to-end test of the replay path: file -> FrameParser -> SampleRing.
//
// This is the same decode path the live serial link uses (ReplaySource and
// SerialSource share one FrameParser), so it doubles as a regression test for
// live acquisition without needing a board.

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#include "core/SampleRing.h"
#include "io/Recorder.h"
#include "io/ReplaySource.h"

extern "C" {
#include "emg_frame.h"
}

using namespace emg;

namespace {

constexpr int kRate = 1000;
constexpr int kNCh = 4;
constexpr int kNSamp = 32;
constexpr double kUvPerCode = 2400000.0 / 6.0 / 8388608.0;

// Build a capture in memory, mirroring what tools/emg_gen.c writes but with
// values the test can predict exactly.
QByteArray synthesise(int blocks, int dropEvery = 0, int corruptEvery = 0)
{
    QByteArray out;
    std::vector<std::uint8_t> frame(EMG_FRAME_MAX_SIZE);
    std::vector<std::int32_t> block(kNCh * kNSamp);

    emg_info info{};
    info.sample_rate_hz = kRate;
    info.vref_uv = 2400000;
    info.gain = 6;
    info.chip_id = 0x92;
    info.n_ch_active = kNCh;
    info.hr_mode = 1;
    std::memcpy(info.fw_version, "test", 4);

    int n = emg_frame_build_info(frame.data(), frame.size(), &info);
    out.append(reinterpret_cast<const char *>(frame.data()), n);

    std::uint32_t seq = 0;
    long sampleIndex = 0;

    for (int b = 0; b < blocks; ++b) {
        for (int c = 0; c < kNCh; ++c) {
            for (int s = 0; s < kNSamp; ++s) {
                // Distinct per channel, and small enough to stay in 24 bits.
                block[c * kNSamp + s] =
                    static_cast<std::int32_t>((c + 1) * 10000 + ((sampleIndex + s) % 997));
            }
        }
        sampleIndex += kNSamp;

        const auto tMs = static_cast<std::uint32_t>(b * kNSamp * 1000 / kRate);

        if (dropEvery > 0 && (b % dropEvery) == dropEvery - 1) {
            ++seq; // burn the sequence number without emitting the frame
            continue;
        }

        n = emg_frame_build_data(frame.data(), frame.size(), seq++, tMs, 0,
                                 block.data(), kNSamp, kNCh, kNSamp,
                                 EMG_CH_MASK_CONTIGUOUS);
        if (corruptEvery > 0 && (b % corruptEvery) == corruptEvery - 1) {
            frame[n / 2] ^= 0x01;
        }
        out.append(reinterpret_cast<const char *>(frame.data()), n);
    }

    return out;
}

QString writeCapture(const QDir &dir, const QString &name, const QByteArray &bytes)
{
    const QString path = dir.filePath(name);
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        return {};
    }
    f.write(bytes);
    f.close();
    return path;
}

} // namespace

class TstReplay : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void replaysWholeCaptureWithoutLoss();
    void reportsInjectedDrops();
    void rejectsInjectedCorruption();
    void recorderRoundTripsToCsv();

private:
    QTemporaryDir m_dir;
};

void TstReplay::initTestCase()
{
    QVERIFY(m_dir.isValid());
    qRegisterMetaType<emg::InfoFrame>("emg::InfoFrame");
    qRegisterMetaType<emg::ParserStats>("emg::ParserStats");
}

void TstReplay::replaysWholeCaptureWithoutLoss()
{
    // 60 s at 1000 SPS x 4 ch - the throughput case from ARCHITECTURE.md 7.
    constexpr int blocks = 60 * kRate / kNSamp;
    const QString path =
        writeCapture(QDir(m_dir.path()), QStringLiteral("clean.emgraw"),
                     synthesise(blocks));
    QVERIFY(!path.isEmpty());

    SampleRing ring;
    ring.configure(kNCh, 10 * kRate);

    ReplaySource src;
    src.setRing(&ring);
    src.setFilePath(path);
    src.setSpeed(20.0); // fast-forward; pacing itself is not what we test here

    QSignalSpy finished(&src, &ReplaySource::finished);
    QSignalSpy info(&src, &ISampleSource::infoReceived);

    ParserStats final;
    connect(&src, &ISampleSource::statsUpdated, this,
            [&final](const ParserStats &s, quint64) { final = s; });

    src.start();
    QVERIFY(finished.wait(60000));

    QCOMPARE(info.count(), 1);

    // Every block landed: no corruption, no gaps, no resyncs.
    QCOMPARE(final.crcErrors, quint64(0));
    QCOMPARE(final.droppedFrames, quint64(0));
    QCOMPARE(final.resyncs, quint64(0));
    QCOMPARE(final.dataFrames, quint64(blocks));
    QCOMPARE(ring.written(), quint64(blocks) * kNSamp);
}

void TstReplay::reportsInjectedDrops()
{
    constexpr int blocks = 500;
    constexpr int dropEvery = 50;

    const QString path =
        writeCapture(QDir(m_dir.path()), QStringLiteral("dropped.emgraw"),
                     synthesise(blocks, dropEvery));
    QVERIFY(!path.isEmpty());

    SampleRing ring;
    ring.configure(kNCh, 10 * kRate);

    ReplaySource src;
    src.setRing(&ring);
    src.setFilePath(path);
    src.setSpeed(20.0);

    quint64 dropped = 0;
    connect(&src, &ISampleSource::statsUpdated, this,
            [&dropped](const ParserStats &s, quint64) { dropped = s.droppedFrames; });

    QSignalSpy finished(&src, &ReplaySource::finished);
    src.start();
    QVERIFY(finished.wait(30000));

    // Drops land on b % 50 == 49, so the final block (b = 499) is dropped too -
    // and that one is invisible: a seq gap only becomes observable when a later
    // frame arrives to reveal it. Trailing loss is indistinguishable from the
    // stream simply ending, on this protocol and on any like it.
    constexpr int injected = blocks / dropEvery;
    QCOMPARE(dropped, quint64(injected - 1));
}

void TstReplay::rejectsInjectedCorruption()
{
    constexpr int blocks = 500;
    constexpr int corruptEvery = 100;

    const QString path =
        writeCapture(QDir(m_dir.path()), QStringLiteral("corrupt.emgraw"),
                     synthesise(blocks, 0, corruptEvery));
    QVERIFY(!path.isEmpty());

    SampleRing ring;
    ring.configure(kNCh, 10 * kRate);

    ReplaySource src;
    src.setRing(&ring);
    src.setFilePath(path);
    src.setSpeed(20.0);

    quint64 crcErrors = 0;
    connect(&src, &ISampleSource::statsUpdated, this,
            [&crcErrors](const ParserStats &s, quint64) { crcErrors = s.crcErrors; });

    QSignalSpy finished(&src, &ReplaySource::finished);
    src.start();
    QVERIFY(finished.wait(30000));

    QCOMPARE(crcErrors, quint64(blocks / corruptEvery));

    // The corrupt blocks are dropped, the rest still arrive.
    const auto expected = quint64(blocks - blocks / corruptEvery) * kNSamp;
    QCOMPARE(ring.written(), expected);
}

void TstReplay::recorderRoundTripsToCsv()
{
    constexpr int blocks = 10;
    const QByteArray capture = synthesise(blocks);

    const QString base = QDir(m_dir.path()).filePath(QStringLiteral("rec"));

    Recorder rec;
    rec.startRecording(base, true, true);
    rec.write(capture);
    rec.stopRecording();

    // .emgraw must be byte-identical to what went in.
    QFile raw(base + QStringLiteral(".emgraw"));
    QVERIFY(raw.open(QIODevice::ReadOnly));
    QCOMPARE(raw.readAll(), capture);
    raw.close();

    QFile csv(base + QStringLiteral(".csv"));
    QVERIFY(csv.open(QIODevice::ReadOnly | QIODevice::Text));
    const QList<QByteArray> lines = csv.readAll().split('\n');
    csv.close();

    QCOMPARE(lines.first(), QByteArray("t_s,ch1_uv,ch2_uv,ch3_uv,ch4_uv"));

    // header + blocks*kNSamp rows + trailing empty element after the last \n
    QCOMPARE(lines.size(), 1 + blocks * kNSamp + 1);

    // First data row: t=0, and codes (c+1)*10000 scaled to microvolts.
    const QList<QByteArray> first = lines.at(1).split(',');
    QCOMPARE(first.size(), 5);
    QCOMPARE(first.at(0), QByteArray("0.000000"));
    for (int c = 0; c < kNCh; ++c) {
        const double want = (c + 1) * 10000 * kUvPerCode;
        QVERIFY(qAbs(first.at(c + 1).toDouble() - want) < 0.01);
    }
}

QTEST_MAIN(TstReplay)
#include "tst_replay.moc"
