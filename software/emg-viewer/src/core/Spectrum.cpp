#include "Spectrum.h"

#include "Fft.h"

#include <algorithm>
#include <cmath>

namespace emg {
namespace {

constexpr double kPi = 3.14159265358979323846;

// Bounds on the transform length. 256 keeps bin width sane at low sample rates;
// 4096 caps the per-repaint cost, and at 1000 SPS already gives 0.24 Hz bins -
// far finer than anything meaningful in a stochastic EMG spectrum.
constexpr std::size_t kMinFft = 256;
constexpr std::size_t kMaxFft = 4096;

} // namespace

void computeSpectrum(const float *samples, std::size_t count, double sampleRate,
                     std::size_t fftSize, SpectrumResult &out,
                     double bandLoHz, double bandHiHz)
{
    out.valid = false;
    out.segments = 0;
    out.medianFrequencyHz = 0.0;
    out.peakFrequencyHz = 0.0;

    if (samples == nullptr || sampleRate <= 0.0 || count < kMinFft) {
        out.magnitudeDb.clear();
        return;
    }

    // Largest power of two that fits, clamped.
    std::size_t n = fftSize != 0 ? fftSize : count;
    n = std::min(n, count);
    n = floorPowerOfTwo(n);
    n = std::clamp(n, kMinFft, kMaxFft);
    if (n > count) {
        n = floorPowerOfTwo(count);
    }
    if (n < kMinFft) {
        out.magnitudeDb.clear();
        return;
    }

    const std::size_t bins = n / 2 + 1;
    out.fftSize = n;
    out.binHz = sampleRate / static_cast<double>(n);
    out.magnitudeDb.assign(bins, 0.0f);

    // Remove DC across the whole record: a gain-6 channel carries millivolts of
    // electrode offset that would otherwise swamp the low bins.
    double mean = 0.0;
    for (std::size_t i = 0; i < count; ++i) {
        mean += samples[i];
    }
    mean /= static_cast<double>(count);

    // Hann window and its coherent gain (sum of the window). Normalising by the
    // sum rather than by n is what makes a sine read back at its true amplitude.
    std::vector<double> window(n);
    double windowSum = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        window[i] = 0.5 * (1.0 - std::cos(2.0 * kPi * static_cast<double>(i) /
                                          static_cast<double>(n - 1)));
        windowSum += window[i];
    }

    std::vector<double> power(bins, 0.0);
    std::vector<std::complex<double>> buf(n);

    const std::size_t hop = n / 2; // 50% overlap
    int segments = 0;

    for (std::size_t start = 0; start + n <= count; start += hop) {
        for (std::size_t i = 0; i < n; ++i) {
            buf[i] = std::complex<double>(
                (static_cast<double>(samples[start + i]) - mean) * window[i], 0.0);
        }

        fftInPlace(buf.data(), n, false);

        for (std::size_t k = 0; k < bins; ++k) {
            // Single-sided: interior bins carry the energy of their negative-
            // frequency twin too, DC and Nyquist do not.
            const double twoSided = std::abs(buf[k]);
            const double scale = (k == 0 || k == bins - 1) ? 1.0 : 2.0;
            const double amp = scale * twoSided / windowSum;
            power[k] += amp * amp;
        }
        ++segments;
    }

    if (segments == 0) {
        out.magnitudeDb.clear();
        return;
    }

    // Average the periodograms, then convert to amplitude and dB.
    std::vector<double> amplitude(bins);
    for (std::size_t k = 0; k < bins; ++k) {
        amplitude[k] = std::sqrt(power[k] / segments);
        out.magnitudeDb[k] = static_cast<float>(
            20.0 * std::log10(std::max(amplitude[k], kMinAmplitudeUv)));
    }

    // Median and peak frequency over the surface-EMG band only - see the header.
    const auto loBin = static_cast<std::size_t>(std::ceil(bandLoHz / out.binHz));
    auto hiBin = static_cast<std::size_t>(std::floor(bandHiHz / out.binHz));
    hiBin = std::min(hiBin, bins - 1);

    if (loBin < hiBin) {
        double total = 0.0;
        double peak = -1.0;
        std::size_t peakBin = loBin;

        for (std::size_t k = loBin; k <= hiBin; ++k) {
            const double p = amplitude[k] * amplitude[k];
            total += p;
            if (p > peak) {
                peak = p;
                peakBin = k;
            }
        }

        out.peakFrequencyHz = static_cast<double>(peakBin) * out.binHz;

        if (total > 0.0) {
            double cumulative = 0.0;
            for (std::size_t k = loBin; k <= hiBin; ++k) {
                cumulative += amplitude[k] * amplitude[k];
                if (cumulative >= total * 0.5) {
                    out.medianFrequencyHz = static_cast<double>(k) * out.binHz;
                    break;
                }
            }
        }
    }

    out.segments = segments;
    out.valid = true;
}

} // namespace emg
