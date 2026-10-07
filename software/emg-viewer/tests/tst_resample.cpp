// Band-limited reconstruction (core/Resample.h).
//
// The point of these tests is that the interpolator is *correct*, not merely
// smooth: a curve that looks nice but sits away from the true waveform would be
// worse than the staircase it replaces, because it would be believable.

#include <QTest>

#include "core/Resample.h"

#include <cmath>
#include <vector>

using namespace emg;

namespace {
constexpr double kPi = 3.14159265358979323846;
}

class TstResample : public QObject {
    Q_OBJECT

private slots:
    void kernelIsUnitAtZeroAndZeroAtIntegers();
    void preservesConstantSignal();
    void passesThroughOriginalSamples();
    void reconstructsSineBetweenSamples();
    void reconstructsSineBetweenSamples_data();
    void beatsLinearInterpolationOnBandLimitedInput();
    void handlesDegenerateInputs();
};

void TstResample::kernelIsUnitAtZeroAndZeroAtIntegers()
{
    QVERIFY(qFuzzyCompare(lanczosKernel(0.0, 3), 1.0));

    // A sinc-family kernel must vanish at every other sample instant, which is
    // what makes the reconstruction pass through the original points.
    for (int n = 1; n < 3; ++n) {
        QVERIFY(std::abs(lanczosKernel(n, 3)) < 1e-12);
        QVERIFY(std::abs(lanczosKernel(-n, 3)) < 1e-12);
    }

    // Outside the window it is identically zero.
    QCOMPARE(lanczosKernel(3.0, 3), 0.0);
    QCOMPARE(lanczosKernel(4.5, 3), 0.0);
}

void TstResample::preservesConstantSignal()
{
    // DC gain must be exactly 1: a flat input has to stay flat, or a resting
    // channel would appear to ripple.
    const std::vector<float> in(64, 42.0f);
    std::vector<float> out(500);

    lanczosResample(in.data(), in.size(), out.data(), out.size());

    for (float v : out) {
        QVERIFY(std::abs(v - 42.0f) < 1e-4f);
    }
}

void TstResample::passesThroughOriginalSamples()
{
    // Where an output point lands exactly on an input sample, it must return
    // that sample untouched - the interpolator adds points, it never moves the
    // measured ones.
    constexpr std::size_t n = 33;
    std::vector<float> in(n);
    for (std::size_t i = 0; i < n; ++i) {
        in[i] = static_cast<float>(std::sin(2.0 * kPi * 3.0 * i / (n - 1)));
    }

    // outCount - 1 an exact multiple of inCount - 1 => every 4th output point
    // coincides with an input sample.
    constexpr std::size_t out_n = 4 * (n - 1) + 1;
    std::vector<float> out(out_n);
    lanczosResample(in.data(), n, out.data(), out_n);

    for (std::size_t i = 0; i < n; ++i) {
        QVERIFY(std::abs(out[i * 4] - in[i]) < 1e-4f);
    }
}

void TstResample::reconstructsSineBetweenSamples_data()
{
    QTest::addColumn<double>("cyclesPerWindow");

    // Well below Nyquist, mid-band, and approaching it. EMG at 1000 SPS lives
    // in the first two rows; the third is where any interpolator degrades.
    QTest::newRow("2 cycles") << 2.0;
    QTest::newRow("8 cycles") << 8.0;
    QTest::newRow("20 cycles") << 20.0;
}

void TstResample::reconstructsSineBetweenSamples()
{
    QFETCH(double, cyclesPerWindow);

    // Sample a sine, upsample 8x, and compare against the true sine at the
    // interpolated instants. This is the real claim: the curve drawn between
    // samples is the actual waveform, not a plausible-looking guess.
    constexpr std::size_t n = 128;
    constexpr std::size_t out_n = 8 * (n - 1) + 1;

    std::vector<float> in(n);
    for (std::size_t i = 0; i < n; ++i) {
        in[i] = static_cast<float>(
            std::sin(2.0 * kPi * cyclesPerWindow * i / (n - 1)));
    }

    std::vector<float> out(out_n);
    lanczosResample(in.data(), n, out.data(), out_n);

    // Ignore a few points at each end: the kernel is clamped there, so edge
    // error is expected and is why the renderer never relies on the extremes.
    constexpr std::size_t guard = 24;
    double worst = 0.0;

    for (std::size_t j = guard; j < out_n - guard; ++j) {
        const double t = static_cast<double>(j) / (out_n - 1);
        const double truth = std::sin(2.0 * kPi * cyclesPerWindow * t);
        worst = std::max(worst, std::abs(out[j] - truth));
    }

    // 20 cycles over 128 samples is ~0.31 of Nyquist; a 6-tap Lanczos holds
    // well inside 5% there, and far better lower down.
    QVERIFY2(worst < 0.05, qPrintable(QStringLiteral("worst error %1").arg(worst)));
}

void TstResample::beatsLinearInterpolationOnBandLimitedInput()
{
    // The practical justification. Straight chords between samples undershoot
    // every peak; band-limited reconstruction does not.
    constexpr std::size_t n = 64;
    constexpr std::size_t out_n = 8 * (n - 1) + 1;
    constexpr double cycles = 12.0;

    std::vector<float> in(n);
    for (std::size_t i = 0; i < n; ++i) {
        in[i] = static_cast<float>(std::sin(2.0 * kPi * cycles * i / (n - 1)));
    }

    std::vector<float> lanczos(out_n);
    lanczosResample(in.data(), n, lanczos.data(), out_n);

    const double scale = static_cast<double>(n - 1) / (out_n - 1);
    double lanczosErr = 0.0;
    double linearErr = 0.0;

    constexpr std::size_t guard = 24;
    for (std::size_t j = guard; j < out_n - guard; ++j) {
        const double pos = j * scale;
        const auto i0 = static_cast<std::size_t>(pos);
        const double frac = pos - i0;
        const double linear =
            in[i0] * (1.0 - frac) + in[std::min(i0 + 1, n - 1)] * frac;

        const double truth = std::sin(2.0 * kPi * cycles * pos / (n - 1));
        lanczosErr = std::max(lanczosErr, std::abs(lanczos[j] - truth));
        linearErr = std::max(linearErr, std::abs(linear - truth));
    }

    QVERIFY2(lanczosErr < linearErr * 0.5,
             qPrintable(QStringLiteral("lanczos %1 vs linear %2")
                            .arg(lanczosErr)
                            .arg(linearErr)));
}

void TstResample::handlesDegenerateInputs()
{
    std::vector<float> out(16, -1.0f);

    // Null/empty input must not write or crash.
    lanczosResample(nullptr, 0, out.data(), out.size());
    QCOMPARE(out[0], -1.0f);

    // A single sample fills the output with itself rather than dividing by zero.
    const float one = 7.0f;
    lanczosResample(&one, 1, out.data(), out.size());
    for (float v : out) {
        QCOMPARE(v, 7.0f);
    }

    // Single output point.
    const std::vector<float> in{1.0f, 2.0f, 3.0f};
    float single = 0.0f;
    lanczosResample(in.data(), in.size(), &single, 1);
    QVERIFY(std::isfinite(single));
}

QTEST_APPLESS_MAIN(TstResample)
#include "tst_resample.moc"
