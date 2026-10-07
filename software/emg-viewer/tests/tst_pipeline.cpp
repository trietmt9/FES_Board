// End-to-end plumbing: source -> filters -> envelope bank -> rings.
//
// tst_emgfilter proves ChannelFilter's envelope responds to a contraction.
// That is not the same as proving the *pipeline* delivers it: the source has to
// call filterBlock(), the bank has to be wired, and the value has to survive the
// hop to the reader. Nothing covered that, and it is exactly where a regression
// hid.

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#include "core/SampleRing.h"
#include "io/ReplaySource.h"

extern "C" {
#include "emg_frame.h"
}

#include <QTimer>

#include <cmath>

using namespace emg;

namespace {

constexpr int kRate = 4000;
constexpr int kNCh = 4;
constexpr int kNSamp = 32;
constexpr double kPi = 3.14159265358979323846;
constexpr double kUvPerCode = 2400000.0 / 6.0 / 8388608.0;

// A capture that looks like the real thing: big DC offset, slow drift, and
// 300 ms bursts of 120 Hz "muscle" separated by 300 ms of quiet.
QByteArray synthesise(int seconds)
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
    std::memcpy(info.fw_version, "pipe", 4);

    int n = emg_frame_build_info(frame.data(), frame.size(), &info);
    out.append(reinterpret_cast<const char *>(frame.data()), n);

    const long blocks = long(seconds) * kRate / kNSamp;
    std::uint32_t seq = 0;
    long idx = 0;
    unsigned state = 4242u;

    for (long b = 0; b < blocks; ++b) {
        for (int s = 0; s < kNSamp; ++s) {
            const double t = double(idx + s) / kRate;
            const bool bursting = std::fmod(t, 0.6) < 0.3;

            for (int c = 0; c < kNCh; ++c) {
                state = state * 1103515245u + 12345u;
                const double noise = double((state >> 16) & 0x7FFF) / 32768.0 - 0.5;

                const double uv =
                    4000.0                                        // electrode offset
                    + 600.0 * std::sin(2.0 * kPi * 0.3 * t)       // drift
                    + (bursting ? 300.0 * noise *
                                      std::sin(2.0 * kPi * 120.0 * t) : 0.0);

                block[c * kNSamp + s] = std::int32_t(uv / kUvPerCode);
            }
        }
        idx += kNSamp;

        const auto tMs = std::uint32_t(b * kNSamp * 1000 / kRate);
        n = emg_frame_build_data(frame.data(), frame.size(), seq++, tMs, 0,
                                 block.data(), kNSamp, kNCh, kNSamp,
                                 EMG_CH_MASK_CONTIGUOUS);
        out.append(reinterpret_cast<const char *>(frame.data()), n);
    }
    return out;
}

/// One channel, non-contiguous ch_mask, a pure sine. This is the configuration
/// the firmware actually streams (CH2 alone, mask 0x02) after EMG_ELECTRODES=3,
/// and a sine is the signal a simulator produces in its function-generator mode.
/// Everything else in this file uses 4 contiguous channels.
QByteArray synthesiseSine(int seconds, double freqHz, double ampUv,
                          std::uint8_t chMask)
{
    QByteArray out;
    std::vector<std::uint8_t> frame(EMG_FRAME_MAX_SIZE);
    std::vector<std::int32_t> block(kNSamp);

    emg_info info{};
    info.sample_rate_hz = kRate;
    info.vref_uv = 2400000;
    info.gain = 6;
    info.n_ch_active = 1;
    int n = emg_frame_build_info(frame.data(), frame.size(), &info);
    out.append(reinterpret_cast<const char *>(frame.data()), n);

    const int blocks = seconds * kRate / kNSamp;
    std::uint32_t seq = 0;
    std::size_t idx = 0;

    for (int b = 0; b < blocks; ++b) {
        for (int s = 0; s < kNSamp; ++s) {
            const double t = double(idx + s) / kRate;
            const double uv = ampUv * std::sin(2.0 * kPi * freqHz * t);
            block[s] = std::int32_t(uv / kUvPerCode);
        }
        idx += kNSamp;

        const auto tMs = std::uint32_t(b * kNSamp * 1000 / kRate);
        n = emg_frame_build_data(frame.data(), frame.size(), seq++, tMs, 0,
                                 block.data(), kNSamp, 1, kNSamp, chMask);
        out.append(reinterpret_cast<const char *>(frame.data()), n);
    }
    return out;
}

} // namespace

class TstPipeline : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void envelopeReachesTheBank();
    void envelopeReachesTheBank_data();
    void filteredRingDiffersFromRaw();
    void snapshotSurvivesOngoingWrites();
    void singleMaskedChannelCarriesACleanSine();

private:
    QString writeCapture(const QString &name, int seconds);
    QTemporaryDir m_dir;
};

void TstPipeline::initTestCase()
{
    QVERIFY(m_dir.isValid());
    qRegisterMetaType<emg::InfoFrame>("emg::InfoFrame");
    qRegisterMetaType<emg::ParserStats>("emg::ParserStats");
}

void TstPipeline::singleMaskedChannelCarriesACleanSine()
{
    // Reproduces the bench setup exactly: one channel, ch_mask 0x02 (CH2), a
    // pure sine. If the samples arrive reordered, duplicated or interleaved
    // wrongly, a sine is where it shows - it comes back noisy, and its measured
    // frequency is wrong. A 1 Hz square wave would hide all of that.
    constexpr double kFreq = 10.0;
    constexpr double kAmp = 1000.0;   // 1 mV

    const QString path = QDir(m_dir.path()).filePath(QStringLiteral("sine.emgraw"));
    {
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(synthesiseSine(6, kFreq, kAmp, EMG_CH_MASK_BIT(2)));
    }

    SampleRing raw;
    raw.configure(1, 10 * kRate);

    ReplaySource src;
    src.setRing(&raw);
    src.setFilePath(path);
    src.setSpeed(20.0);

    QEventLoop loop;
    connect(&src, &ISampleSource::stopped, &loop, &QEventLoop::quit);
    QTimer::singleShot(30000, &loop, &QEventLoop::quit);
    src.start();
    loop.exec();
    src.stop();

    std::vector<float> win(4 * std::size_t(kRate));
    const std::size_t have = raw.readLatest(0, win.data(), win.size());
    QVERIFY2(have > std::size_t(2 * kRate),
             qPrintable(QStringLiteral("only %1 samples arrived").arg(have)));
    win.resize(have);

    // 1. Is it actually a sine? Correlate against the ideal at the known
    //    frequency, sweeping phase. Reordering destroys this even when the
    //    amplitude looks right.
    double best = 0.0;
    for (int p = 0; p < 64; ++p) {
        const double phase = 2.0 * kPi * p / 64.0;
        double num = 0.0, da = 0.0, db = 0.0;
        for (std::size_t i = 0; i < win.size(); ++i) {
            const double t = double(i) / kRate;
            const double ref = std::sin(2.0 * kPi * kFreq * t + phase);
            num += win[i] * ref;
            da += double(win[i]) * win[i];
            db += ref * ref;
        }
        if (da > 0 && db > 0) {
            best = std::max(best, num / std::sqrt(da * db));
        }
    }
    QVERIFY2(best > 0.99,
             qPrintable(QStringLiteral("sine correlation only %1 - samples are "
                                       "not arriving in order").arg(best)));

    // 2. Amplitude preserved.
    const auto [lo, hi] = std::minmax_element(win.begin(), win.end());
    const double p2p = *hi - *lo;
    QVERIFY2(p2p > 1.8 * kAmp && p2p < 2.2 * kAmp,
             qPrintable(QStringLiteral("peak-to-peak %1 uV, expected ~%2")
                            .arg(p2p).arg(2 * kAmp)));

    // 3. Frequency: count zero crossings. Wrong by the same ratio as any
    //    timebase error, which is what makes a sine the better test signal.
    std::size_t crossings = 0;
    for (std::size_t i = 1; i < win.size(); ++i) {
        if ((win[i - 1] < 0.0f) != (win[i] < 0.0f)) {
            ++crossings;
        }
    }
    const double measured = crossings * kRate / (2.0 * win.size());
    QVERIFY2(measured > kFreq * 0.95 && measured < kFreq * 1.05,
             qPrintable(QStringLiteral("measured %1 Hz, generated %2 Hz")
                            .arg(measured).arg(kFreq)));
}

QString TstPipeline::writeCapture(const QString &name, int seconds)
{
    const QString path = QDir(m_dir.path()).filePath(name);
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        return {};
    }
    f.write(synthesise(seconds));
    f.close();
    return path;
}

void TstPipeline::envelopeReachesTheBank_data()
{
    QTest::addColumn<bool>("displayFiltered");

    // The envelope must behave identically either way: `enabled` selects only
    // what the display shows. Raw display is the default, and is the case that
    // regressed.
    QTest::newRow("display raw") << false;
    QTest::newRow("display filtered") << true;
}

void TstPipeline::envelopeReachesTheBank()
{
    QFETCH(bool, displayFiltered);

    const QString path = writeCapture(QStringLiteral("pipe.emgraw"), 6);
    QVERIFY(!path.isEmpty());

    SampleRing raw;
    SampleRing filtered;
    raw.configure(kNCh, 10 * kRate);
    filtered.configure(kNCh, 10 * kRate);

    ISampleSource::EnvelopeBank bank{};
    for (auto &v : bank) {
        v.store(0.0f);
    }

    FilterConfig cfg;
    cfg.enabled = displayFiltered;

    ReplaySource src;
    src.setRing(&raw);
    src.setFilteredRing(&filtered);
    src.setEnvelopes(&bank);
    src.applyFilterConfig(cfg);
    src.setFilePath(path);
    src.setSpeed(20.0);

    // Sample the bank densely. NOT off statsUpdated: that is throttled to 1 Hz
    // of wall clock, and at 20x playback it fires once or twice for the whole
    // capture - which is how an earlier version of this test "measured" a flat
    // envelope that was in fact swinging 10:1.
    double lo = 1e9, hi = -1e9;
    QTimer sampler;
    sampler.setInterval(1);
    connect(&sampler, &QTimer::timeout, this, [&] {
        const double v = bank[0].load();
        if (v > 0.0) {
            lo = std::min(lo, v);
            hi = std::max(hi, v);
        }
    });
    sampler.start();

    QSignalSpy finished(&src, &ReplaySource::finished);
    src.start();
    QVERIFY(finished.wait(60000));
    sampler.stop();

    QVERIFY2(hi > 0.0, "envelope never left zero - the bank is not being written");

    // Bursts are ~300 uV peak of shaped noise; quiet periods are pure drift,
    // which the chain removes. So the envelope must swing by a wide margin.
    QVERIFY2(hi > 20.0, qPrintable(QStringLiteral("peak envelope only %1 uV").arg(hi)));
    QVERIFY2(hi > 5.0 * std::max(lo, 0.01),
             qPrintable(QStringLiteral("envelope barely moved: %1 -> %2 uV").arg(lo).arg(hi)));
}

void TstPipeline::filteredRingDiffersFromRaw()
{
    const QString path = writeCapture(QStringLiteral("rings.emgraw"), 4);
    QVERIFY(!path.isEmpty());

    SampleRing raw;
    SampleRing filtered;
    raw.configure(kNCh, 10 * kRate);
    filtered.configure(kNCh, 10 * kRate);

    ISampleSource::EnvelopeBank bank{};
    for (auto &v : bank) {
        v.store(0.0f);
    }

    ReplaySource src;
    src.setRing(&raw);
    src.setFilteredRing(&filtered);
    src.setEnvelopes(&bank);
    // Default config = display raw. Both rings are still filled, and the
    // filtered one must still be filtered - that is the bug this catches.
    src.applyFilterConfig(FilterConfig{});
    src.setFilePath(path);
    src.setSpeed(20.0);

    QSignalSpy finished(&src, &ReplaySource::finished);
    src.start();
    QVERIFY(finished.wait(60000));

    QCOMPARE(raw.written(), filtered.written());
    QVERIFY(raw.written() > 0);

    // Both rings must be populated, and the filtered one must have had the
    // 4 mV offset removed while the raw one keeps it.
    std::vector<float> rawWin(2000), filtWin(2000);
    const std::size_t nr = raw.readLatest(0, rawWin.data(), rawWin.size());
    const std::size_t nf = filtered.readLatest(0, filtWin.data(), filtWin.size());
    QCOMPARE(nr, rawWin.size());
    QCOMPARE(nf, filtWin.size());

    auto mean = [](const std::vector<float> &v, std::size_t n) {
        double s = 0.0;
        for (std::size_t i = 0; i < n; ++i) {
            s += v[i];
        }
        return s / double(n);
    };

    QVERIFY2(mean(rawWin, nr) > 2000.0,
             qPrintable(QStringLiteral("raw mean %1 uV, expected ~4000").arg(mean(rawWin, nr))));
    // The intent is "the 4 mV offset is gone", so the bound is a few percent of
    // that - not zero. The capture also carries 600 uV of 0.3 Hz drift, which
    // sits BELOW the 0.5 Hz corner: a 4th-order Butterworth leaves
    //   600 uV / sqrt(1 + (0.5/0.3)^8) = 77 uV
    // of it, and a 0.5 s window (this one) of a 77 uV, 0.3 Hz sine can average
    // up to about that. Measured here: -59 uV, matching an independent NumPy
    // run of the same filter (-59.1 uV).
    //
    // This used to assert < 50 uV and passed by coincidence: the filter started
    // from zero state, so the 4 mV step's leftover transient (+20 uV at t=3.5 s)
    // happened to cancel part of the drift remnant (-24 uV net). ChannelFilter
    // now primes on its first sample, which removes the transient and leaves
    // only the physically correct remnant. 100 uV is 2.5 % of the offset.
    QVERIFY2(std::abs(mean(filtWin, nf)) < 100.0,
             qPrintable(QStringLiteral("filtered mean %1 uV, expected ~0").arg(mean(filtWin, nf))));
}

void TstPipeline::snapshotSurvivesOngoingWrites()
{
    // The guarantee the display freeze rests on. Remembering a read position
    // would not be enough: the producer keeps running while frozen and, within
    // one ring capacity, would overwrite the very samples being looked at.
    SampleRing live;
    live.configure(2, 1000);

    std::vector<float> block(2 * 100);
    auto fill = [&](float value) {
        for (auto &v : block) {
            v = value;
        }
        live.write(block.data(), 2, 100);
    };

    for (int i = 0; i < 5; ++i) {
        fill(10.0f);
    }

    SampleRing frozen;
    live.snapshotInto(frozen);

    const quint64 frozenWritten = frozen.written();
    std::vector<float> before(200);
    const std::size_t n = frozen.readLatest(0, before.data(), before.size());
    QCOMPARE(n, before.size());

    // Keep writing, enough to lap the ring twice over.
    for (int i = 0; i < 25; ++i) {
        fill(99.0f);
    }

    // The live ring moved on...
    QVERIFY(live.written() > frozenWritten);
    std::vector<float> liveWin(200);
    live.readLatest(0, liveWin.data(), liveWin.size());
    QCOMPARE(liveWin.front(), 99.0f);

    // ...and the snapshot did not.
    QCOMPARE(frozen.written(), frozenWritten);
    std::vector<float> after(200);
    QCOMPARE(frozen.readLatest(0, after.data(), after.size()), after.size());
    QCOMPARE(after, before);
    QCOMPARE(after.front(), 10.0f);
}

QTEST_MAIN(TstPipeline)
#include "tst_pipeline.moc"
