#include "Biquad.h"

#include <algorithm>

namespace emg {
namespace {

// Guard against a corner at or above Nyquist, which would produce garbage
// coefficients rather than an obvious failure.
double clampCorner(double f, double fs)
{
    // Floor at 0.01 Hz: diagnostic ECG specifies a 0.05 Hz high-pass, and at
    // 4 kSPS that puts the poles very close to z = 1. The coefficients and
    // state are computed in double, which carries it comfortably.
    return std::clamp(f, 0.01, 0.45 * fs);
}

Biquad normalise(double b0, double b1, double b2, double a0, double a1, double a2)
{
    Biquad q;
    q.b0 = b0 / a0;
    q.b1 = b1 / a0;
    q.b2 = b2 / a0;
    q.a1 = a1 / a0;
    q.a2 = a2 / a0;
    return q;
}

} // namespace

Biquad designLowpass(double fc, double fs, double q)
{
    const double w0 = 2.0 * M_PI * clampCorner(fc, fs) / fs;
    const double cw = std::cos(w0), sw = std::sin(w0);
    const double alpha = sw / (2.0 * q);

    return normalise((1.0 - cw) / 2.0, 1.0 - cw, (1.0 - cw) / 2.0,
                     1.0 + alpha, -2.0 * cw, 1.0 - alpha);
}

Biquad designHighpass(double fc, double fs, double q)
{
    const double w0 = 2.0 * M_PI * clampCorner(fc, fs) / fs;
    const double cw = std::cos(w0), sw = std::sin(w0);
    const double alpha = sw / (2.0 * q);

    return normalise((1.0 + cw) / 2.0, -(1.0 + cw), (1.0 + cw) / 2.0,
                     1.0 + alpha, -2.0 * cw, 1.0 - alpha);
}

Biquad designNotch(double f0, double fs, double q)
{
    const double w0 = 2.0 * M_PI * clampCorner(f0, fs) / fs;
    const double cw = std::cos(w0), sw = std::sin(w0);
    const double alpha = sw / (2.0 * q);

    return normalise(1.0, -2.0 * cw, 1.0,
                     1.0 + alpha, -2.0 * cw, 1.0 - alpha);
}

} // namespace emg
