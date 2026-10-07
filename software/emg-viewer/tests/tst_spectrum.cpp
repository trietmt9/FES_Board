// FFT and Welch spectrum.
//
// A spectrum plot is trusted implicitly - nobody eyeballs a peak and asks
// whether the axis is right - so the numbers get checked against signals whose
// answer is known in closed form: a sine lands on its own frequency, at its own
// amplitude, and Parseval's theorem holds.

#include <QTest>

#include "core/Fft.h"
#include "core/Spectrum.h"

#include <cmath>
#include <complex>
#include <vector>

using namespace emg;

namespace {
constexpr double kPi = 3.14159265358979323846;

std::vector<float> sine(std::size_t n, double freqHz, double rateHz,
                        double amplitude, double offset = 0.0)
{
    std::vector<float> v(n);
    for (std::size_t i = 0; i < n; ++i) {
        v[i] = static_cast<float>(
            offset + amplitude * std::sin(2.0 * kPi * freqHz * i / rateHz));
    }
    return v;
}
} // namespace

class TstSpectrum : public QObject {
    Q_OBJECT

private slots:
    void powerOfTwoHelpers();
    void fftOfDeltaIsFlat();
    void fftOfConstantIsDcOnly();
    void fftMatchesNaiveDft();
    void inverseUndoesForward();
    void satisfiesParseval();
    void medianFollowsTheAnalysisBand();

    void findsSinePeakAtCorrectFrequency();
    void findsSinePeakAtCorrectFrequency_data();
    void reportsCorrectAmplitude();
    void rejectsDcOffset();
    void averagingReducesNoiseVariance();
    void medianFrequencySplitsBandPower();
    void handlesTooFewSamples();
};

// ------------------------------------------------------------------- helpers

void TstSpectrum::powerOfTwoHelpers()
{
    QVERIFY(isPowerOfTwo(1));
    QVERIFY(isPowerOfTwo(1024));
    QVERIFY(!isPowerOfTwo(0));
    QVERIFY(!isPowerOfTwo(1000));

    QCOMPARE(floorPowerOfTwo(1024), std::size_t(1024));
    QCOMPARE(floorPowerOfTwo(1025), std::size_t(1024));
    QCOMPARE(floorPowerOfTwo(1023), std::size_t(512));
    QCOMPARE(floorPowerOfTwo(1), std::size_t(1));
    QCOMPARE(floorPowerOfTwo(0), std::size_t(0));
}

// ----------------------------------------------------------------------- FFT

void TstSpectrum::fftOfDeltaIsFlat()
{
    // A unit impulse contains every frequency equally.
    std::vector<std::complex<double>> a(64, {0.0, 0.0});
    a[0] = {1.0, 0.0};

    fftInPlace(a.data(), a.size());

    for (const auto &c : a) {
        QVERIFY(std::abs(std::abs(c) - 1.0) < 1e-12);
    }
}

void TstSpectrum::fftOfConstantIsDcOnly()
{
    std::vector<std::complex<double>> a(64, {1.0, 0.0});
    fftInPlace(a.data(), a.size());

    QVERIFY(std::abs(a[0].real() - 64.0) < 1e-9);
    for (std::size_t k = 1; k < a.size(); ++k) {
        QVERIFY(std::abs(a[k]) < 1e-9);
    }
}

void TstSpectrum::fftMatchesNaiveDft()
{
    // The definitive check: compare against the O(n^2) definition.
    constexpr std::size_t n = 64;
    std::vector<std::complex<double>> in(n);
    for (std::size_t i = 0; i < n; ++i) {
        in[i] = {std::sin(0.3 * i) + 0.5 * std::cos(1.1 * i), 0.2 * std::sin(0.7 * i)};
    }

    std::vector<std::complex<double>> naive(n, {0.0, 0.0});
    for (std::size_t k = 0; k < n; ++k) {
        for (std::size_t j = 0; j < n; ++j) {
            const double ang = -2.0 * kPi * k * j / n;
            naive[k] += in[j] * std::complex<double>(std::cos(ang), std::sin(ang));
        }
    }

    std::vector<std::complex<double>> fast = in;
    fftInPlace(fast.data(), n);

    for (std::size_t k = 0; k < n; ++k) {
        QVERIFY(std::abs(fast[k] - naive[k]) < 1e-9);
    }
}

void TstSpectrum::inverseUndoesForward()
{
    constexpr std::size_t n = 256;
    std::vector<std::complex<double>> original(n);
    for (std::size_t i = 0; i < n; ++i) {
        original[i] = {std::sin(0.13 * i), std::cos(0.07 * i)};
    }

    std::vector<std::complex<double>> round = original;
    fftInPlace(round.data(), n, false);
    fftInPlace(round.data(), n, true);

    for (std::size_t i = 0; i < n; ++i) {
        QVERIFY(std::abs(round[i] - original[i]) < 1e-10);
    }
}

void TstSpectrum::satisfiesParseval()
{
    // Energy is conserved: sum|x[n]|^2 == (1/N) sum|X[k]|^2.
    constexpr std::size_t n = 128;
    std::vector<std::complex<double>> a(n);
    double timeEnergy = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        a[i] = {std::sin(0.21 * i) * 3.0, 0.0};
        timeEnergy += std::norm(a[i]);
    }

    fftInPlace(a.data(), n);

    double freqEnergy = 0.0;
    for (const auto &c : a) {
        freqEnergy += std::norm(c);
    }
    freqEnergy /= n;

    QVERIFY(std::abs(timeEnergy - freqEnergy) / timeEnergy < 1e-12);
}

// ------------------------------------------------------------------ spectrum

void TstSpectrum::findsSinePeakAtCorrectFrequency_data()
{
    QTest::addColumn<double>("freq");

    // Across the surface-EMG band, including the mains frequency the UI marks.
    QTest::newRow("30 Hz") << 30.0;
    QTest::newRow("50 Hz mains") << 50.0;
    QTest::newRow("120 Hz") << 120.0;
    QTest::newRow("400 Hz") << 400.0;
}

void TstSpectrum::findsSinePeakAtCorrectFrequency()
{
    QFETCH(double, freq);

    constexpr double rate = 1000.0;
    const auto x = sine(8192, freq, rate, 100.0);

    SpectrumResult s;
    computeSpectrum(x.data(), x.size(), rate, 0, s);

    QVERIFY(s.valid);
    QVERIFY(s.segments > 1); // Welch actually averaged something

    // Peak bin must be the sine's own frequency, within one bin.
    QVERIFY2(std::abs(s.peakFrequencyHz - freq) <= s.binHz,
             qPrintable(QStringLiteral("peak %1 Hz, expected %2 Hz, bin %3 Hz")
                            .arg(s.peakFrequencyHz).arg(freq).arg(s.binHz)));
}

void TstSpectrum::reportsCorrectAmplitude()
{
    // Coherent-gain normalisation means a 100 uV sine must read 100 uV
    // (40 dB re 1 uV), independent of window and transform length.
    constexpr double rate = 1000.0;
    constexpr double amp = 100.0;
    const auto x = sine(8192, 125.0, rate, amp);

    SpectrumResult s;
    computeSpectrum(x.data(), x.size(), rate, 0, s);
    QVERIFY(s.valid);

    const auto bin = static_cast<std::size_t>(std::lround(125.0 / s.binHz));
    QVERIFY(bin < s.magnitudeDb.size());

    const double expectedDb = 20.0 * std::log10(amp);
    QVERIFY2(std::abs(s.magnitudeDb[bin] - expectedDb) < 0.5,
             qPrintable(QStringLiteral("got %1 dB, expected %2 dB")
                            .arg(double(s.magnitudeDb[bin])).arg(expectedDb)));
}

void TstSpectrum::rejectsDcOffset()
{
    // A gain-6 EMG channel sits on millivolts of electrode offset. If that is
    // not removed it dominates the DC bin and leaks across the low end, which
    // would swamp the very band the median frequency is computed over.
    constexpr double rate = 1000.0;
    const auto withOffset = sine(8192, 100.0, rate, 50.0, 5000.0);
    const auto without = sine(8192, 100.0, rate, 50.0, 0.0);

    SpectrumResult a;
    SpectrumResult b;
    computeSpectrum(withOffset.data(), withOffset.size(), rate, 0, a);
    computeSpectrum(without.data(), without.size(), rate, 0, b);

    QVERIFY(a.valid && b.valid);

    // The 5 mV offset must not change the 100 Hz component at all.
    const auto bin = static_cast<std::size_t>(std::lround(100.0 / a.binHz));
    QVERIFY(std::abs(a.magnitudeDb[bin] - b.magnitudeDb[bin]) < 0.1);

    // And it must not survive as a DC spike.
    QVERIFY2(a.magnitudeDb[0] < a.magnitudeDb[bin],
             qPrintable(QStringLiteral("DC %1 dB vs signal %2 dB")
                            .arg(double(a.magnitudeDb[0]))
                            .arg(double(a.magnitudeDb[bin]))));
}

void TstSpectrum::averagingReducesNoiseVariance()
{
    // The justification for Welch over a single periodogram: on a stochastic
    // signal, averaging must visibly flatten the estimate.
    constexpr double rate = 1000.0;
    constexpr std::size_t n = 16384;

    std::vector<float> noise(n);
    unsigned state = 987654321u;
    for (std::size_t i = 0; i < n; ++i) {
        state = state * 1103515245u + 12345u;
        noise[i] = static_cast<float>(
            (double((state >> 16) & 0x7FFF) / 32768.0 - 0.5) * 200.0);
    }

    SpectrumResult many;
    computeSpectrum(noise.data(), n, rate, 1024, many);

    SpectrumResult few;
    computeSpectrum(noise.data(), 2048, rate, 1024, few);

    QVERIFY(many.valid && few.valid);
    QVERIFY(many.segments > few.segments);

    auto spread = [](const SpectrumResult &s) {
        double mean = 0.0;
        for (std::size_t k = 1; k + 1 < s.magnitudeDb.size(); ++k) {
            mean += s.magnitudeDb[k];
        }
        mean /= double(s.magnitudeDb.size() - 2);

        double var = 0.0;
        for (std::size_t k = 1; k + 1 < s.magnitudeDb.size(); ++k) {
            const double d = s.magnitudeDb[k] - mean;
            var += d * d;
        }
        return std::sqrt(var / double(s.magnitudeDb.size() - 2));
    };

    QVERIFY2(spread(many) < spread(few),
             qPrintable(QStringLiteral("%1 segments: %2 dB spread; %3 segments: %4 dB")
                            .arg(many.segments).arg(spread(many))
                            .arg(few.segments).arg(spread(few))));
}

void TstSpectrum::medianFrequencySplitsBandPower()
{
    // Two equal tones inside the band put the median between them.
    constexpr double rate = 1000.0;
    constexpr std::size_t n = 8192;

    const auto lo = sine(n, 60.0, rate, 100.0);
    const auto hi = sine(n, 300.0, rate, 100.0);

    std::vector<float> mixed(n);
    for (std::size_t i = 0; i < n; ++i) {
        mixed[i] = lo[i] + hi[i];
    }

    SpectrumResult s;
    computeSpectrum(mixed.data(), n, rate, 0, s);
    QVERIFY(s.valid);

    QVERIFY2(s.medianFrequencyHz > 55.0 && s.medianFrequencyHz < 305.0,
             qPrintable(QStringLiteral("median %1 Hz").arg(s.medianFrequencyHz)));

    // Weighting the low tone far more heavily must drag the median down to it.
    for (std::size_t i = 0; i < n; ++i) {
        mixed[i] = 10.0f * lo[i] + hi[i];
    }
    SpectrumResult weighted;
    computeSpectrum(mixed.data(), n, rate, 0, weighted);
    QVERIFY(weighted.valid);
    QVERIFY(weighted.medianFrequencyHz < s.medianFrequencyHz);
    QVERIFY(std::abs(weighted.medianFrequencyHz - 60.0) < 10.0);
}

void TstSpectrum::handlesTooFewSamples()
{
    constexpr double rate = 1000.0;
    const auto x = sine(64, 100.0, rate, 10.0);

    SpectrumResult s;
    computeSpectrum(x.data(), x.size(), rate, 0, s);
    QVERIFY(!s.valid); // below the 256-point minimum
    QVERIFY(s.magnitudeDb.empty());

    computeSpectrum(nullptr, 0, rate, 0, s);
    QVERIFY(!s.valid);

    computeSpectrum(x.data(), x.size(), 0.0, 0, s); // zero sample rate
    QVERIFY(!s.valid);
}

void TstSpectrum::medianFollowsTheAnalysisBand()
{
    // An ECG-shaped spectrum: nearly all the power at 10 Hz, plus a little
    // broadband interference up at 120 Hz. Measured over the EMG band the
    // median reports the interference; measured over the ECG band it reports
    // the signal. Same data, and only one of the two answers is about the heart.
    constexpr double rate = 1000.0;
    constexpr std::size_t n = 4096;
    std::vector<float> x(n);
    for (std::size_t i = 0; i < n; ++i) {
        const double t = double(i) / rate;
        x[i] = static_cast<float>(1000.0 * std::sin(2.0 * M_PI * 10.0 * t)
                                 +  40.0 * std::sin(2.0 * M_PI * 120.0 * t));
    }

    emg::SpectrumResult emgBand;
    emg::computeSpectrum(x.data(), n, rate, 0, emgBand, 20.0, 450.0);
    QVERIFY(emgBand.valid);

    emg::SpectrumResult ecgBand;
    emg::computeSpectrum(x.data(), n, rate, 0, ecgBand, 0.5, 40.0);
    QVERIFY(ecgBand.valid);

    // EMG band cannot see the 10 Hz component at all - it starts above it.
    QVERIFY2(emgBand.medianFrequencyHz > 100.0,
             qPrintable(QStringLiteral("EMG band median %1 Hz")
                            .arg(emgBand.medianFrequencyHz)));

    // ECG band lands on the real signal.
    QVERIFY2(ecgBand.medianFrequencyHz > 8.0 && ecgBand.medianFrequencyHz < 12.0,
             qPrintable(QStringLiteral("ECG band median %1 Hz, expected ~10")
                            .arg(ecgBand.medianFrequencyHz)));
}

QTEST_APPLESS_MAIN(TstSpectrum)
#include "tst_spectrum.moc"
