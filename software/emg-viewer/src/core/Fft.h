#pragma once

// Radix-2 Cooley-Tukey FFT.
//
// Small and self-contained on purpose: the transform sizes here are 256-4096
// points on four channels at display rate, a few hundred microseconds of work,
// so pulling in FFTW or KissFFT would cost more in build and packaging than it
// saves. If the analysis ever grows (spectrograms, long averages, non-power-of-2
// sizes) that trade flips and a real library is the right answer.
//
// This is the *legitimate* use of a Fourier transform in this application:
// showing what frequencies the signal contains. It is unrelated to
// core/Resample.h, which reconstructs the time-domain waveform between samples -
// a transform cannot make sampled data continuous.

#include <complex>
#include <cstddef>

namespace emg {

/// True if @p n is a power of two and non-zero.
constexpr bool isPowerOfTwo(std::size_t n)
{
    return n != 0 && (n & (n - 1)) == 0;
}

/// Largest power of two <= @p n, or 0 if n == 0.
constexpr std::size_t floorPowerOfTwo(std::size_t n)
{
    if (n == 0) {
        return 0;
    }
    // Compare against n/2 rather than doubling p and testing p <= n, so the
    // shift can never overflow on the last iteration.
    std::size_t p = 1;
    while (p <= n / 2) {
        p <<= 1u;
    }
    return p;
}

/**
 * @brief In-place complex FFT. @p n must be a power of two.
 *
 * Decimation-in-time with a bit-reversal permutation up front. Twiddles are
 * advanced by repeated multiplication, which drifts by roughly 1e-13 over a
 * 4096-point transform - far below the noise floor of anything being displayed.
 *
 * @param inverse run the inverse transform, including the 1/n scaling.
 */
void fftInPlace(std::complex<double> *a, std::size_t n, bool inverse = false);

} // namespace emg
