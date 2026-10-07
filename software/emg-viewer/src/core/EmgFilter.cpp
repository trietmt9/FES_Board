#include "EmgFilter.h"

#include <algorithm>
#include <cmath>

namespace emg {

void ChannelFilter::configure(const FilterConfig &cfg, double sampleRate)
{
    m_cfg = cfg;
    m_rate = sampleRate > 0.0 ? sampleRate : 1000.0;

    // 4th-order Butterworth as two biquads with the standard Q pair.
    m_hp1 = designHighpass(m_cfg.highpassHz, m_rate, kButter4Q1);
    m_hp2 = designHighpass(m_cfg.highpassHz, m_rate, kButter4Q2);

    // Clamp the low-pass below Nyquist. At 1000 SPS a 450 Hz corner is already
    // at 0.45*fs, and the ADC's own sinc^3 filter is -3 dB at 262 Hz anyway, so
    // this mostly matters for not producing nonsense coefficients.
    const double lp = std::min(m_cfg.lowpassHz, 0.45 * m_rate);
    m_lp1 = designLowpass(lp, m_rate, kButter4Q1);
    m_lp2 = designLowpass(lp, m_rate, kButter4Q2);

    m_notches.clear();
    if (m_cfg.notchEnabled && m_cfg.notchHz > 0.0) {
        for (int h = 1; h <= std::max(1, m_cfg.notchHarmonics); ++h) {
            const double f = m_cfg.notchHz * h;
            if (f >= 0.45 * m_rate) {
                break; // too close to Nyquist to design
            }
            // A HARMONIC above the low-pass corner is already well down and not
            // worth a notch. The FUNDAMENTAL is different: with the ECG band's
            // 40 Hz corner a 50 Hz mains line is only ~8 dB down on the filter's
            // roll-off alone, and removing it is the whole point of an ECG
            // notch. This used to skip any notch >= the corner, which silently
            // dropped the one that mattered - while the UI went on showing
            // "Notch 50 Hz".
            if (h > 1 && f >= lp) {
                break;
            }
            m_notches.push_back(designNotch(f, m_rate, 30.0));
        }
    }

    const auto n = static_cast<std::size_t>(
        std::max(1.0, m_cfg.envelopeMs * 1e-3 * m_rate));
    m_sq.assign(n, 0.0f);

    reset();
}

void ChannelFilter::reset()
{
    m_hp1.reset();
    m_hp2.reset();
    m_lp1.reset();
    m_lp2.reset();
    for (auto &n : m_notches) {
        n.reset();
    }

    std::fill(m_sq.begin(), m_sq.end(), 0.0f);
    m_sqIndex = 0;
    m_sqSum = 0.0;
    m_sqFilled = 0;
    m_envelope = 0.0;
    m_primed = false;
}

float ChannelFilter::process(float uv)
{
    // Always filters, and always returns the filtered value. Whether the user
    // *sees* raw or filtered is decided upstream by StreamController::displayRing(),
    // which picks between two rings - so this function has no business consulting
    // FilterConfig::enabled. It used to, and the result was that the "filtered"
    // ring got filled with raw samples whenever the display was set to raw:
    // switching to the filtered view then showed unfiltered history.
    // Prime on the first sample. This front end is DC-coupled and the electrode
    // half-cell offset is millivolts to hundreds of millivolts, so a high-pass
    // started from zero state sees that offset as a step and rings for many
    // time constants - at a 0.5 Hz corner, several seconds with the trace
    // pinned off the top of a +/-1.6 mV plot. The display used to hide this by
    // subtracting the window mean; the spec's display has no such step, so the
    // filter has to start already settled. Only the first high-pass needs it:
    // its output starts at exactly 0, so everything downstream correctly
    // starts from zero state.
    if (!m_primed) {
        m_hp1.prime(static_cast<double>(uv));
        m_primed = true;
    }

    float y = uv;
    y = m_hp1.process(y);
    y = m_hp2.process(y);
    y = m_lp1.process(y);
    y = m_lp2.process(y);
    for (auto &n : m_notches) {
        y = n.process(y);
    }

    // Sliding-window RMS of the filtered signal.
    if (!m_sq.empty()) {
        const float sq = y * y;
        m_sqSum -= m_sq[m_sqIndex];
        m_sq[m_sqIndex] = sq;
        m_sqSum += sq;

        m_sqIndex = (m_sqIndex + 1) % m_sq.size();
        if (m_sqFilled < m_sq.size()) {
            ++m_sqFilled;
        }

        // Guard against the running sum drifting negative through float
        // cancellation over millions of samples.
        if (m_sqSum < 0.0) {
            m_sqSum = 0.0;
        }
        m_envelope = std::sqrt(m_sqSum / static_cast<double>(m_sqFilled));
    }

    return y;
}

} // namespace emg
