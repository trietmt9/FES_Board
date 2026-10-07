#pragma once

// Amplitude spectrum of an EMG channel, by Welch's method.
//
// A single FFT of a stretch of EMG is close to useless to look at: EMG is a
// stochastic signal, so one periodogram has ~100% variance per bin and the plot
// is a hedge of noise no matter how long the record. Welch's method - split into
// overlapping windowed segments, transform each, average the power - trades
// frequency resolution for variance and is what makes the spectrum readable.
//
// Hann window at 50% overlap: the standard pairing. Hann's -31 dB sidelobes stop
// a large low-frequency component (motion artifact, or residual DC) from
// smearing across the whole band, and 50% overlap means the window's tapered
// ends do not throw away data.

#include <cstddef>
#include <vector>

namespace emg {

struct SpectrumResult {
    /// Single-sided amplitude in dB re 1 uV, one entry per bin, DC first.
    std::vector<float> magnitudeDb;

    double binHz = 0.0;         ///< frequency step between bins
    std::size_t fftSize = 0;    ///< transform length actually used
    int segments = 0;           ///< periodograms averaged

    /// Frequency below which half the band power lies - the classic surface-EMG
    /// fatigue index, which falls as a muscle tires. Computed over
    /// [kBandLowHz, kBandHighHz], not the full spectrum: below 20 Hz is motion
    /// artifact and electrode drift, above 450 Hz there is no EMG left, and
    /// including either would move the median for reasons unrelated to the
    /// muscle.
    double medianFrequencyHz = 0.0;

    /// Frequency of the largest bin within the same band.
    double peakFrequencyHz = 0.0;

    bool valid = false;
};

/// Surface-EMG band of interest, per SENIAM.
// Default analysis band: surface EMG per SENIAM. Median frequency over this
// band is the standard muscle-fatigue metric.
//
// It is the WRONG band for an ECG, and not marginally: an ECG's energy lies
// almost entirely below 20 Hz, so measuring from 20 Hz up excludes the signal
// and reports the noise floor above it. An ECG measured this way returns a
// median of 70-180 Hz that fluctuates with whatever interference is present -
// a number about the noise, presented as though it were about the heart.
// Callers analysing ECG must pass the ECG band instead.
inline constexpr double kBandLowHz = 20.0;
inline constexpr double kBandHighHz = 450.0;

/// Floor for the dB conversion, so silence plots as a flat line rather than
/// diverging to negative infinity.
inline constexpr double kMinAmplitudeUv = 1e-4;

/**
 * @brief Welch amplitude spectrum of @p count samples in microvolts.
 *
 * @param fftSize desired transform length; reduced to the largest power of two
 *                that fits @p count. Pass 0 to choose automatically.
 *
 * The mean is removed before transforming - a gain-6 EMG channel sits on
 * several millivolts of electrode offset, which would otherwise dominate the
 * DC bin and leak across the low end.
 *
 * Scaling is coherent-gain normalised, so a pure sine of amplitude A microvolts
 * peaks at A (that is, 20*log10(A) dB) regardless of window or transform length.
 */
/// @param bandLoHz,bandHiHz  band over which the median and peak are found.
///        Defaults to the surface-EMG band; pass the signal's own band when
///        analysing anything else, or the result describes the noise.
void computeSpectrum(const float *samples, std::size_t count, double sampleRate,
                     std::size_t fftSize, SpectrumResult &out,
                     double bandLoHz = kBandLowHz,
                     double bandHiHz = kBandHighHz);

} // namespace emg
