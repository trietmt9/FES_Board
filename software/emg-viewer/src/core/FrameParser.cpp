#include "FrameParser.h"

#include "Crc16.h"

namespace emg {
namespace {

std::uint16_t readU16le(const std::uint8_t *p)
{
    return static_cast<std::uint16_t>(p[0] | (p[1] << 8));
}

std::uint32_t readU32le(const std::uint8_t *p)
{
    return static_cast<std::uint32_t>(p[0]) |
           (static_cast<std::uint32_t>(p[1]) << 8) |
           (static_cast<std::uint32_t>(p[2]) << 16) |
           (static_cast<std::uint32_t>(p[3]) << 24);
}

// 24-bit big-endian two's complement -> int32. Mirrors the sign extension in
// ads_emg_read(), firmware/drivers/src/ads129x.c:149-153.
std::int32_t readS24be(const std::uint8_t *p)
{
    std::int32_t v = (static_cast<std::int32_t>(p[0]) << 16) |
                     (static_cast<std::int32_t>(p[1]) << 8) |
                     static_cast<std::int32_t>(p[2]);
    if (v & 0x800000) {
        v |= ~0x00FFFFFF; // sign-extend bit 23
    }
    return v;
}

bool isKnownType(std::uint8_t t)
{
    return t == static_cast<std::uint8_t>(FrameType::Data) ||
           t == static_cast<std::uint8_t>(FrameType::Info) ||
           t == static_cast<std::uint8_t>(FrameType::Text);
}

// Strip ANSI SGR sequences. Zephyr is built with CONFIG_LOG_BACKEND_SHOW_COLOR,
// so <err>/<wrn> lines arrive wrapped in \x1b[1;31m ... \x1b[0m. <inf> lines are
// uncoloured, but stripping unconditionally keeps the console pane clean.
QString stripAnsi(const QByteArray &line)
{
    QString out;
    out.reserve(line.size());

    for (qsizetype i = 0; i < line.size(); ++i) {
        const char c = line.at(i);
        if (c == '\x1b') {
            // Skip through the final byte of a CSI sequence (0x40..0x7E).
            qsizetype j = i + 1;
            if (j < line.size() && line.at(j) == '[') {
                ++j;
                while (j < line.size()) {
                    const unsigned char f = static_cast<unsigned char>(line.at(j));
                    if (f >= 0x40 && f <= 0x7E) {
                        break;
                    }
                    ++j;
                }
            }
            i = j;
            continue;
        }
        if (c == '\r') {
            continue;
        }
        out.append(QLatin1Char(c));
    }
    return out;
}

// Bound on how much unparsed data we hold before concluding that a plausible
// but bogus length field (from a false sync inside a payload) is stalling us.
constexpr qsizetype kStallLimit =
    static_cast<qsizetype>(kMaxFrameSize + kMaxPayload + kOverhead);

} // namespace

FrameParser::FrameParser() = default;

void FrameParser::reset()
{
    m_buf.clear();
    m_textBuf.clear();
    m_stats.reset();
    m_haveSeq = false;
    m_lastSeq = 0;
}

FrameParser::HeaderCheck FrameParser::inspectHeader(qsizetype pos,
                                                    std::uint8_t &type,
                                                    std::uint16_t &len) const
{
    if (m_buf.size() - pos < static_cast<qsizetype>(kHeaderSize)) {
        return HeaderCheck::Incomplete;
    }

    const auto *p = reinterpret_cast<const std::uint8_t *>(m_buf.constData()) + pos;
    type = p[2];
    const std::uint8_t ver = p[3];
    len = readU16le(p + 4);

    // Validate before trusting `len`, so a false sync cannot make us wait for a
    // frame that will never arrive.
    if (!isKnownType(type) || ver != kVersion ||
        len > static_cast<std::uint16_t>(kMaxPayload)) {
        return HeaderCheck::Invalid;
    }
    return HeaderCheck::Ok;
}

void FrameParser::feed(const char *data, qsizetype len)
{
    if (data == nullptr || len <= 0) {
        return;
    }

    m_stats.bytesIn += static_cast<std::uint64_t>(len);
    m_buf.append(data, len);

    qsizetype pos = 0;

    while (pos < m_buf.size()) {
        // Hunt for the sync pattern from the current position.
        qsizetype magic = -1;
        for (qsizetype i = pos; i + 1 < m_buf.size(); ++i) {
            if (static_cast<std::uint8_t>(m_buf.at(i)) == kMagic0 &&
                static_cast<std::uint8_t>(m_buf.at(i + 1)) == kMagic1) {
                magic = i;
                break;
            }
        }

        if (magic < 0) {
            // No sync anywhere. Everything is text except a trailing lone 0xAA,
            // which might be the first half of a frame arriving next chunk.
            qsizetype textEnd = m_buf.size();
            if (textEnd > pos &&
                static_cast<std::uint8_t>(m_buf.at(textEnd - 1)) == kMagic0) {
                --textEnd;
            }
            pushText(m_buf.constData() + pos, textEnd - pos);
            pos = textEnd;
            break;
        }

        if (magic > pos) {
            pushText(m_buf.constData() + pos, magic - pos);
            pos = magic;
        }

        std::uint8_t type = 0;
        std::uint16_t payloadLen = 0;
        const HeaderCheck check = inspectHeader(pos, type, payloadLen);

        if (check == HeaderCheck::Incomplete) {
            break; // wait for more bytes
        }

        if (check == HeaderCheck::Invalid) {
            // Not a real header. Give up exactly one byte, so a pattern like
            // AA AA 55 still finds the genuine sync on the next pass.
            pushText(m_buf.constData() + pos, 1);
            ++pos;
            ++m_stats.resyncs;
            continue;
        }

        const qsizetype total = static_cast<qsizetype>(kOverhead) + payloadLen;
        if (m_buf.size() - pos < total) {
            // Plausible header, frame still in flight. Only bail out if we have
            // clearly stalled on a bogus length.
            if (m_buf.size() - pos > kStallLimit) {
                pushText(m_buf.constData() + pos, 1);
                ++pos;
                ++m_stats.resyncs;
                continue;
            }
            break;
        }

        const auto *frame =
            reinterpret_cast<const std::uint8_t *>(m_buf.constData()) + pos;

        // CRC spans type/ver/len plus payload - covering the length field is
        // what stops a corrupted `len` from resyncing us onto garbage.
        const std::uint16_t want = readU16le(frame + kHeaderSize + payloadLen);
        const std::uint16_t got =
            crc16(frame + 2, (kHeaderSize - 2) + payloadLen);

        if (want != got) {
            ++m_stats.crcErrors;
            ++m_stats.resyncs;
            pushText(m_buf.constData() + pos, 1);
            ++pos;
            continue;
        }

        dispatch(type, frame + kHeaderSize, payloadLen);
        pos += total;
    }

    if (pos > 0) {
        m_buf.remove(0, pos);
    }
}

void FrameParser::dispatch(std::uint8_t type, const std::uint8_t *payload,
                           std::uint16_t len)
{
    switch (static_cast<FrameType>(type)) {
    case FrameType::Data:
        emitData(payload, len);
        break;
    case FrameType::Info:
        emitInfo(payload, len);
        break;
    case FrameType::Text:
        ++m_stats.textFrames;
        pushText(reinterpret_cast<const char *>(payload), len);
        break;
    }
}

void FrameParser::emitData(const std::uint8_t *payload, std::uint16_t len)
{
    if (len < kDataHeaderSize) {
        ++m_stats.crcErrors; // structurally impossible but CRC-clean: count it
        return;
    }

    DataFrame f;
    f.seq = readU32le(payload);
    f.tMs = readU32le(payload + 4);
    f.nCh = payload[8];
    f.nSamp = payload[9];
    f.flags = payload[10];
    f.chMask = payload[11];   // 0 on pre-mask firmware = contiguous CH1..CH(nCh)

    const std::size_t count = static_cast<std::size_t>(f.nCh) * f.nSamp;
    const std::size_t need = kDataHeaderSize + count * kBytesPerSample;

    if (f.nCh == 0 || f.nCh > kMaxChannels || f.nSamp == 0 ||
        f.nSamp > kMaxSamples || len != need) {
        ++m_stats.crcErrors;
        return;
    }

    f.codes.resize(count);
    const std::uint8_t *s = payload + kDataHeaderSize;
    for (std::size_t i = 0; i < count; ++i) {
        f.codes[i] = readS24be(s + i * kBytesPerSample);
    }

    // Gaps in the sequence number are the only way to see loss that happened
    // upstream of us - on the wire, or in the firmware's own ring buffer.
    if (m_haveSeq) {
        const std::uint32_t expected = m_lastSeq + 1;
        if (f.seq != expected) {
            m_stats.droppedFrames += static_cast<std::uint32_t>(f.seq - expected);
        }
    }
    m_lastSeq = f.seq;
    m_haveSeq = true;

    ++m_stats.dataFrames;
    m_stats.samples += count;

    if (m_onData) {
        m_onData(f);
    }
}

void FrameParser::emitInfo(const std::uint8_t *payload, std::uint16_t len)
{
    if (len != kInfoPayloadSize) {
        ++m_stats.crcErrors;
        return;
    }

    InfoFrame f;
    f.sampleRateHz = readU16le(payload);
    f.vrefUv = readU32le(payload + 2);
    f.gain = payload[6];
    f.chipId = payload[7];
    f.nChActive = payload[8];
    f.hrMode = payload[9];
    f.uptimeS = readU32le(payload + 10);

    const auto *v = reinterpret_cast<const char *>(payload + 14);
    qsizetype vlen = 0;
    while (vlen < static_cast<qsizetype>(kFwVersionLen) && v[vlen] != '\0') {
        ++vlen;
    }
    f.fwVersion = QString::fromLatin1(v, vlen);

    ++m_stats.infoFrames;

    if (m_onInfo) {
        m_onInfo(f);
    }
}

void FrameParser::pushText(const char *data, qsizetype len)
{
    if (len <= 0) {
        return;
    }

    m_stats.textBytes += static_cast<std::uint64_t>(len);
    m_textBuf.append(data, len);

    // Emit complete lines only; hold the remainder for the next chunk.
    qsizetype nl;
    while ((nl = m_textBuf.indexOf('\n')) >= 0) {
        const QByteArray line = m_textBuf.left(nl);
        m_textBuf.remove(0, nl + 1);

        const QString text = stripAnsi(line);
        if (!text.isEmpty() && m_onText) {
            m_onText(text);
        }
    }

    // Guard against a peer that never sends a newline.
    if (m_textBuf.size() > 4096) {
        const QString text = stripAnsi(m_textBuf);
        m_textBuf.clear();
        if (!text.isEmpty() && m_onText) {
            m_onText(text);
        }
    }
}

void FrameParser::flushText()
{
    if (m_textBuf.isEmpty()) {
        return;
    }
    const QString text = stripAnsi(m_textBuf);
    m_textBuf.clear();
    if (!text.isEmpty() && m_onText) {
        m_onText(text);
    }
}

} // namespace emg
