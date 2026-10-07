#include "SpectrumEngine.h"

#include "Fft.h"

#include <algorithm>
#include <cmath>
#include <complex>

namespace emg {

namespace {
constexpr double kPi = 3.14159265358979323846;

// Weight of the NEW frame in the exponential average - spec section 6.2.
constexpr double kWeightLow = 0.6;
constexpr double kWeightMedium = 0.3;
constexpr double kWeightHigh = 0.12;

// Smoothing of the axis top: 0.92 old / 0.08 new - spec section 6.2.
constexpr double kAxisKeep = 0.92;
constexpr double kAxisTake = 0.08;

double toDb(double power)
{
    return 10.0 * std::log10(std::max(power, SpectrumEngine::kPowerFloor));
}
} // namespace

double SpectrumEngine::weightFor(Averaging a)
{
    switch (a) {
    case Averaging::Low:
        return kWeightLow;
    case Averaging::High:
        return kWeightHigh;
    case Averaging::Medium:
        break;
    }
    return kWeightMedium;
}

void SpectrumEngine::configure(const Config &cfg)
{
    m_cfg = cfg;
    if (m_cfg.sampleRateHz <= 0.0) {
        m_cfg.sampleRateHz = 1.0;
    }

    const std::size_t half = kFftSize / 2;
    const auto wanted = static_cast<std::size_t>(m_cfg.displayMaxHz / binHz()) + 1;
    m_bins = std::min(wanted, half + 1);

    // Periodic Hann: the right one for spectral analysis (the symmetric form is
    // for filter design and leaks a little more).
    m_window.resize(kFftSize);
    for (std::size_t n = 0; n < kFftSize; ++n) {
        m_window[n] = 0.5 - 0.5 * std::cos(2.0 * kPi * double(n) / double(kFftSize));
    }

    reset();
}

void SpectrumEngine::reset()
{
    m_avgPower.assign(kFftSize / 2 + 1, 0.0);
    m_db.assign(m_bins, static_cast<float>(toDb(0.0)));
    m_smoothedMax = 0.0;
    m_peakHz = 0.0;
    m_peakDb = 0.0;
    m_valid = false;
}

void SpectrumEngine::update(const float *samples, Averaging averaging)
{
    if (!samples || m_window.empty()) {
        return;
    }

    // STEP 1: remove the mean. A DC-coupled channel sits on millivolts of
    // electrode offset, which would otherwise own bin 0 and leak into bin 1-2.
    double mean = 0.0;
    for (std::size_t n = 0; n < kFftSize; ++n) {
        mean += samples[n];
    }
    mean /= double(kFftSize);

    // STEP 2: window and transform.
    std::vector<std::complex<double>> x(kFftSize);
    for (std::size_t n = 0; n < kFftSize; ++n) {
        x[n] = (double(samples[n]) - mean) * m_window[n];
    }
    fftInPlace(x.data(), kFftSize);

    // STEP 3: power = |X|^2 / N, folded into the exponential average. The first
    // frame seeds the average so the display does not ramp up from silence.
    const double a = weightFor(averaging);
    for (std::size_t k = 0; k <= kFftSize / 2; ++k) {
        const double p = std::norm(x[k]) / double(kFftSize);
        m_avgPower[k] = m_valid ? (1.0 - a) * m_avgPower[k] + a * p : p;
    }

    // STEP 4: dB for the display range, and the maximum that drives the axis.
    double maxDb = toDb(0.0);
    for (std::size_t k = 0; k < m_bins; ++k) {
        const double db = toDb(m_avgPower[k]);
        m_db[k] = static_cast<float>(db);
        if (k > 0) {
            maxDb = std::max(maxDb, db);
        }
    }
    m_smoothedMax = m_valid ? kAxisKeep * m_smoothedMax + kAxisTake * maxDb : maxDb;

    // STEP 5: the peak. Largest bin at or above the search floor, then a
    // parabola through it and its neighbours. A Hann-windowed tone spreads over
    // three bins, so without this the answer is only good to half a bin - 0.24
    // Hz at 250 Hz, 1 Hz at 1 kHz, which is more than the figure being read.
    m_peakHz = 0.0;
    m_peakDb = toDb(0.0);
    const double df = binHz();
    std::size_t best = 0;
    double bestPower = -1.0;
    for (std::size_t k = 1; k < m_bins; ++k) {
        if (double(k) * df < m_cfg.peakMinHz) {
            continue;
        }
        if (m_avgPower[k] > bestPower) {
            bestPower = m_avgPower[k];
            best = k;
        }
    }
    // On silence every bin ties at the floor and "the largest" is just the first
    // eligible one. Reporting that as a peak would draw a marker on nothing.
    if (bestPower <= kPowerFloor) {
        best = 0;
    }
    if (best > 0) {
        double delta = 0.0;
        double peakDb = toDb(m_avgPower[best]);
        if (best + 1 < m_bins) {
            const double l = toDb(m_avgPower[best - 1]);
            const double c = toDb(m_avgPower[best]);
            const double r = toDb(m_avgPower[best + 1]);
            const double den = l - 2.0 * c + r;
            if (den < -1e-9) {                 // a genuine maximum, not a plateau
                delta = std::clamp(0.5 * (l - r) / den, -0.5, 0.5);
                peakDb = c - 0.25 * (l - r) * delta;
            }
        }
        m_peakHz = (double(best) + delta) * df;
        m_peakDb = peakDb;
    }

    m_valid = true;
}

double SpectrumEngine::axisTopDb(double gain) const
{
    const double g = gain > 0.0 ? gain : 1.0;
    return m_smoothedMax + kHeadroomDb - 20.0 * std::log10(g);
}

double SpectrumEngine::medianHz(double loHz, double hiHz) const
{
    if (!m_valid) {
        return 0.0;
    }
    const double df = binHz();
    const std::size_t half = kFftSize / 2;

    double total = 0.0;
    for (std::size_t k = 0; k <= half; ++k) {
        const double f = double(k) * df;
        if (f >= loHz && f <= hiHz) {
            total += m_avgPower[k];
        }
    }
    if (total <= 0.0) {
        return 0.0;
    }

    const double target = 0.5 * total;
    double cum = 0.0;
    for (std::size_t k = 0; k <= half; ++k) {
        const double f = double(k) * df;
        if (f < loHz || f > hiHz) {
            continue;
        }
        const double p = m_avgPower[k];
        if (cum + p >= target) {
            // Bin k covers [(k-0.5)df, (k+0.5)df): place the median inside it
            // in proportion to how far through the bin's power the half-way
            // point falls.
            const double t = p > 0.0 ? (target - cum) / p : 0.5;
            return (double(k) - 0.5 + t) * df;
        }
        cum += p;
    }
    return hiHz;
}

double SpectrumEngine::bandPower(double loHz, double hiHz) const
{
    if (!m_valid) {
        return 0.0;
    }
    const double df = binHz();
    double sum = 0.0;
    for (std::size_t k = 0; k <= kFftSize / 2; ++k) {
        const double f = double(k) * df;
        if (f >= loHz && f < hiHz) {
            sum += m_avgPower[k];
        }
    }
    return sum;
}

int SpectrumEngine::decimationFactor(double deviceRateHz, double targetRateHz)
{
    if (deviceRateHz <= 0.0 || targetRateHz <= 0.0) {
        return 1;
    }
    return std::max(1, static_cast<int>(std::lround(deviceRateHz / targetRateHz)));
}

void SpectrumEngine::decimateMean(const float *in, std::size_t nIn, int factor,
                                  float *out)
{
    const auto m = static_cast<std::size_t>(std::max(1, factor));
    const std::size_t nOut = nIn / m;
    for (std::size_t i = 0; i < nOut; ++i) {
        double s = 0.0;
        for (std::size_t j = 0; j < m; ++j) {
            s += in[i * m + j];
        }
        out[i] = static_cast<float>(s / double(m));
    }
}

} // namespace emg
