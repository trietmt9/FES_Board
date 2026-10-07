#pragma once

// Band-limited reconstruction for display.
//
// The sampling theorem says a band-limited signal sampled above its Nyquist
// rate has exactly ONE continuous waveform passing through those samples, given
// by the Whittaker-Shannon formula:
//
//     x(t) = SUM x[n] * sinc(t - n)
//
// That is the principled way to draw a smooth curve through sampled EMG, rather
// than a staircase (sample-and-hold) or straight chords between points. The
// ADS1298's sinc^3 decimation filter band-limits the signal before it ever
// reaches us, so the precondition genuinely holds here.
//
// It does not invent information. It reconstructs the unique band-limited curve
// the samples already imply - which is exactly why it is legitimate, and also
// why it cannot show you anything above Nyquist that was never captured.
//
// WHY NOT A LITERAL FFT. The frequency-domain equivalent (FFT -> zero-pad ->
// inverse FFT) is mathematically the same reconstruction, but it assumes the
// window is periodic. On a scrolling scope the left and right edges of the
// viewport are unrelated, so that assumption wraps one edge into the other and
// produces visible ringing at both ends of every frame. A windowed sinc applied
// directly in the time domain has no such edge coupling, costs no transform,
// and is what audio and instrument software actually use.
//
// The ideal sinc has infinite support, so it is truncated by a Lanczos window
// (a=3): six taps per output point, the standard practical compromise between
// sharpness and ringing.

#include <cmath>
#include <cstddef>

namespace emg {

/// Lanczos kernel, L(x) = sinc(x) * sinc(x/a) for |x| < a, else 0.
inline double lanczosKernel(double x, int a)
{
    if (x == 0.0) {
        return 1.0;
    }
    const double ax = std::abs(x);
    if (ax >= static_cast<double>(a)) {
        return 0.0;
    }
    constexpr double pi = 3.14159265358979323846;
    const double px = pi * x;
    return (std::sin(px) / px) * (std::sin(px / a) / (px / a));
}

/**
 * @brief Resample @p in to @p outCount points by windowed-sinc interpolation.
 *
 * Intended for the upsampling case (outCount > inCount) - drawing more pixel
 * columns than there are samples. Downsampling through this would alias; use
 * min/max decimation for that instead, which also preserves spikes that any
 * interpolator would smooth away.
 *
 * Out-of-range taps clamp to the end samples rather than treating the signal as
 * zero outside the window, which would otherwise dip both ends toward zero.
 */
inline void lanczosResample(const float *in, std::size_t inCount, float *out,
                            std::size_t outCount, int a = 3)
{
    if (in == nullptr || out == nullptr || inCount == 0 || outCount == 0) {
        return;
    }

    if (inCount == 1) {
        for (std::size_t j = 0; j < outCount; ++j) {
            out[j] = in[0];
        }
        return;
    }

    const double scale = (outCount > 1)
                             ? static_cast<double>(inCount - 1) / static_cast<double>(outCount - 1)
                             : 0.0;

    for (std::size_t j = 0; j < outCount; ++j) {
        const double t = static_cast<double>(j) * scale;
        const auto centre = static_cast<long>(std::floor(t));

        double acc = 0.0;
        double norm = 0.0;

        for (long k = centre - a + 1; k <= centre + a; ++k) {
            const double w = lanczosKernel(t - static_cast<double>(k), a);
            if (w == 0.0) {
                continue;
            }
            // Clamp rather than zero-extend past the ends.
            const long idx = k < 0 ? 0
                                   : (k >= static_cast<long>(inCount)
                                          ? static_cast<long>(inCount) - 1
                                          : k);
            acc += w * static_cast<double>(in[idx]);
            norm += w;
        }

        // Normalising by the tap sum keeps the DC gain at exactly 1, so a
        // constant input stays constant instead of rippling.
        out[j] = static_cast<float>(norm != 0.0 ? acc / norm : acc);
    }
}

} // namespace emg
