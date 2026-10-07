#pragma once

// Per-channel surface-EMG conditioning chain.
//
// WHY THIS IS NOT OPTIONAL. The ADS1298ECG-FE input network is DC-coupled as
// populated - the AC-coupling capacitors (C87, C25, ...) are Not Installed and
// JP6-JP14 default to pins 1-2 shorted - so there is no high-pass anywhere in
// hardware. Raw from the ADC you therefore get:
//
//   * millivolts of electrode half-cell offset,
//   * sub-hertz baseline drift from electrode settling and movement,
//   * whatever mains hum the electrodes pick up,
//
// all of which are one to three orders of magnitude larger than the 50-500 uV
// of muscle activity underneath. On screen the drift sets the autoscale range
// and the burst becomes invisible; in a window-wide RMS the drift dominates the
// number. Filtering is what makes surface EMG visible at all.
//
// Chain: 4th-order Butterworth high-pass -> 4th-order Butterworth low-pass ->
// optional mains notch and harmonics -> RMS envelope.
//
// Fourth order rather than second for the high-pass because drift, not noise,
// is the enemy here: at 0.5 Hz a 4th-order 20 Hz high-pass is about 128 dB
// down versus 64 dB for a 2nd-order.
//
// Band per SENIAM: 20-450 Hz. Below 20 Hz is motion artifact, above 450 Hz
// there is no EMG left.

#include "Biquad.h"

#include <QMetaType>

#include <cstddef>
#include <vector>

namespace emg {

struct FilterConfig {
    /// Selects which ring the *display* reads (see StreamController::displayRing()).
    /// It does not gate the filter itself - both rings are always filled, and
    /// the envelope always comes from the filtered chain.
    ///
    /// OFF by default: the scope shows exactly what the ADC captured.
    ///
    /// This was briefly flipped on, reasoning that raw is not a clean view on a
    /// DC-coupled front end - offset, drift and mains all reach the screen. That
    /// is true and it is still not the right default. An instrument's job is to
    /// show the measurement; deciding that some of it is noise is the operator's
    /// call, not the tool's. The cost of getting this wrong is not cosmetic: a
    /// hidden default filter silently removes the signal under test, which is
    /// exactly what happened to a 40-100 Hz sine sweep against the 0.5-40 Hz
    /// ECG band - a working board showed almost nothing and the filter, not the
    /// hardware, was the reason.
    ///
    /// Turning it on is one click, and the choice now persists.
    bool enabled = false;

    /// Corners default to the ECG-monitor band, matching
    /// StreamController::m_signalMode and ChannelModel::Scheme::Ads1298Evm -
    /// the attached board is an ECG front end, so the ECG band is the
    /// consistent default. Switching the Signal selector to EMG moves all
    /// three together; see StreamController::setSignalMode().
    double highpassHz = 0.5;
    double lowpassHz = 40.0;

    bool notchEnabled = true;
    double notchHz = 50.0;      ///< mains: 50 or 60
    int notchHarmonics = 2;     ///< notch f0 and this many multiples (50, 100)

    /// RMS averaging window, and therefore the envelope's response time.
    ///
    /// 250 ms is the textbook surface-EMG constant, but it feels sluggish for
    /// live feedback. 100 ms still averages ~10 cycles of the dominant 100 Hz
    /// content - enough to smooth the interference pattern - while responding
    /// fast enough to feel immediate, so that is the default here.
    double envelopeMs = 100.0;

    bool operator==(const FilterConfig &o) const
    {
        return enabled == o.enabled && highpassHz == o.highpassHz &&
               lowpassHz == o.lowpassHz && notchEnabled == o.notchEnabled &&
               notchHz == o.notchHz && notchHarmonics == o.notchHarmonics &&
               envelopeMs == o.envelopeMs;
    }
    bool operator!=(const FilterConfig &o) const { return !(*this == o); }
};

/// One channel's filter state. Stateful and sample-by-sample: run it on the
/// stream as samples arrive, never on a re-snapshotted display window, or every
/// repaint restarts the transient.
class ChannelFilter {
public:
    void configure(const FilterConfig &cfg, double sampleRate);
    void reset();

    /// Filter one sample of microvolts, advance the envelope, return the
    /// filtered value. Unconditional: FilterConfig::enabled is a *display*
    /// choice consumed by StreamController::displayRing(), not a bypass here.
    float process(float uv);

    /// Running RMS of the filtered signal, in microvolts.
    double envelope() const { return m_envelope; }

    bool enabled() const { return m_cfg.enabled; }

private:
    FilterConfig m_cfg;
    double m_rate = 1000.0;

    /// False until the first sample after configure()/reset(), which primes the
    /// high-pass to that sample's DC level - see process().
    bool m_primed = false;

    Biquad m_hp1, m_hp2;
    Biquad m_lp1, m_lp2;
    std::vector<Biquad> m_notches;

    // Circular sum-of-squares for the envelope. Keeping a running sum makes
    // this O(1) per sample instead of O(window).
    std::vector<float> m_sq;
    std::size_t m_sqIndex = 0;
    double m_sqSum = 0.0;
    std::size_t m_sqFilled = 0;
    double m_envelope = 0.0;
};

} // namespace emg

// Crosses a queued connection from the GUI thread to the IO thread.
Q_DECLARE_METATYPE(emg::FilterConfig)
