#pragma once

// QRS detection and heart rate, by a reduced Pan-Tompkins.
//
// The number you check an ECG simulator against: set it to 60 BPM, the readout
// must say 60. That makes it the single most useful end-to-end validation of the
// whole chain - electrodes, AFE, SPI, link, host, display - because unlike EMG
// the correct answer is known in advance.
//
// Pipeline, per Pan-Tompkins (1985):
//   band-pass 5-15 Hz   isolate QRS energy from P/T waves, drift and muscle
//   derivative          QRS slope is what distinguishes it from P and T
//   square              rectify, and emphasise the large slopes
//   moving integration  150 ms window -> one broad bump per QRS
//   adaptive threshold  tracks signal and noise levels separately
//   200 ms refractory   no physiological beat follows another that fast
//
// Deliberately fed the RAW signal: it runs its own band-pass, so the detector
// is independent of whatever the display filter is set to.

#include "Biquad.h"

#include <cstddef>
#include <deque>
#include <vector>

namespace emg {

/// Shortest credible interval between beats. 200 ms is the refractory period
/// Pan-Tompkins uses; it corresponds to 300 BPM, above any real heart rate and
/// above what a simulator will produce.
inline constexpr double kRefractoryMs = 200.0;

class HeartRateDetector {
public:
    void configure(double sampleRate);
    void reset();

    /// Feed one raw sample in microvolts. Returns true on the sample where a
    /// QRS complex is detected.
    bool process(float uv);

    /// Heart rate in BPM, or 0 before enough beats have been seen.
    ///
    /// Taken as the median of the recent RR intervals rather than the mean: one
    /// missed or doubled beat shifts a mean noticeably but leaves a median
    /// alone, and a rate readout that lurches on a single bad beat is worse
    /// than useless.
    double bpm() const;

    /// Most recent beat-to-beat interval, milliseconds. 0 if unknown.
    double lastRrMs() const { return m_lastRrMs; }

    /// Beats detected since the last reset.
    std::size_t beats() const { return m_beats; }

private:
    double m_rate = 1000.0;

    // QRS isolation band, 4th-order Butterworth as two cascaded biquads a side.
    //
    // A single biquad (12 dB/octave) is not enough ahead of the derivative
    // stage. An impulsive artifact is broadband, so a 2nd-order low-pass at
    // 15 Hz leaves it only 33 dB down at 100 Hz - and the derivative that
    // follows multiplies by frequency, handing most of that straight back.
    // Doubling the order doubles the rejection in dB: 66 dB at 100 Hz.
    Biquad m_hp1, m_hp2;
    Biquad m_lp1, m_lp2;

    std::vector<float> m_window;   // moving integration window
    std::size_t m_windowIndex = 0;
    double m_windowSum = 0.0;

    double m_prevFiltered = 0.0;   // for the derivative
    double m_signalLevel = 0.0;    // adaptive: running estimate of QRS peaks
    double m_noiseLevel = 0.0;     // adaptive: running estimate of everything else

    // Current above-threshold excursion. The level estimates are updated from
    // an excursion's PEAK when it ends, never per sample - updating them
    // continuously makes the threshold follow the signal down, so it can never
    // fall below and the detector latches after one beat.
    bool m_inExcursion = false;
    double m_peak = 0.0;
    bool m_beatThisExcursion = false;

    // Pan-Tompkins' learning phase: observe before deciding. Without it the
    // levels start at zero, the threshold starts at zero, and the first sample
    // of anything at all looks like a QRS.
    bool m_learning = true;
    std::size_t m_learnRemaining = 0;

    // Settling period BEFORE learning. A DC-coupled channel starts with several
    // millivolts of electrode offset, which is a step into the 5 Hz high-pass
    // and rings for far longer than any QRS. Learning through that transient
    // sets the signal level absurdly high and the detector never fires again.
    std::size_t m_settleRemaining = 0;

    // Per-sample decay applied to the signal level when nothing has been
    // detected for implausibly long, so a one-off artifact cannot deafen the
    // detector permanently.
    double m_stuckDecay = 1.0;
    std::size_t m_stuckSamples = 0;

    std::size_t m_sinceBeat = 0;
    std::size_t m_refractorySamples = 0;
    std::size_t m_beats = 0;
    double m_lastRrMs = 0.0;

    std::deque<double> m_rr;       // recent intervals, milliseconds

    /// Median of the recent RR intervals, or 0 with fewer than two.
    /// Used both to report the rate and to size the adaptive refractory.
    double medianRrMs() const;
};

} // namespace emg
