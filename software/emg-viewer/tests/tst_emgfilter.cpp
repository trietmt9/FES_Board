// EMG conditioning chain.
//
// The point of these tests is the claim the chain is *for*: that a realistic
// EMG channel - millivolts of electrode offset, sub-hertz drift, mains hum, and
// only ~100 uV of muscle underneath - comes out with the muscle signal intact
// and everything else gone, and that the envelope responds to a contraction on
// a human timescale rather than averaging it away.

#include <QTest>

#include "core/Biquad.h"
#include "core/EmgFilter.h"

#include <cmath>
#include <vector>

using namespace emg;

namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kRate = 4000.0; // the firmware's configured rate

// Sinusoid amplitude recovered from a settled filter, by peak detection over
// the back half of the run (the front half covers the transient).
double gainAt(ChannelFilter &f, double freqHz, double amplitude, double rate,
              double seconds = 3.0)
{
    f.reset();
    const auto n = static_cast<std::size_t>(seconds * rate);
    double peak = 0.0;

    for (std::size_t i = 0; i < n; ++i) {
        const auto x = static_cast<float>(
            amplitude * std::sin(2.0 * kPi * freqHz * i / rate));
        const float y = f.process(x);
        if (i > n / 2) {
            peak = std::max(peak, std::abs(double(y)));
        }
    }
    return peak / amplitude;
}

double db(double gain) { return 20.0 * std::log10(std::max(gain, 1e-12)); }

// The sEMG band, stated explicitly.
//
// These tests measure the SENIAM 20-450 Hz chain, so they must pin the corners
// rather than inherit FilterConfig's defaults - those defaults are a UI
// preference (they follow StreamController's default SignalMode, currently the
// ECG-monitor band) and moving them must not silently redefine what this file
// is testing. `enabled` is likewise set on purpose: it is a display choice, and
// a response test that leaves it at whatever the default is can end up
// measuring the bypass path and passing for the wrong reason.
FilterConfig semg(bool enabled = true)
{
    FilterConfig cfg;
    cfg.enabled = enabled;
    cfg.highpassHz = 20.0;
    cfg.lowpassHz = 450.0;
    return cfg;
}

FilterConfig filtering() { return semg(true); }

} // namespace

class TstEmgFilter : public QObject {
    Q_OBJECT

private slots:
    void biquadDesignersHitTheirCorners();
    void passbandIsFlat();
    void passbandIsFlat_data();
    void rejectsDriftAndOffset();
    void rejectsMains();
    void notchLeavesNeighboursAlone();
    void envelopeTracksBurstOnHumanTimescale();
    void survivesRealisticCompositeSignal();
    void enabledFlagDoesNotGateTheFilter();
    void envelopeIgnoresDisplayBypass();
    void envelopeWindowSetsResponseTime();
    void primingRemovesTheStartupTransient();
    void primingDoesNotHideAnInputStep();
    void biquadPrimeMatchesALongRun();
    void notchAppliesEvenAboveTheLowpassCorner();
};

void TstEmgFilter::biquadDesignersHitTheirCorners()
{
    // A single biquad is -3 dB at its design corner when Q = 1/sqrt(2).
    Biquad lp = designLowpass(450.0, kRate);
    QVERIFY(std::abs(db(lp.magnitudeAt(450.0, kRate)) + 3.0) < 0.1);
    QVERIFY(std::abs(db(lp.magnitudeAt(1.0, kRate))) < 0.01); // DC passes

    Biquad hp = designHighpass(20.0, kRate);
    QVERIFY(std::abs(db(hp.magnitudeAt(20.0, kRate)) + 3.0) < 0.1);
    QVERIFY(db(hp.magnitudeAt(0.1, kRate)) < -80.0); // DC blocked hard

    Biquad notch = designNotch(50.0, kRate, 30.0);
    QVERIFY(db(notch.magnitudeAt(50.0, kRate)) < -40.0);      // deep at f0
    QVERIFY(std::abs(db(notch.magnitudeAt(150.0, kRate))) < 0.5); // clear by 150
}

void TstEmgFilter::passbandIsFlat_data()
{
    QTest::addColumn<double>("freq");
    QTest::addColumn<double>("toleranceDb");

    // Across the sEMG band, avoiding the notch frequencies. Butterworth is
    // maximally flat but not perfectly flat: approaching the 450 Hz corner the
    // roll-off legitimately begins, so the tolerance widens there rather than
    // pretending the response is ruler-flat up to the edge.
    //   4th-order: |H| = 1/sqrt(1 + (f/fc)^8)  ->  -1.4 dB at 400 Hz.
    QTest::newRow("30 Hz") << 30.0 << 1.5;
    QTest::newRow("80 Hz") << 80.0 << 1.5;
    QTest::newRow("120 Hz") << 120.0 << 1.5;
    QTest::newRow("200 Hz") << 200.0 << 1.5;
    QTest::newRow("300 Hz") << 300.0 << 1.5;
    QTest::newRow("400 Hz (near corner)") << 400.0 << 2.5;
}

void TstEmgFilter::passbandIsFlat()
{
    QFETCH(double, freq);
    QFETCH(double, toleranceDb);

    ChannelFilter f;
    f.configure(filtering(), kRate);

    const double g = db(gainAt(f, freq, 100.0, kRate));
    QVERIFY2(std::abs(g) < toleranceDb,
             qPrintable(QStringLiteral("%1 Hz: %2 dB (tolerance %3)")
                            .arg(freq).arg(g).arg(toleranceDb)));
}

void TstEmgFilter::rejectsDriftAndOffset()
{
    ChannelFilter f;
    f.configure(filtering(), kRate);

    // Baseline drift at 0.5 Hz: the dominant artifact on a DC-coupled front end.
    const double drift = db(gainAt(f, 0.5, 5000.0, kRate, 12.0));
    QVERIFY2(drift < -60.0, qPrintable(QStringLiteral("0.5 Hz: %1 dB").arg(drift)));

    // A pure DC offset must settle to nothing at all.
    f.reset();
    float last = 0.0f;
    for (int i = 0; i < int(20 * kRate); ++i) {
        last = f.process(5000.0f); // 5 mV of electrode half-cell offset
    }
    QVERIFY2(std::abs(double(last)) < 1.0,
             qPrintable(QStringLiteral("DC residue %1 uV").arg(double(last))));
}

void TstEmgFilter::rejectsMains()
{
    FilterConfig cfg = filtering();
    cfg.notchHz = 50.0;
    ChannelFilter f;
    f.configure(cfg, kRate);

    const double f50 = db(gainAt(f, 50.0, 100.0, kRate));
    const double f100 = db(gainAt(f, 100.0, 100.0, kRate));
    QVERIFY2(f50 < -20.0, qPrintable(QStringLiteral("50 Hz: %1 dB").arg(f50)));
    QVERIFY2(f100 < -20.0, qPrintable(QStringLiteral("100 Hz harmonic: %1 dB").arg(f100)));

    // 60 Hz selected instead: 50 Hz must then pass.
    cfg.notchHz = 60.0;
    f.configure(cfg, kRate);
    QVERIFY(db(gainAt(f, 60.0, 100.0, kRate)) < -20.0);
    QVERIFY(db(gainAt(f, 50.0, 100.0, kRate)) > -6.0);
}

void TstEmgFilter::notchLeavesNeighboursAlone()
{
    // The notch must not gut the 50-150 Hz region where most EMG power sits.
    ChannelFilter f;
    f.configure(filtering(), kRate);

    for (double freq : {35.0, 75.0, 130.0, 250.0}) {
        const double g = db(gainAt(f, freq, 100.0, kRate));
        QVERIFY2(g > -3.0, qPrintable(QStringLiteral("%1 Hz: %2 dB").arg(freq).arg(g)));
    }
}

void TstEmgFilter::envelopeTracksBurstOnHumanTimescale()
{
    // The original symptom: "no change in the envelope when I contract".
    // A 1 s burst inside a multi-second record must move the envelope clearly,
    // and must do so within a few hundred milliseconds of onset.
    ChannelFilter f;
    f.configure(filtering(), kRate);

    auto run = [&](double seconds, double burstAmp) {
        const auto n = static_cast<std::size_t>(seconds * kRate);
        unsigned state = 22222u;
        for (std::size_t i = 0; i < n; ++i) {
            state = state * 1103515245u + 12345u;
            const double noise = (double((state >> 16) & 0x7FFF) / 32768.0 - 0.5);
            // 120 Hz carrier, amplitude-modulated, on 5 mV of offset.
            const double emg = burstAmp * noise *
                               std::sin(2.0 * kPi * 120.0 * i / kRate);
            f.process(static_cast<float>(5000.0 + emg));
        }
        return f.envelope();
    };

    const double rest = run(3.0, 0.0);      // 3 s at rest
    const double active = run(1.0, 400.0);  // then 1 s of contraction

    QVERIFY2(active > 8.0 * rest,
             qPrintable(QStringLiteral("rest %1 uV -> active %2 uV")
                            .arg(rest).arg(active)));

    // Responsiveness: from rest, the envelope must be most of the way up within
    // 400 ms of onset. A window-wide RMS would take seconds.
    f.reset();
    run(2.0, 0.0);
    const auto n400 = static_cast<std::size_t>(0.4 * kRate);
    unsigned state = 999u;
    for (std::size_t i = 0; i < n400; ++i) {
        state = state * 1103515245u + 12345u;
        const double noise = (double((state >> 16) & 0x7FFF) / 32768.0 - 0.5);
        f.process(static_cast<float>(
            5000.0 + 400.0 * noise * std::sin(2.0 * kPi * 120.0 * i / kRate)));
    }
    const double after400ms = f.envelope();
    QVERIFY2(after400ms > 0.7 * active,
             qPrintable(QStringLiteral("after 400 ms: %1 uV vs settled %2 uV")
                            .arg(after400ms).arg(active)));
}

void TstEmgFilter::survivesRealisticCompositeSignal()
{
    // Everything at once, at the amplitudes the hardware actually presents:
    //   5 mV DC offset + 200 uV of 0.3 Hz drift + 40 uV mains + 100 uV EMG.
    // The EMG must come out close to its true amplitude.
    ChannelFilter f;
    f.configure(filtering(), kRate);

    const auto n = static_cast<std::size_t>(10.0 * kRate);
    double peak = 0.0;

    for (std::size_t i = 0; i < n; ++i) {
        const double t = double(i) / kRate;
        const double x = 5000.0                                     // offset
                       + 200.0 * std::sin(2.0 * kPi * 0.3 * t)      // drift
                       + 40.0 * std::sin(2.0 * kPi * 50.0 * t)      // mains
                       + 100.0 * std::sin(2.0 * kPi * 120.0 * t);   // "EMG"
        const float y = f.process(static_cast<float>(x));
        if (t > 5.0) {
            peak = std::max(peak, std::abs(double(y)));
        }
    }

    // Within 10% of the 100 uV component, with everything else removed.
    QVERIFY2(std::abs(peak - 100.0) < 10.0,
             qPrintable(QStringLiteral("recovered %1 uV of 100 uV").arg(peak)));
}

void TstEmgFilter::enabledFlagDoesNotGateTheFilter()
{
    // FilterConfig::enabled must NOT gate process(): it is a display choice
    // consumed by StreamController::displayRing(), which selects between a raw
    // and a filtered ring. When process() honoured it, the filtered ring got
    // filled with raw samples and switching to the filtered view showed
    // unfiltered history.
    const FilterConfig off = semg(false);
    const FilterConfig on = semg(true);

    ChannelFilter a;
    ChannelFilter b;
    a.configure(off, kRate);
    b.configure(on, kRate);

    // Identical output and identical envelope regardless of the flag.
    for (int i = 0; i < int(2 * kRate); ++i) {
        const double t = double(i) / kRate;
        const auto x = static_cast<float>(
            5000.0 + 100.0 * std::sin(2.0 * kPi * 120.0 * t));
        QCOMPARE(a.process(x), b.process(x));
    }
    QCOMPARE(a.envelope(), b.envelope());

    // And it really did filter: the 5 mV offset is gone.
    QVERIFY2(a.envelope() > 50.0 && a.envelope() < 90.0,
             qPrintable(QStringLiteral("envelope %1 uV, expected ~70").arg(a.envelope())));
}

void TstEmgFilter::envelopeIgnoresDisplayBypass()
{
    // ...but the envelope is still taken from the filtered chain, so you can
    // look at the raw trace and keep a meaningful activity reading. Without
    // this, turning the filter off makes the envelope measure drift - which is
    // exactly what it used to do.
    auto measure = [](bool displayFiltered) {
        const FilterConfig cfg = semg(displayFiltered);
        ChannelFilter f;
        f.configure(cfg, kRate);

        // 5 mV offset + heavy 0.3 Hz drift + a 120 Hz "muscle" component.
        const auto n = static_cast<std::size_t>(8.0 * kRate);
        for (std::size_t i = 0; i < n; ++i) {
            const double t = double(i) / kRate;
            f.process(static_cast<float>(5000.0
                + 900.0 * std::sin(2.0 * kPi * 0.3 * t)
                + 100.0 * std::sin(2.0 * kPi * 120.0 * t)));
        }
        return f.envelope();
    };

    const double shown = measure(true);
    const double bypassed = measure(false);

    // Identical either way, and reporting the 100 uV component (~70 uV RMS)
    // rather than the 900 uV of drift.
    QVERIFY2(std::abs(shown - bypassed) < 0.01,
             qPrintable(QStringLiteral("filtered %1 vs raw-display %2")
                            .arg(shown).arg(bypassed)));
    QVERIFY2(bypassed > 50.0 && bypassed < 90.0,
             qPrintable(QStringLiteral("envelope %1 uV, expected ~70").arg(bypassed)));
}

void TstEmgFilter::envelopeWindowSetsResponseTime()
{
    // The "quite slow" complaint. Measured as a step response with the window
    // already full - not from startup, because the envelope divides by the
    // number of filled slots and so is valid from the very first sample,
    // meaning the startup ramp is fast regardless of window length.
    auto stepResponseMs = [](double windowMs) {
        FilterConfig cfg = filtering();
        cfg.envelopeMs = windowMs;
        ChannelFilter f;
        f.configure(cfg, kRate);

        auto drive = [&](double amp, std::size_t n) {
            static std::size_t phase = 0;
            for (std::size_t i = 0; i < n; ++i, ++phase) {
                f.process(static_cast<float>(
                    amp * std::sin(2.0 * kPi * 120.0 * phase / kRate)));
            }
        };

        // Settle at a low level so the window is full and stable.
        drive(20.0, static_cast<std::size_t>(2.0 * kRate));
        const double before = f.envelope();

        // Step up, and time how long to cover 63% of the way to the new level.
        const double after = 200.0 / std::sqrt(2.0);   // RMS of the new amplitude
        const double threshold = before + 0.63 * (after - before);

        const auto limit = static_cast<std::size_t>(2.0 * kRate);
        for (std::size_t i = 0; i < limit; ++i) {
            const double t = double(i) / kRate;
            f.process(static_cast<float>(200.0 * std::sin(2.0 * kPi * 120.0 * t)));
            if (f.envelope() >= threshold) {
                return double(i) / kRate * 1000.0;
            }
        }
        return 1e9;
    };

    const double fast = stepResponseMs(50.0);
    const double slow = stepResponseMs(250.0);

    QVERIFY2(fast < slow,
             qPrintable(QStringLiteral("50 ms window: %1 ms; 250 ms window: %2 ms")
                            .arg(fast).arg(slow)));
    // A 50 ms window must feel immediate.
    QVERIFY2(fast < 60.0,
             qPrintable(QStringLiteral("50 ms window responded in %1 ms").arg(fast)));
    // And 250 ms must be visibly slower - the thing that prompted the change.
    QVERIFY2(slow > 100.0,
             qPrintable(QStringLiteral("250 ms window responded in %1 ms").arg(slow)));
}



// A DC-coupled front end hands the filter hundreds of millivolts of electrode
// offset on its very first sample. Started from zero state, a 0.5 Hz high-pass
// reads that as a step and rings for seconds - with the trace pinned off a
// +/-1.6 mV plot. Priming on the first sample is what stops that.
void TstEmgFilter::primingRemovesTheStartupTransient()
{
    FilterConfig cfg;          // ECG monitor band, 0.5-40 Hz
    cfg.notchEnabled = false;
    ChannelFilter f;
    f.configure(cfg, kRate);

    // 200 mV of offset, a 20 uV 10 Hz signal riding on it.
    constexpr double kOffsetUv = 200000.0;
    constexpr double kSigUv = 20.0;
    double worst = 0.0;
    for (std::size_t i = 0; i < static_cast<std::size_t>(2.0 * kRate); ++i) {
        const double x = kOffsetUv + kSigUv * std::sin(2.0 * kPi * 10.0 * i / kRate);
        const double y = f.process(static_cast<float>(x));
        worst = std::max(worst, std::abs(y));
    }

    // Never leaves the signal's own amplitude (plus float rounding on a 200 mV
    // input: float has ~7 digits, so 200000 uV carries ~0.02 uV of noise).
    QVERIFY2(worst < 2.0 * kSigUv,
             qPrintable(QStringLiteral("worst excursion %1 uV on a %2 uV signal - "
                                       "the offset is ringing through")
                            .arg(worst).arg(kSigUv)));
}

// Priming must not become a way to hide a real step: an offset that appears
// AFTER the first sample is a genuine event and has to come through.
void TstEmgFilter::primingDoesNotHideAnInputStep()
{
    FilterConfig cfg;
    cfg.notchEnabled = false;
    ChannelFilter f;
    f.configure(cfg, kRate);

    for (int i = 0; i < 4000; ++i) {
        f.process(0.0f);                       // settled at zero
    }

    // A 5 mV step, as an electrode being touched would produce.
    double first = 0.0;
    for (int i = 0; i < 40; ++i) {
        first = f.process(5000.0f);
    }
    QVERIFY2(std::abs(first) > 1000.0,
             qPrintable(QStringLiteral("a real 5 mV step was swallowed (%1 uV)").arg(first)));
}

// The closed-form steady state must equal what a long run converges to.
void TstEmgFilter::biquadPrimeMatchesALongRun()
{
    for (double x : {1.0, -37.5, 200000.0}) {
        Biquad run = designHighpass(0.5, kRate, kButter4Q1);
        for (int i = 0; i < 400000; ++i) {
            run.process(static_cast<float>(x));          // settle by brute force
        }

        Biquad primed = designHighpass(0.5, kRate, kButter4Q1);
        primed.prime(x);

        // Both should now emit ~0 for the same constant input.
        const double a = run.process(static_cast<float>(x));
        const double b = primed.process(static_cast<float>(x));
        const double tol = 1e-3 * std::max(1.0, std::abs(x)) * 1e-3;
        QVERIFY2(std::abs(b) <= std::max(tol, 1e-3),
                 qPrintable(QStringLiteral("primed output %1 for DC %2").arg(b).arg(x)));
        QVERIFY2(std::abs(a - b) < std::max(0.05, std::abs(x) * 2e-6),
                 qPrintable(QStringLiteral("long run %1 vs primed %2 for DC %3")
                                .arg(a).arg(b).arg(x)));
    }
}

// The ECG band is 0.5-40 Hz, so 50 Hz mains sits ABOVE the low-pass corner. The
// notch must still be there: the roll-off alone leaves it only ~8 dB down.
void TstEmgFilter::notchAppliesEvenAboveTheLowpassCorner()
{
    constexpr double kFs = 1130.0;   // what this board actually runs at

    FilterConfig with;               // 0.5-40 Hz
    with.notchEnabled = true;
    with.notchHz = 50.0;
    with.notchHarmonics = 1;

    FilterConfig without = with;
    without.notchEnabled = false;

    ChannelFilter fWith, fWithout;
    fWith.configure(with, kFs);
    fWithout.configure(without, kFs);

    const double gWith = 20.0 * std::log10(gainAt(fWith, 50.0, 100.0, kFs));
    const double gWithout = 20.0 * std::log10(gainAt(fWithout, 50.0, 100.0, kFs));

    // Roll-off alone: 1/sqrt(1 + (50/40)^8) = -8.4 dB.
    QVERIFY2(gWithout > -10.0 && gWithout < -7.0,
             qPrintable(QStringLiteral("roll-off alone: %1 dB").arg(gWithout)));
    // With the notch it is far deeper. Fails (-8 dB) if the notch is skipped.
    QVERIFY2(gWith < gWithout - 15.0,
             qPrintable(QStringLiteral("with notch %1 dB vs without %2 dB").arg(gWith).arg(gWithout)));

    // The harmonic IS still skipped above the corner: 100 Hz is already far
    // down, and a second notch costs phase for nothing.
    FilterConfig harm = with;
    harm.notchHarmonics = 2;
    ChannelFilter fHarm;
    fHarm.configure(harm, kFs);
    const double g100with = 20.0 * std::log10(gainAt(fHarm, 100.0, 100.0, kFs));
    const double g100without = 20.0 * std::log10(gainAt(fWithout, 100.0, 100.0, kFs));
    QVERIFY2(std::abs(g100with - g100without) < 1.0,
             qPrintable(QStringLiteral("100 Hz harmonic notch was applied: %1 vs %2 dB")
                            .arg(g100with).arg(g100without)));
}

QTEST_APPLESS_MAIN(TstEmgFilter)
#include "tst_emgfilter.moc"
