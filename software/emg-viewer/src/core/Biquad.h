#pragma once

// Second-order IIR section, plus RBJ-cookbook designers.
//
// Direct Form II transposed: two state variables, one multiply-accumulate per
// coefficient, and better numerical behaviour than Direct Form I at the low
// corner frequencies we need here (a 20 Hz highpass at 4 kSPS puts poles very
// close to z = 1, where DF-I in single precision starts to lose the plot).
// State is kept in double for the same reason.

#include <cmath>

namespace emg {

struct Biquad {
    // Normalised so a0 == 1.
    double b0 = 1.0, b1 = 0.0, b2 = 0.0;
    double a1 = 0.0, a2 = 0.0;

    double z1 = 0.0, z2 = 0.0;

    inline float process(float x) noexcept
    {
        const double in = static_cast<double>(x);
        const double y = b0 * in + z1;
        z1 = b1 * in - a1 * y + z2;
        z2 = b2 * in - a2 * y;
        return static_cast<float>(y);
    }

    void reset() noexcept { z1 = z2 = 0.0; }

    /// Set the state to what it would be after an infinite run of constant
    /// input @p x, so the first real sample sees no start-up transient.
    ///
    /// Steady state of Direct Form II transposed for constant input x:
    ///     y  = x * (b0 + b1 + b2) / (1 + a1 + a2)     (the DC gain)
    ///     z2 = b2*x - a2*y
    ///     z1 = b1*x - a1*y + z2
    /// For a high-pass the DC gain is exactly 0, so y = 0 and the section
    /// starts silent however large the offset on the input is.
    void prime(double x) noexcept
    {
        const double y = x * (b0 + b1 + b2) / (1.0 + a1 + a2);
        z2 = b2 * x - a2 * y;
        z1 = b1 * x - a1 * y + z2;
    }

    /// Magnitude response at @p f, for tests and for plotting.
    double magnitudeAt(double f, double fs) const
    {
        const double w = 2.0 * M_PI * f / fs;
        const double cw = std::cos(w), sw = std::sin(w);
        const double c2w = std::cos(2 * w), s2w = std::sin(2 * w);

        const double numRe = b0 + b1 * cw + b2 * c2w;
        const double numIm = -(b1 * sw + b2 * s2w);
        const double denRe = 1.0 + a1 * cw + a2 * c2w;
        const double denIm = -(a1 * sw + a2 * s2w);

        return std::sqrt((numRe * numRe + numIm * numIm) /
                         (denRe * denRe + denIm * denIm));
    }
};

/// Butterworth Q values for a 4th-order cascade of two biquads.
inline constexpr double kButter4Q1 = 0.54119610;
inline constexpr double kButter4Q2 = 1.30656296;

Biquad designLowpass(double fc, double fs, double q = 0.70710678);
Biquad designHighpass(double fc, double fs, double q = 0.70710678);

/// Single-frequency notch. @p q sets the width: 30 keeps a 50 Hz notch about
/// 1.7 Hz wide at -3 dB, narrow enough to leave the EMG band essentially intact.
Biquad designNotch(double f0, double fs, double q = 30.0);

} // namespace emg
