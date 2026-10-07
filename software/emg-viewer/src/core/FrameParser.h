#pragma once

// Incremental decoder for the EMG wire protocol (ARCHITECTURE.md section 4).
//
// Deliberately Qt-light: bytes in, callbacks out, no QObject, no signals, no
// event loop. That keeps it unit-testable standalone and lets ReplaySource
// reuse it byte-for-byte, so replaying a capture exercises the same decode path
// as the live link rather than a parallel implementation of it.
//
// Robustness contract - all of these are routine, none is an error state:
//   * a frame split across arbitrary read boundaries reassembles;
//   * ASCII log text between frames is passed through in order (Zephyr LOG_*
//     shares the UART; see ARCHITECTURE.md 4.2);
//   * a chance 0xAA 0x55 inside a payload does not cause a false lock;
//   * corruption is caught by CRC, counted, and the stream resynchronises.

#include "EmgProtocol.h"

#include <QByteArray>
#include <QMetaType>
#include <QString>

#include <cstdint>
#include <functional>
#include <vector>

namespace emg {

struct DataFrame {
    std::uint32_t seq = 0;
    std::uint32_t tMs = 0;
    std::uint8_t nCh = 0;
    std::uint8_t nSamp = 0;
    std::uint8_t flags = 0;

    /// Which ADS129x channels these nCh traces are, bit 0 = CH1 .. bit 7 = CH8,
    /// carried in the byte that protocol version 1 originally reserved. Zero
    /// keeps the old meaning: CH1..CH(nCh), contiguous. Anything that names a
    /// channel - lead labels above all - must go through channelIndex(), or a
    /// firmware streaming CH2 and CH3 gets its traces labelled V6 and LEAD I.
    std::uint8_t chMask = 0;

    // Raw ADC codes, sign-extended from 24 bits. Sample-major, exactly as they
    // arrive on the wire: index [sample * nCh + channel].
    std::vector<std::int32_t> codes;

    std::int32_t code(int sample, int channel) const
    {
        return codes[static_cast<std::size_t>(sample) * nCh + channel];
    }

    bool overflow() const { return (flags & kFlagOverflow) != 0; }
    bool leadOff() const { return (flags & kFlagLeadOff) != 0; }

    /// Zero-based ADS channel index carried by trace @p i (0-based).
    int channelIndex(int i) const { return channelIndexFor(chMask, i, nCh); }

    /// Free function form, so callers holding only an InfoFrame-era mask can
    /// resolve labels without a DataFrame in hand.
    static int channelIndexFor(std::uint8_t mask, int i, int nCh)
    {
        if (mask == 0) {
            return i; // contiguous CH1..CH(nCh)
        }
        int seen = 0;
        for (int bit = 0; bit < 8; ++bit) {
            if ((mask & (1u << bit)) == 0) {
                continue;
            }
            if (seen == i) {
                return bit;
            }
            ++seen;
        }
        return i < nCh ? i : 0;
    }
};

struct InfoFrame {
    std::uint16_t sampleRateHz = 0;
    std::uint32_t vrefUv = 0;
    std::uint8_t gain = 0;
    std::uint8_t chipId = 0;
    std::uint8_t nChActive = 0;
    std::uint8_t hrMode = 0;
    std::uint32_t uptimeS = 0;
    QString fwVersion;

    // Microvolts per LSB: V = code / 2^23 * (VREF / gain). Mirrors code_to_uV()
    // in firmware/src/main.c. Never hardcode this on the host - it arrives on
    // the wire precisely because the firmware and its docs have disagreed about
    // gain before.
    double uvPerCode() const
    {
        if (gain == 0) {
            return 0.0;
        }
        return static_cast<double>(vrefUv) / gain / kFullScaleCode;
    }

    bool isValid() const { return sampleRateHz > 0 && gain > 0 && nChActive > 0; }
};

struct ParserStats {
    std::uint64_t bytesIn = 0;
    std::uint64_t dataFrames = 0;
    std::uint64_t infoFrames = 0;
    std::uint64_t textFrames = 0;
    std::uint64_t samples = 0;      // total samples across all channels
    std::uint64_t crcErrors = 0;
    std::uint64_t resyncs = 0;      // times we discarded a byte to re-hunt sync
    std::uint64_t droppedFrames = 0; // inferred from gaps in DataFrame::seq
    std::uint64_t textBytes = 0;

    void reset() { *this = ParserStats{}; }
};

class FrameParser {
public:
    using DataHandler = std::function<void(const DataFrame &)>;
    using InfoHandler = std::function<void(const InfoFrame &)>;
    using TextHandler = std::function<void(const QString &)>;

    FrameParser();

    void setDataHandler(DataHandler h) { m_onData = std::move(h); }
    void setInfoHandler(InfoHandler h) { m_onInfo = std::move(h); }
    void setTextHandler(TextHandler h) { m_onText = std::move(h); }

    // Feed arbitrary-sized chunks. Callbacks fire synchronously from here.
    void feed(const char *data, qsizetype len);
    void feed(const QByteArray &chunk) { feed(chunk.constData(), chunk.size()); }

    // Emit any buffered partial text line. Call on disconnect / end of file so
    // a final unterminated log line is not swallowed.
    void flushText();

    void reset();

    const ParserStats &stats() const { return m_stats; }

    // Total bytes currently held pending more input.
    qsizetype pending() const { return m_buf.size(); }

private:
    enum class HeaderCheck { Ok, Incomplete, Invalid };

    HeaderCheck inspectHeader(qsizetype pos, std::uint8_t &type,
                              std::uint16_t &len) const;
    void dispatch(std::uint8_t type, const std::uint8_t *payload,
                  std::uint16_t len);
    void emitData(const std::uint8_t *payload, std::uint16_t len);
    void emitInfo(const std::uint8_t *payload, std::uint16_t len);
    void pushText(const char *data, qsizetype len);

    QByteArray m_buf;      // undecoded bytes
    QByteArray m_textBuf;  // partial trailing text line
    ParserStats m_stats;

    bool m_haveSeq = false;
    std::uint32_t m_lastSeq = 0;

    DataHandler m_onData;
    InfoHandler m_onInfo;
    TextHandler m_onText;
};

} // namespace emg

// Both cross a queued connection from the IO thread to the GUI thread.
Q_DECLARE_METATYPE(emg::InfoFrame)
Q_DECLARE_METATYPE(emg::ParserStats)
