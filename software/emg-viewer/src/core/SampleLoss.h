#pragma once

// Device-side conversion loss, measured from the stream's own timestamps.
//
// Lives here rather than in ISampleSource so it can be unit-tested directly:
// it is the number a user reaches for when a trace looks wrong, so it has to be
// right, and "it compiled" is not evidence of that.

#include <cstdint>

namespace emg {

/**
 * Device-side conversion loss, measured rather than assumed.
 *
 * Every DATA frame carries t_ms, the device's own uptime at block close.
 * Between the first block close and the latest one the device should have
 * produced (dt_ms * rate / 1000) conversions; it actually delivered
 * (blocks - 1) * nSamp of them. The difference is conversions the firmware
 * never got - a DRDY it missed, or an RDATAC frame it discarded because the
 * status word was misaligned.
 *
 * This matters because of how the loss otherwise presents. The host lays
 * samples down at exactly 1/rate, so a dropped conversion does not leave a
 * gap - it SHORTENS the waveform. A QRS complex comes out narrower and
 * taller, the baseline looks jittery, and the whole trace reads as "noisy"
 * when nothing analog is wrong at all. Both clocks here are the device's
 * own, so this is immune to host scheduling; the only error term is the
 * ppm difference between the STM32 and ADS oscillators, which is orders of
 * magnitude below any loss worth reporting.
 */
class SampleLossTracker {
public:
    void reset() { *this = SampleLossTracker{}; }

    void note(std::uint32_t tMs, int nSamp, double rate)
    {
        if (!m_have) {
            m_have = true;
            m_firstMs = tMs;
            m_lastMs = tMs;
            return;
        }
        // t_ms is a uint32 of milliseconds: it wraps after 49.7 days. Treat
        // a backwards step as a device restart and rebase rather than
        // reporting a 4-billion-sample loss.
        if (tMs < m_lastMs) {
            reset();
            m_have = true;
            m_firstMs = tMs;
            m_lastMs = tMs;
            return;
        }
        m_lastMs = tMs;
        m_delivered += static_cast<std::uint64_t>(nSamp);

        const double expected =
            double(m_lastMs - m_firstMs) * rate / 1000.0;
        const double lost = expected - double(m_delivered);
        m_lost = lost > 0.0 ? static_cast<std::uint64_t>(lost + 0.5) : 0;
    }

    std::uint64_t lost() const { return m_lost; }

private:
    bool m_have = false;
    std::uint32_t m_firstMs = 0;
    std::uint32_t m_lastMs = 0;
    std::uint64_t m_delivered = 0;   // conversions since the first block close
    std::uint64_t m_lost = 0;
};

} // namespace emg
