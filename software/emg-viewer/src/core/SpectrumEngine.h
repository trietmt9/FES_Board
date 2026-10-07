#pragma once

// Frame-by-frame spectrum for the frequency-domain view.
//
// This is NOT core/Spectrum.h. That one is a Welch estimator built to read a
// long, stochastic EMG record - many segments, heavy variance reduction, one
// result. This one is the live display the spec describes (Biosignal Monitor -
// Qt Spec.md, section 6.2): one 512-point transform every few frames, smoothed
// by an exponential average so the line is steady but still follows the signal.
//
//   N = 512, Hann window, mean removed, power = |X|^2 / N
//   averaging: exponential, new-frame weight  Low 0.6 / Med 0.3 / High 0.12
//   y axis:    top = smoothed max (0.92 / 0.08 EMA) + 4 dB, span 48 dB
//
// Everything is relative. There is no absolute dB scale on screen (the row label
// reads "dB . rel"), so what matters is that the SHAPE and the peak frequency are
// right, and that nothing diverges on silence.

#include <cstddef>
#include <vector>

namespace emg {

class SpectrumEngine {
public:
    /// Transform length, fixed by the spec. Frequency resolution is therefore
    /// sampleRate / 512: 0.49 Hz at 250 Hz, 0.50 Hz at 256 Hz, 1.95 Hz at 1 kHz,
    /// which are the three figures the spec prints in its footer.
    static constexpr std::size_t kFftSize = 512;

    /// How hard the exponential average smooths. The value is the weight given
    /// to the NEW frame, so Low is the least smoothing.
    enum class Averaging { Low, Medium, High };
    static double weightFor(Averaging a);

    /// Y-axis span in dB. The top follows the data, the bottom is always this
    /// far below it.
    static constexpr double kSpanDb = 48.0;

    /// Headroom above the smoothed maximum, so the tallest peak does not touch
    /// the top edge.
    static constexpr double kHeadroomDb = 4.0;

    /// Floor on linear power before the dB conversion. Silence plots as a flat
    /// line at the bottom instead of diverging to -infinity.
    static constexpr double kPowerFloor = 1e-12;

    struct Config {
        /// Rate of the samples handed to update() - i.e. AFTER decimation.
        double sampleRateHz = 250.0;
        /// Highest frequency kept for display and analysis.
        double displayMaxHz = 40.0;
        /// The peak search ignores everything below this. 0.8 Hz for ECG/EEG
        /// (below that is baseline wander), 20 Hz for EMG (motion artifact).
        double peakMinHz = 0.8;
    };

    void configure(const Config &cfg);

    /// Forget the averages. Call when the signal source changes.
    void reset();

    /// Feed exactly kFftSize samples at the configured rate. The mean is removed
    /// here, so a DC offset cannot smear across the low bins.
    void update(const float *samples, Averaging averaging);

    bool valid() const { return m_valid; }
    double sampleRateHz() const { return m_cfg.sampleRateHz; }
    double binHz() const { return m_cfg.sampleRateHz / double(kFftSize); }

    /// Bins from 0 Hz up to displayMaxHz inclusive.
    std::size_t displayBins() const { return m_bins; }

    /// Averaged power per bin in dB, displayBins() entries, bin 0 = 0 Hz.
    const std::vector<float> &powerDb() const { return m_db; }

    /// Smoothed maximum of the averaged spectrum, dB. Excludes bin 0, which the
    /// mean removal leaves near-empty.
    double smoothedMaxDb() const { return m_smoothedMax; }

    /// Axis top in dB for a given display gain.
    ///
    /// DELIBERATE DEVIATION FROM THE SPEC, which writes "+ 20*log10(gain)". With
    /// a plus, gain x2 moves the top UP 6 dB and so draws the spectrum LOWER -
    /// the opposite of the time-domain gain, where x2 makes the trace taller. A
    /// control that does the reverse of what its label says is a bug whichever
    /// way the mockup wrote it, so the term is subtracted: x2 raises the curve.
    /// See SPEC_COMPLIANCE.md.
    double axisTopDb(double gain) const;
    double axisBottomDb(double gain) const { return axisTopDb(gain) - kSpanDb; }

    /// Frequency of the largest bin at or above peakMinHz, refined by parabolic
    /// interpolation on the dB values (a Hann-windowed tone spreads over three
    /// bins, so the raw bin centre is up to half a bin off). 0 if none.
    double peakHz() const { return m_peakHz; }
    double peakDb() const { return m_peakDb; }

    /// Frequency below which half the power in [loHz, hiHz] lies - the classic
    /// fatigue index for surface EMG. Linear interpolation inside the bin.
    double medianHz(double loHz, double hiHz) const;

    /// Linear power summed over the bins whose centres fall in [loHz, hiHz).
    double bandPower(double loHz, double hiHz) const;

    // ---- decimation ---------------------------------------------------------
    //
    // The device streams at its ADC rate (about 1130 or 2000 SPS here), not at
    // the spec's nominal 250 / 256 / 1000 Hz. Run the transform at the device
    // rate and the resolution is 4-8x coarser than the spec shows, which is
    // 18 bins across an ECG's 0-40 Hz. Averaging blocks of M samples first gets
    // back to about the spec's rate.

    /// Integer factor that brings @p deviceRateHz closest to @p targetRateHz,
    /// never below 1.
    static int decimationFactor(double deviceRateHz, double targetRateHz);

    /// Mean of each block of @p factor samples. The mean is a crude low-pass, so
    /// it also keeps energy above the new Nyquist from aliasing back in.
    /// @p out receives nIn / factor samples.
    static void decimateMean(const float *in, std::size_t nIn, int factor, float *out);

private:
    Config m_cfg;
    std::size_t m_bins = 0;
    std::vector<double> m_window;      // Hann, kFftSize
    std::vector<double> m_avgPower;    // averaged linear power, kFftSize/2 + 1
    std::vector<float> m_db;           // displayBins()
    double m_smoothedMax = 0.0;
    double m_peakHz = 0.0;
    double m_peakDb = 0.0;
    bool m_valid = false;
};

} // namespace emg
