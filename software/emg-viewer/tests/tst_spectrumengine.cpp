// The live frequency-domain engine (core/SpectrumEngine.h).
//
// Every expectation here is a closed-form answer, not a snapshot of whatever the
// code printed: a tone exactly on a bin lands on that bin, the exponential
// average of two known powers is the weighted sum the spec's weights give, and
// so on. A test that records current output and asserts it back proves nothing.

#include <QTest>

#include "core/SpectrumEngine.h"

#include <cmath>
#include <vector>

using namespace emg;

namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr std::size_t kN = SpectrumEngine::kFftSize;

std::vector<float> tone(double fs, double freq, double amp, double dc = 0.0,
                        double phase = 0.0)
{
    std::vector<float> x(kN);
    for (std::size_t n = 0; n < kN; ++n) {
        x[n] = static_cast<float>(dc + amp * std::sin(2.0 * kPi * freq * double(n) / fs + phase));
    }
    return x;
}

std::vector<float> sum(const std::vector<float> &a, const std::vector<float> &b)
{
    std::vector<float> r(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        r[i] = a[i] + b[i];
    }
    return r;
}

SpectrumEngine make(double fs, double maxHz, double peakMin)
{
    SpectrumEngine e;
    SpectrumEngine::Config c;
    c.sampleRateHz = fs;
    c.displayMaxHz = maxHz;
    c.peakMinHz = peakMin;
    e.configure(c);
    return e;
}

double dbOf(double p) { return 10.0 * std::log10(p); }
} // namespace

class TstSpectrumEngine : public QObject {
    Q_OBJECT

private slots:
    void resolutionMatchesTheSpecFooter();
    void peakOnABinCentreLandsOnIt();
    void peakBetweenBinsIsInterpolated();
    void peakSearchIgnoresBelowItsFloor();
    void dcOffsetDoesNotLeak();
    void firstFrameSeedsTheAverage();
    void exponentialAverageUsesTheSpecWeights_data();
    void exponentialAverageUsesTheSpecWeights();
    void axisFollowsTheSmoothedMaximum();
    void gainRaisesTheCurve();
    void silenceStaysFiniteAndHasNoPeak();
    void medianFindsTheCentreOfPower();
    void medianHonoursItsBand();
    void bandPowersPartitionTheSpectrum();
    void decimationFactorsFollowTheDeviceRate();
    void decimateMeanAverages();
    void decimationRestoresSpecResolution();
    void blockMeanAttenuatesAboveTheNewNyquist();
    void displayBinsStopAtTheDisplayRange();
};

void TstSpectrumEngine::resolutionMatchesTheSpecFooter()
{
    // The three figures the spec prints: "FFT 512 pt . Hann . df 0.49 Hz" for
    // ECG at 250 Hz, 0.50 for EEG at 256 Hz, 1.95 for EMG at 1 kHz.
    QCOMPARE(QString::number(make(250.0, 40.0, 0.8).binHz(), 'f', 2), QStringLiteral("0.49"));
    QCOMPARE(QString::number(make(256.0, 45.0, 0.8).binHz(), 'f', 2), QStringLiteral("0.50"));
    QCOMPARE(QString::number(make(1000.0, 450.0, 20.0).binHz(), 'f', 2), QStringLiteral("1.95"));
}

void TstSpectrumEngine::peakOnABinCentreLandsOnIt()
{
    // 10 Hz at 256 Hz is exactly bin 20 (256/512 = 0.5 Hz per bin).
    SpectrumEngine e = make(256.0, 45.0, 0.8);
    e.update(tone(256.0, 10.0, 100.0).data(), SpectrumEngine::Averaging::Medium);
    QVERIFY(e.valid());
    QVERIFY2(std::abs(e.peakHz() - 10.0) < 0.01, qPrintable(QString::number(e.peakHz())));
}

void TstSpectrumEngine::peakBetweenBinsIsInterpolated()
{
    // A tone between two bins: the raw largest bin is up to half a bin off, which
    // at 0.49 Hz/bin is a quarter of a hertz - more than the readout resolves.
    struct Case { double fs, f; } cases[] = {
        {256.0, 10.25},   // exactly half a bin away from both neighbours
        {250.0, 10.2},    // the spec's own example label, "10.2 Hz"
        {256.0, 9.8},
        {282.5, 12.3},    // an ECG rate after decimating 1130 SPS by 4
    };
    for (const auto &c : cases) {
        SpectrumEngine e = make(c.fs, 45.0, 0.8);
        e.update(tone(c.fs, c.f, 100.0).data(), SpectrumEngine::Averaging::Medium);
        QVERIFY2(std::abs(e.peakHz() - c.f) < 0.08,
                 qPrintable(QStringLiteral("fs %1: tone %2 Hz read as %3 Hz")
                                .arg(c.fs).arg(c.f).arg(e.peakHz())));
    }
}

void TstSpectrumEngine::peakSearchIgnoresBelowItsFloor()
{
    // Baseline wander is larger than the signal of interest and sits below
    // 0.8 Hz; it must not be reported as the dominant frequency.
    //
    // 2x the signal's amplitude, not 100x: a Hann main lobe is only -6 dB one
    // bin from its tone, so an enormous wander tone leaks into the first bin
    // above ANY search floor and no floor could help. That is physics, not a
    // bug - and the live path removes the wander with the 0.5 Hz high-pass long
    // before it gets here.
    const auto wander = tone(256.0, 0.3, 200.0);
    const auto signal = tone(256.0, 10.0, 100.0);

    // Control: with no floor the wander wins. This is what makes the next
    // assertion mean something - without it, "found 10 Hz" could simply be
    // because the wander never dominated.
    SpectrumEngine unfloored = make(256.0, 45.0, 0.0);
    unfloored.update(sum(wander, signal).data(), SpectrumEngine::Averaging::Medium);
    QVERIFY2(unfloored.peakHz() < 0.8,
             qPrintable(QStringLiteral("control: no floor should pick the wander, got %1")
                            .arg(unfloored.peakHz())));

    SpectrumEngine e = make(256.0, 45.0, 0.8);
    e.update(sum(wander, signal).data(), SpectrumEngine::Averaging::Medium);
    QVERIFY2(std::abs(e.peakHz() - 10.0) < 0.1, qPrintable(QString::number(e.peakHz())));

    // And the EMG floor of 20 Hz: a strong 8 Hz motion artifact must not win.
    SpectrumEngine m = make(1000.0, 450.0, 20.0);
    m.update(sum(tone(1000.0, 8.0, 3000.0), tone(1000.0, 90.0, 40.0)).data(),
             SpectrumEngine::Averaging::Medium);
    QVERIFY2(std::abs(m.peakHz() - 90.0) < 2.0, qPrintable(QString::number(m.peakHz())));
}

void TstSpectrumEngine::dcOffsetDoesNotLeak()
{
    // 200 mV of electrode offset under a 20 uV signal. Without mean removal the
    // offset owns bin 0 and the window's skirts push it well into the signal.
    SpectrumEngine e = make(256.0, 45.0, 0.8);
    e.update(tone(256.0, 10.0, 20.0, 200000.0).data(), SpectrumEngine::Averaging::Medium);

    QVERIFY(std::abs(e.peakHz() - 10.0) < 0.1);
    // The DC bin and its neighbour sit far below the peak.
    QVERIFY2(e.powerDb()[0] < e.peakDb() - 40.0,
             qPrintable(QStringLiteral("DC bin %1 dB vs peak %2 dB").arg(e.powerDb()[0]).arg(e.peakDb())));
    QVERIFY(e.powerDb()[1] < e.peakDb() - 30.0);
}

void TstSpectrumEngine::firstFrameSeedsTheAverage()
{
    // The very first frame must show the real spectrum, not a ramp up from
    // silence - otherwise every signal change flashes an empty plot.
    SpectrumEngine e = make(256.0, 45.0, 0.8);
    e.update(tone(256.0, 10.0, 100.0).data(), SpectrumEngine::Averaging::High);

    // |X[k]|^2/N for a Hann-windowed tone on a bin: X = A*N/4 (Hann sums to N/2,
    // half of a sine's two-sided amplitude lands on this bin).
    const double expectedPower = std::pow(100.0 * double(kN) / 4.0, 2.0) / double(kN);
    QVERIFY2(std::abs(e.powerDb()[20] - dbOf(expectedPower)) < 0.2,
             qPrintable(QStringLiteral("%1 dB, expected %2 dB").arg(e.powerDb()[20]).arg(dbOf(expectedPower))));
}

void TstSpectrumEngine::exponentialAverageUsesTheSpecWeights_data()
{
    QTest::addColumn<int>("averaging");
    QTest::addColumn<double>("weight");
    QTest::newRow("Low 0.6") << int(SpectrumEngine::Averaging::Low) << 0.6;
    QTest::newRow("Med 0.3") << int(SpectrumEngine::Averaging::Medium) << 0.3;
    QTest::newRow("High 0.12") << int(SpectrumEngine::Averaging::High) << 0.12;
}

void TstSpectrumEngine::exponentialAverageUsesTheSpecWeights()
{
    QFETCH(int, averaging);
    QFETCH(double, weight);
    QCOMPARE(SpectrumEngine::weightFor(static_cast<SpectrumEngine::Averaging>(averaging)), weight);

    // Frame 1: amplitude A. Frame 2: amplitude 2A, i.e. four times the power.
    // New average = (1-w)*P1 + w*4*P1, exactly.
    SpectrumEngine e = make(256.0, 45.0, 0.8);
    const auto avg = static_cast<SpectrumEngine::Averaging>(averaging);
    e.update(tone(256.0, 10.0, 100.0).data(), avg);
    const double p1 = std::pow(10.0, e.powerDb()[20] / 10.0);

    e.update(tone(256.0, 10.0, 200.0).data(), avg);
    const double expected = (1.0 - weight) * p1 + weight * 4.0 * p1;
    QVERIFY2(std::abs(e.powerDb()[20] - dbOf(expected)) < 0.05,
             qPrintable(QStringLiteral("%1 dB, expected %2 dB").arg(e.powerDb()[20]).arg(dbOf(expected))));
}

void TstSpectrumEngine::axisFollowsTheSmoothedMaximum()
{
    SpectrumEngine e = make(256.0, 45.0, 0.8);
    e.update(tone(256.0, 10.0, 100.0).data(), SpectrumEngine::Averaging::Low);
    const double m1 = e.smoothedMaxDb();

    // First frame: the smoothed maximum IS the maximum - no ramp.
    QVERIFY(std::abs(m1 - e.peakDb()) < 0.5);

    // Axis geometry: top = max + 4 dB headroom, bottom = top - 48 dB.
    QVERIFY(std::abs(e.axisTopDb(1.0) - (m1 + 4.0)) < 1e-9);
    QVERIFY(std::abs(e.axisTopDb(1.0) - e.axisBottomDb(1.0) - 48.0) < 1e-9);

    // A frame 20 dB louder (Low averaging replaces 60 % of the average, so the
    // plotted maximum rises by 0.6 * 20 = 12 dB): the axis moves 8 % of the way
    // there, 0.92 old + 0.08 new.
    e.update(tone(256.0, 10.0, 1000.0).data(), SpectrumEngine::Averaging::Low);
    const double newMaxDb = dbOf((1.0 - 0.6) * std::pow(10.0, m1 / 10.0) +
                                 0.6 * 100.0 * std::pow(10.0, m1 / 10.0));
    const double expected = 0.92 * m1 + 0.08 * newMaxDb;
    QVERIFY2(std::abs(e.smoothedMaxDb() - expected) < 0.2,
             qPrintable(QStringLiteral("%1, expected %2").arg(e.smoothedMaxDb()).arg(expected)));
}

void TstSpectrumEngine::gainRaisesTheCurve()
{
    // Gain x2 must make the spectrum LARGER on screen, as it does in the time
    // domain: the axis top comes DOWN by 20*log10(2) = 6.02 dB, so the same
    // curve sits that much higher in the plot. (The spec's literal "+" does the
    // opposite - see SpectrumEngine::axisTopDb and SPEC_COMPLIANCE.md.)
    SpectrumEngine e = make(256.0, 45.0, 0.8);
    e.update(tone(256.0, 10.0, 100.0).data(), SpectrumEngine::Averaging::Medium);

    const double top1 = e.axisTopDb(1.0);
    QVERIFY(std::abs(e.axisTopDb(2.0) - (top1 - 6.0206)) < 1e-3);
    QVERIFY(std::abs(e.axisTopDb(0.5) - (top1 + 6.0206)) < 1e-3);
    QVERIFY(e.axisTopDb(2.0) < top1);
    QVERIFY(std::abs(e.axisTopDb(0.0) - top1) < 1e-9);   // nonsense gain falls back to x1
}

void TstSpectrumEngine::silenceStaysFiniteAndHasNoPeak()
{
    SpectrumEngine e = make(256.0, 45.0, 0.8);
    const std::vector<float> zeros(kN, 0.0f);
    e.update(zeros.data(), SpectrumEngine::Averaging::Medium);

    QVERIFY(e.valid());
    for (float db : e.powerDb()) {
        QVERIFY(std::isfinite(db));
        QVERIFY(db >= -121.0f);
    }
    QVERIFY(std::isfinite(e.axisTopDb(1.0)));
    // No tone, so no peak: a marker drawn on silence would be reporting noise
    // floor tie-breaking, not a frequency.
    QCOMPARE(e.peakHz(), 0.0);
}

void TstSpectrumEngine::medianFindsTheCentreOfPower()
{
    // One tone: the median is the tone, to within a bin or two.
    SpectrumEngine one = make(1000.0, 450.0, 20.0);
    one.update(tone(1000.0, 100.0, 50.0).data(), SpectrumEngine::Averaging::Medium);
    QVERIFY2(std::abs(one.medianHz(20.0, 450.0) - 100.0) < 4.0,
             qPrintable(QString::number(one.medianHz(20.0, 450.0))));

    // Two tones, 4:1 in power (2:1 in amplitude): 80 % of the power is at 60 Hz,
    // so the half-power point is inside that tone, not between them.
    SpectrumEngine two = make(1000.0, 450.0, 20.0);
    two.update(sum(tone(1000.0, 60.0, 100.0), tone(1000.0, 200.0, 50.0)).data(),
               SpectrumEngine::Averaging::Medium);
    QVERIFY2(std::abs(two.medianHz(20.0, 450.0) - 60.0) < 4.0,
             qPrintable(QString::number(two.medianHz(20.0, 450.0))));
}

void TstSpectrumEngine::medianHonoursItsBand()
{
    // The fatigue index is measured from 20 Hz up. Motion artifact below that
    // must not drag it down.
    SpectrumEngine e = make(1000.0, 450.0, 20.0);
    e.update(sum(tone(1000.0, 5.0, 4000.0), tone(1000.0, 120.0, 60.0)).data(),
             SpectrumEngine::Averaging::Medium);

    QVERIFY2(e.medianHz(0.0, 450.0) < 20.0, "full-band median should be dragged to the artifact");
    QVERIFY2(std::abs(e.medianHz(20.0, 450.0) - 120.0) < 4.0,
             qPrintable(QString::number(e.medianHz(20.0, 450.0))));

    SpectrumEngine silent = make(1000.0, 450.0, 20.0);
    QCOMPARE(silent.medianHz(20.0, 450.0), 0.0);   // nothing measured yet
}

void TstSpectrumEngine::bandPowersPartitionTheSpectrum()
{
    // The EEG band edges from the spec: delta 0.5-4, theta 4-8, alpha 8-13,
    // beta 13-30, gamma 30-45.
    SpectrumEngine e = make(256.0, 45.0, 0.8);
    e.update(tone(256.0, 10.0, 100.0).data(), SpectrumEngine::Averaging::Medium);

    const double d = e.bandPower(0.5, 4.0), t = e.bandPower(4.0, 8.0), a = e.bandPower(8.0, 13.0),
                 b = e.bandPower(13.0, 30.0), g = e.bandPower(30.0, 45.0);
    const double total = e.bandPower(0.5, 45.0);

    // Half-open bands tile the range exactly - no bin counted twice or dropped.
    QVERIFY(std::abs((d + t + a + b + g) - total) < total * 1e-9);
    QVERIFY2(a / total > 0.95, qPrintable(QStringLiteral("alpha share %1").arg(a / total)));
    QVERIFY(d / total < 0.01 && g / total < 0.01);
}

void TstSpectrumEngine::decimationFactorsFollowTheDeviceRate()
{
    // The rates this board actually produces against the spec's nominal ones.
    QCOMPARE(SpectrumEngine::decimationFactor(1130.0, 250.0), 5);    // 4.52 rounds to 5
    QCOMPARE(SpectrumEngine::decimationFactor(1000.0, 250.0), 4);
    QCOMPARE(SpectrumEngine::decimationFactor(2000.0, 1000.0), 2);   // EMG
    QCOMPARE(SpectrumEngine::decimationFactor(1000.0, 1000.0), 1);
    QCOMPARE(SpectrumEngine::decimationFactor(200.0, 250.0), 1);     // never below 1
    QCOMPARE(SpectrumEngine::decimationFactor(0.0, 250.0), 1);       // garbage in, safe out
    QCOMPARE(SpectrumEngine::decimationFactor(1000.0, -1.0), 1);
}

void TstSpectrumEngine::decimateMeanAverages()
{
    const float in[] = {1, 2, 3, 4, 5, 6};
    float out[3] = {};
    SpectrumEngine::decimateMean(in, 6, 2, out);
    QCOMPARE(out[0], 1.5f);
    QCOMPARE(out[1], 3.5f);
    QCOMPARE(out[2], 5.5f);

    SpectrumEngine::decimateMean(in, 6, 3, out);
    QCOMPARE(out[0], 2.0f);
    QCOMPARE(out[1], 5.0f);

    // A trailing partial block is dropped, not padded.
    float out2[4] = {-1, -1, -1, -1};
    SpectrumEngine::decimateMean(in, 5, 2, out2);
    QCOMPARE(out2[0], 1.5f);
    QCOMPARE(out2[1], 3.5f);
    QCOMPARE(out2[2], -1.0f);

    // Factor 1 is a copy.
    float out3[6] = {};
    SpectrumEngine::decimateMean(in, 6, 1, out3);
    for (int i = 0; i < 6; ++i) {
        QCOMPARE(out3[i], in[i]);
    }
}

void TstSpectrumEngine::decimationRestoresSpecResolution()
{
    // Why decimate at all: at the raw device rate the resolution is 4-8x coarser
    // than the spec's footer shows. After block-averaging it is back near it.
    const double ecgRaw = 1130.0, ecgTarget = 250.0;
    const int m = SpectrumEngine::decimationFactor(ecgRaw, ecgTarget);
    const double rawDf = ecgRaw / double(kN);
    const double decDf = (ecgRaw / m) / double(kN);
    QVERIFY2(rawDf > 2.0, "undecimated ECG resolution is about 2.2 Hz");
    QVERIFY2(std::abs(decDf - 0.49) < 0.12, qPrintable(QStringLiteral("decimated df %1").arg(decDf)));

    const int mEmg = SpectrumEngine::decimationFactor(2000.0, 1000.0);
    QVERIFY(std::abs((2000.0 / mEmg) / double(kN) - 1.953) < 0.01);
}

void TstSpectrumEngine::blockMeanAttenuatesAboveTheNewNyquist()
{
    // 4-sample means at 1000 SPS, so the new rate is 250 Hz and Nyquist 125 Hz.
    // A 300 Hz tone must not fold back in at full strength: the mean is a
    // crude low-pass (a sinc), and at 300 Hz it is down by
    //   |sin(pi*f*M/fs) / (M*sin(pi*f/fs))| = 0.15  ->  -16 dB.
    constexpr double fs = 1000.0;
    constexpr int m = 4;
    auto level = [&](double f) {
        std::vector<float> in(kN * m), out(kN);
        for (std::size_t n = 0; n < in.size(); ++n) {
            in[n] = static_cast<float>(100.0 * std::sin(2.0 * kPi * f * double(n) / fs));
        }
        SpectrumEngine::decimateMean(in.data(), in.size(), m, out.data());
        double peak = 0.0;
        for (float v : out) {
            peak = std::max(peak, std::abs(double(v)));
        }
        return peak / 100.0;
    };

    const double passband = level(20.0);
    const double stopband = level(300.0);
    QVERIFY2(passband > 0.97, qPrintable(QString::number(passband)));
    QVERIFY2(stopband < 0.2, qPrintable(QString::number(stopband)));
}

void TstSpectrumEngine::displayBinsStopAtTheDisplayRange()
{
    SpectrumEngine e = make(250.0, 40.0, 0.8);
    // 40 Hz / 0.488 Hz per bin = 81.9 -> bins 0..81, so 82 entries.
    QCOMPARE(e.displayBins(), std::size_t(82));
    e.update(tone(250.0, 10.0, 100.0).data(), SpectrumEngine::Averaging::Medium);
    QCOMPARE(e.powerDb().size(), std::size_t(82));

    // A display range beyond Nyquist is clamped, not read past the end.
    SpectrumEngine wide = make(250.0, 5000.0, 0.8);
    QCOMPARE(wide.displayBins(), kN / 2 + 1);
}

QTEST_APPLESS_MAIN(TstSpectrumEngine)
#include "tst_spectrumengine.moc"
