// Cross-validation of the two halves of the EMG wire protocol.
//
// This test links firmware/proto/src/emg_frame.c directly and decodes its
// output with the host FrameParser. That is the whole point: a CRC or
// endianness disagreement between firmware and host is otherwise invisible
// until you are staring at corrupt data from real hardware.

#include <QTest>

#include "core/Crc16.h"
#include "core/FrameParser.h"

extern "C" {
#include "emg_frame.h"
}

using namespace emg;

// Every constant duplicated in EmgProtocol.h, checked against the firmware
// header at compile time. If someone edits one side only, this fails to build.
static_assert(kMagic0 == EMG_FRAME_MAGIC0);
static_assert(kMagic1 == EMG_FRAME_MAGIC1);
static_assert(kVersion == EMG_FRAME_VERSION);
static_assert(static_cast<std::uint8_t>(FrameType::Data) == EMG_FRAME_TYPE_DATA);
static_assert(static_cast<std::uint8_t>(FrameType::Info) == EMG_FRAME_TYPE_INFO);
static_assert(static_cast<std::uint8_t>(FrameType::Text) == EMG_FRAME_TYPE_TEXT);
static_assert(kHeaderSize == EMG_FRAME_HEADER_SIZE);
static_assert(kCrcSize == EMG_FRAME_CRC_SIZE);
static_assert(kOverhead == EMG_FRAME_OVERHEAD);
static_assert(kMaxChannels == EMG_MAX_CHANNELS);
static_assert(kMaxSamples == EMG_MAX_SAMPLES);
static_assert(kMaxPayload == EMG_MAX_PAYLOAD);
static_assert(kDataHeaderSize == EMG_DATA_HDR_SIZE);
static_assert(kBytesPerSample == EMG_BYTES_PER_SAMPLE);
static_assert(kFlagOverflow == EMG_FLAG_OVERFLOW);
static_assert(kFlagLeadOff == EMG_FLAG_LEADOFF);
static_assert(kInfoPayloadSize == EMG_INFO_PAYLOAD_SIZE);
static_assert(kFwVersionLen == EMG_FW_VERSION_LEN);
static_assert(kMaxFrameSize == EMG_FRAME_MAX_SIZE);

namespace {

constexpr int kNCh = 4;
constexpr int kNSamp = 32;
constexpr std::size_t kStride = kNSamp;

// Build a DATA frame with the firmware encoder. block is channel-major.
QByteArray buildData(std::uint32_t seq, std::uint32_t tMs, std::uint8_t flags,
                     const std::vector<std::int32_t> &block, int nCh = kNCh,
                     int nSamp = kNSamp, std::size_t stride = kStride,
                     std::uint8_t chMask = EMG_CH_MASK_CONTIGUOUS)
{
    std::vector<std::uint8_t> out(EMG_FRAME_MAX_SIZE);
    const int n = emg_frame_build_data(out.data(), out.size(), seq, tMs, flags,
                                       block.data(), stride,
                                       static_cast<std::uint8_t>(nCh),
                                       static_cast<std::uint8_t>(nSamp),
                                       chMask);
    if (n < 0) {
        return {};
    }
    return QByteArray(reinterpret_cast<const char *>(out.data()), n);
}

// A recognisable ramp: channel c, sample s -> a value unique to that cell.
std::vector<std::int32_t> makeBlock(int nCh = kNCh, int nSamp = kNSamp,
                                    std::size_t stride = kStride)
{
    std::vector<std::int32_t> block(static_cast<std::size_t>(nCh) * stride, 0);
    for (int c = 0; c < nCh; ++c) {
        for (int s = 0; s < nSamp; ++s) {
            block[static_cast<std::size_t>(c) * stride + s] =
                (c + 1) * 1000 + s - 16;
        }
    }
    return block;
}

QByteArray buildInfo(std::uint32_t uptime = 42)
{
    emg_info info{};
    info.sample_rate_hz = 1000;
    info.vref_uv = 2400000;
    info.gain = 6;
    info.chip_id = 0x92;
    info.n_ch_active = 4;
    info.hr_mode = 1;
    info.uptime_s = uptime;
    std::memcpy(info.fw_version, "abc1234", 7);

    std::vector<std::uint8_t> out(EMG_INFO_FRAME_SIZE);
    const int n = emg_frame_build_info(out.data(), out.size(), &info);
    return QByteArray(reinterpret_cast<const char *>(out.data()), n);
}

// Collects everything a parser emits, so tests can assert on it afterwards.
struct Sink {
    std::vector<DataFrame> data;
    std::vector<InfoFrame> info;
    QStringList text;

    void attach(FrameParser &p)
    {
        p.setDataHandler([this](const DataFrame &f) { data.push_back(f); });
        p.setInfoHandler([this](const InfoFrame &f) { info.push_back(f); });
        p.setTextHandler([this](const QString &s) { text << s; });
    }
};

} // namespace

class TstFrameParser : public QObject {
    Q_OBJECT

private slots:
    void crcMatchesReferenceVector();
    void crcAgreesWithFirmware();
    void crcAgreesWithFirmware_data();

    void decodesDataFrame();
    void decodesInfoFrame();
    void signExtendsNegativeCodes();
    void microvoltScaleMatchesFirmware();

    void reassemblesSplitFrame();
    void reassemblesSplitFrame_data();
    void passesThroughInterleavedLogText();
    void rejectsCorruptedFrame();
    void survivesFalseMagicInPayload();
    void countsDroppedFramesFromSeqGaps();
    void resyncsAfterLeadingGarbage();
    void doesNotStallOnBogusLength();
    void stripsAnsiColourFromLogLines();

    void defaultsToContiguousChannelMap();
    void decodesNonContiguousChannelMap();
    void rejectsMaskThatDisagreesWithChannelCount();
};

// --------------------------------------------------------------------- CRC

void TstFrameParser::crcMatchesReferenceVector()
{
    // The canonical CRC-16/CCITT-FALSE check value.
    const char *s = "123456789";
    QCOMPARE(crc16(reinterpret_cast<const std::uint8_t *>(s), 9),
             std::uint16_t(0x29B1));
    QCOMPARE(emg_crc16(reinterpret_cast<const std::uint8_t *>(s), 9),
             std::uint16_t(0x29B1));
}

void TstFrameParser::crcAgreesWithFirmware_data()
{
    QTest::addColumn<QByteArray>("payload");

    QTest::newRow("empty") << QByteArray();
    QTest::newRow("one zero") << QByteArray(1, '\0');
    QTest::newRow("one ff") << QByteArray(1, '\xff');
    QTest::newRow("ascii") << QByteArray("the quick brown fox");

    QByteArray ramp(256, '\0');
    for (int i = 0; i < 256; ++i) {
        ramp[i] = static_cast<char>(i);
    }
    QTest::newRow("all byte values") << ramp;

    QTest::newRow("max payload") << QByteArray(int(kMaxPayload), '\xa5');
}

void TstFrameParser::crcAgreesWithFirmware()
{
    QFETCH(QByteArray, payload);

    const auto *p = reinterpret_cast<const std::uint8_t *>(payload.constData());
    const auto n = static_cast<std::size_t>(payload.size());

    // Table-driven (host) vs bitwise (firmware) - independent implementations.
    QCOMPARE(crc16(p, n), emg_crc16(p, n));
}

// ------------------------------------------------------------------ decode

void TstFrameParser::decodesDataFrame()
{
    const auto block = makeBlock();
    const QByteArray frame = buildData(7, 12345, 0, block);

    QCOMPARE(frame.size(), qsizetype(EMG_DATA_FRAME_SIZE(kNCh, kNSamp)));
    QCOMPARE(frame.size(), qsizetype(404)); // ARCHITECTURE.md section 4.3 budget

    FrameParser p;
    Sink sink;
    sink.attach(p);
    p.feed(frame);

    QCOMPARE(sink.data.size(), std::size_t(1));
    const DataFrame &f = sink.data.front();
    QCOMPARE(f.seq, 7u);
    QCOMPARE(f.tMs, 12345u);
    QCOMPARE(f.nCh, std::uint8_t(kNCh));
    QCOMPARE(f.nSamp, std::uint8_t(kNSamp));
    QVERIFY(!f.overflow());

    // Every cell survives the channel-major -> sample-major transpose.
    for (int c = 0; c < kNCh; ++c) {
        for (int s = 0; s < kNSamp; ++s) {
            QCOMPARE(f.code(s, c), block[c * kStride + s]);
        }
    }

    QCOMPARE(p.stats().dataFrames, 1u);
    QCOMPARE(p.stats().crcErrors, 0u);
    QCOMPARE(p.pending(), qsizetype(0));
}

void TstFrameParser::decodesInfoFrame()
{
    FrameParser p;
    Sink sink;
    sink.attach(p);

    const QByteArray frame = buildInfo(99);
    QCOMPARE(frame.size(), qsizetype(EMG_INFO_FRAME_SIZE));
    QCOMPARE(frame.size(), qsizetype(30)); // ARCHITECTURE.md section 4.4

    p.feed(frame);

    QCOMPARE(sink.info.size(), std::size_t(1));
    const InfoFrame &f = sink.info.front();
    QCOMPARE(f.sampleRateHz, std::uint16_t(1000));
    QCOMPARE(f.vrefUv, 2400000u);
    QCOMPARE(f.gain, std::uint8_t(6));
    QCOMPARE(f.chipId, std::uint8_t(0x92));
    QCOMPARE(f.nChActive, std::uint8_t(4));
    QCOMPARE(f.uptimeS, 99u);
    QCOMPARE(f.fwVersion, QStringLiteral("abc1234"));
    QVERIFY(f.isValid());
}

void TstFrameParser::signExtendsNegativeCodes()
{
    // Full negative scale, zero, full positive scale, and -1.
    std::vector<std::int32_t> block(kNCh * kStride, 0);
    block[0 * kStride + 0] = -8388608; // 0x800000
    block[1 * kStride + 0] = 0;
    block[2 * kStride + 0] = 8388607; // 0x7FFFFF
    block[3 * kStride + 0] = -1;      // 0xFFFFFF

    FrameParser p;
    Sink sink;
    sink.attach(p);
    p.feed(buildData(0, 0, 0, block));

    QCOMPARE(sink.data.size(), std::size_t(1));
    const DataFrame &f = sink.data.front();
    QCOMPARE(f.code(0, 0), -8388608);
    QCOMPARE(f.code(0, 1), 0);
    QCOMPARE(f.code(0, 2), 8388607);
    QCOMPARE(f.code(0, 3), -1);
}

void TstFrameParser::microvoltScaleMatchesFirmware()
{
    FrameParser p;
    Sink sink;
    sink.attach(p);
    p.feed(buildInfo());

    QCOMPARE(sink.info.size(), std::size_t(1));
    const double uvPerCode = sink.info.front().uvPerCode();

    // firmware/src/main.c: V = code / 2^23 * (VREF / gain), VREF 2.4 V, gain 6.
    // 1 LSB ~= 0.0477 uV, full scale +/-400 mV.
    QVERIFY(qAbs(uvPerCode - 2400000.0 / 6.0 / 8388608.0) < 1e-12);
    QVERIFY(qAbs(uvPerCode - 0.04768371582) < 1e-9);
    QVERIFY(qAbs(8388608.0 * uvPerCode - 400000.0) < 1e-6); // 400 mV in uV
}

// -------------------------------------------------------------- robustness

void TstFrameParser::reassemblesSplitFrame_data()
{
    QTest::addColumn<int>("split");

    // Split inside the magic, inside the header, inside the payload, and
    // inside the trailing CRC.
    for (int at : {1, 2, 3, 5, 6, 7, 200, 401, 402, 403}) {
        QTest::newRow(qPrintable(QStringLiteral("at %1").arg(at))) << at;
    }
}

void TstFrameParser::reassemblesSplitFrame()
{
    QFETCH(int, split);

    const auto block = makeBlock();
    const QByteArray frame = buildData(1, 0, 0, block);
    QVERIFY(split < frame.size());

    FrameParser p;
    Sink sink;
    sink.attach(p);

    p.feed(frame.left(split));
    QCOMPARE(sink.data.size(), std::size_t(0)); // nothing yet
    p.feed(frame.mid(split));

    QCOMPARE(sink.data.size(), std::size_t(1));
    QCOMPARE(sink.data.front().code(3, 2), block[2 * kStride + 3]);
    QCOMPARE(p.stats().crcErrors, 0u);
    QCOMPARE(p.stats().resyncs, 0u);
}

void TstFrameParser::passesThroughInterleavedLogText()
{
    // The property that lets binary frames and Zephyr LOG_* share one UART.
    const auto block = makeBlock();
    const QByteArray frame = buildData(1, 0, 0, block);

    QByteArray stream;
    stream += "[00:00:01.234,567] <inf> emg_read_example: Chip ID = 0x92\r\n";
    stream += frame;
    stream += "[00:00:01.300,000] <wrn> emg_read_example: no data in 2s\r\n";
    stream += buildData(2, 1, 0, block);

    FrameParser p;
    Sink sink;
    sink.attach(p);
    p.feed(stream);

    QCOMPARE(sink.data.size(), std::size_t(2));
    QCOMPARE(sink.text.size(), qsizetype(2));
    QVERIFY(sink.text.at(0).endsWith(QStringLiteral("Chip ID = 0x92")));
    QVERIFY(sink.text.at(1).endsWith(QStringLiteral("no data in 2s")));
    QCOMPARE(p.stats().crcErrors, 0u);
}

void TstFrameParser::rejectsCorruptedFrame()
{
    const auto block = makeBlock();
    QByteArray frame = buildData(1, 0, 0, block);

    frame[100] = static_cast<char>(frame.at(100) ^ 0x01); // one flipped bit

    FrameParser p;
    Sink sink;
    sink.attach(p);
    p.feed(frame);

    QCOMPARE(sink.data.size(), std::size_t(0));
    QCOMPARE(p.stats().crcErrors, 1u);
    QVERIFY(p.stats().resyncs >= 1u);

    // And the stream recovers on the next good frame.
    p.feed(buildData(2, 0, 0, block));
    QCOMPARE(sink.data.size(), std::size_t(1));
    QCOMPARE(sink.data.front().seq, 2u);
}

void TstFrameParser::survivesFalseMagicInPayload()
{
    // Plant the sync pattern in the sample data. Sample values are chosen so
    // the packed 24-bit words literally contain 0xAA 0x55.
    std::vector<std::int32_t> block(kNCh * kStride, 0);
    for (int c = 0; c < kNCh; ++c) {
        for (int s = 0; s < kNSamp; ++s) {
            block[c * kStride + s] = 0xAA55AA - 0x1000000; // 0xAA55AA, negative
        }
    }

    const QByteArray frame = buildData(5, 0, 0, block);
    QVERIFY(frame.mid(int(kHeaderSize)).contains(QByteArray("\xaa\x55", 2)));

    FrameParser p;
    Sink sink;
    sink.attach(p);
    p.feed(frame);

    // The real header wins; the embedded pattern is consumed as payload.
    QCOMPARE(sink.data.size(), std::size_t(1));
    QCOMPARE(sink.data.front().seq, 5u);
    QCOMPARE(p.stats().crcErrors, 0u);
    QCOMPARE(p.stats().resyncs, 0u);
}

void TstFrameParser::countsDroppedFramesFromSeqGaps()
{
    const auto block = makeBlock();

    FrameParser p;
    Sink sink;
    sink.attach(p);

    p.feed(buildData(10, 0, 0, block));
    p.feed(buildData(11, 0, 0, block));
    p.feed(buildData(15, 0, 0, block)); // 12, 13, 14 lost upstream

    QCOMPARE(sink.data.size(), std::size_t(3));
    QCOMPARE(p.stats().droppedFrames, 3u);
}

void TstFrameParser::resyncsAfterLeadingGarbage()
{
    const auto block = makeBlock();

    QByteArray stream("\x01\x02\xaa\xaa\x55\xff garbage ", 15);
    stream += buildData(3, 0, 0, block);

    FrameParser p;
    Sink sink;
    sink.attach(p);
    p.feed(stream);

    QCOMPARE(sink.data.size(), std::size_t(1));
    QCOMPARE(sink.data.front().seq, 3u);
}

void TstFrameParser::doesNotStallOnBogusLength()
{
    // A false sync carrying a valid-looking maximum length must not make the
    // parser wait forever for bytes that will never arrive.
    QByteArray stream;
    stream += static_cast<char>(kMagic0);
    stream += static_cast<char>(kMagic1);
    stream += static_cast<char>(FrameType::Data);
    stream += static_cast<char>(kVersion);
    stream += static_cast<char>(kMaxPayload & 0xFF);
    stream += static_cast<char>((kMaxPayload >> 8) & 0xFF);
    stream += QByteArray(64, '\x00');

    const auto block = makeBlock();
    FrameParser p;
    Sink sink;
    sink.attach(p);

    p.feed(stream);
    QCOMPARE(sink.data.size(), std::size_t(0)); // legitimately still waiting

    // Push enough real traffic that the bogus header must be abandoned.
    for (int i = 0; i < 8; ++i) {
        p.feed(buildData(static_cast<std::uint32_t>(100 + i), 0, 0, block));
    }

    QVERIFY(!sink.data.empty());
    QVERIFY(p.stats().resyncs >= 1u);
}

void TstFrameParser::stripsAnsiColourFromLogLines()
{
    // CONFIG_LOG_BACKEND_SHOW_COLOR wraps <err>/<wrn> lines in SGR sequences.
    FrameParser p;
    Sink sink;
    sink.attach(p);

    p.feed(QByteArray("\x1b[1;31m<err> SPI bus not ready\x1b[0m\r\n"));

    QCOMPARE(sink.text.size(), qsizetype(1));
    QCOMPARE(sink.text.at(0), QStringLiteral("<err> SPI bus not ready"));
}

// A frame from firmware that predates the ch_mask byte carries zero there, and
// must keep meaning "CH1..CH(nCh)". Getting this wrong would silently relabel
// every existing capture.
void TstFrameParser::defaultsToContiguousChannelMap()
{
    FrameParser p;
    Sink sink;
    sink.attach(p);

    p.feed(buildData(0, 0, 0, makeBlock()));

    QCOMPARE(sink.data.size(), std::size_t(1));
    QCOMPARE(sink.data.front().chMask, std::uint8_t(0));
    for (int i = 0; i < kNCh; ++i) {
        QCOMPARE(sink.data.front().channelIndex(i), i);
    }
}

// Firmware streaming CH2 and CH3 (the only bipolar leads on the ADS1298ECG-FE)
// must decode to ADS indices 1 and 2, not 0 and 1.
void TstFrameParser::decodesNonContiguousChannelMap()
{
    const std::uint8_t mask = EMG_CH_MASK_BIT(2) | EMG_CH_MASK_BIT(3);

    FrameParser p;
    Sink sink;
    sink.attach(p);

    std::vector<std::int32_t> block(2 * kNSamp, 0);
    p.feed(buildData(0, 0, 0, block, 2, kNSamp, kNSamp, mask));

    QCOMPARE(sink.data.size(), std::size_t(1));
    const DataFrame &f = sink.data.front();
    QCOMPARE(f.nCh, std::uint8_t(2));
    QCOMPARE(f.chMask, mask);
    QCOMPARE(f.channelIndex(0), 1);   // CH2
    QCOMPARE(f.channelIndex(1), 2);   // CH3
}

// The encoder refuses a mask whose popcount is not n_ch. Such a frame would
// decode without error and mislabel every trace, which is worse than no frame.
void TstFrameParser::rejectsMaskThatDisagreesWithChannelCount()
{
    std::vector<std::int32_t> block(2 * kNSamp, 0);
    const std::uint8_t threeBits =
        EMG_CH_MASK_BIT(1) | EMG_CH_MASK_BIT(2) | EMG_CH_MASK_BIT(3);

    QVERIFY(buildData(0, 0, 0, block, 2, kNSamp, kNSamp, threeBits).isEmpty());
    QVERIFY(!buildData(0, 0, 0, block, 2, kNSamp, kNSamp,
                       EMG_CH_MASK_BIT(2) | EMG_CH_MASK_BIT(3)).isEmpty());
}

QTEST_APPLESS_MAIN(TstFrameParser)
#include "tst_frameparser.moc"
