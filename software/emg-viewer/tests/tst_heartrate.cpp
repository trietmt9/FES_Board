// QRS detection and heart rate.
//
// The whole point of an ECG simulator is that the right answer is known in
// advance, so these tests check the detector against synthetic ECG at set rates
// and demand it reads them back. Anything that survives a 1 mV signal buried
// under 5 mV of offset, drift and mains will survive a simulator.

#include <QTest>

#include "core/HeartRate.h"

#include <cmath>
#include <vector>

using namespace emg;

namespace {

constexpr double kPi = 3.14159265358979323846;

// Same P-Q-R-S-T Gaussian model as tools/emg_gen.c, kept in step by hand.
double ecgWave(double phaseS, double tScale = 1.0)
{
    static const double off[5] = {-0.160, -0.020, 0.0, 0.020, 0.300};
    static const double amp[5] = {0.15, -0.10, 1.0, -0.25, 0.30};
    static const double wid[5] = {0.025, 0.008, 0.010, 0.010, 0.050};

    double v = 0.0;
    for (int i = 0; i < 5; ++i) {
        // Index 4 is the T wave. Scaling it is the whole point of the
        // bradycardia test: a tall T is what a naive detector counts twice.
        const double a = (i == 4) ? amp[i] * tScale : amp[i];
        const double d = (phaseS - off[i]) / wid[i];
        v += a * std::exp(-0.5 * d * d);
    }
    return v;
}

struct EcgOptions {
    double bpm = 60.0;
    double rMv = 1.0;
    double offsetUv = 0.0;
    double driftUv = 0.0;
    double humUv = 0.0;
    double noiseUv = 0.0;
    double tScale = 1.0;   ///< T-wave height relative to the default 0.30 R
    double spikeUv = 0.0;  ///< amplitude of impulsive artifacts
    double spikeHz = 0.0;  ///< how often they occur
};

/// Feed @p seconds of a signal-free channel: electrode offset plus a little
/// noise, no ECG at all. Models the board running before the source is on.
void feedQuiet(HeartRateDetector &det, double rate, double seconds,
               double offsetUv, double noiseUv)
{
    unsigned state = 24680u;
    const auto n = static_cast<std::size_t>(seconds * rate);
    for (std::size_t i = 0; i < n; ++i) {
        state = state * 1103515245u + 12345u;
        const double rnd = double((state >> 16) & 0x7FFF) / 32768.0 - 0.5;
        det.process(static_cast<float>(offsetUv + noiseUv * rnd));
    }
}

/// Feed @p seconds of synthetic ECG through @p det and return the BPM it settles on.
double measure(HeartRateDetector &det, double rate, double seconds, const EcgOptions &o)
{
    const double rr = 60.0 / o.bpm;
    unsigned state = 13579u;

    const auto n = static_cast<std::size_t>(seconds * rate);
    for (std::size_t i = 0; i < n; ++i) {
        const double t = double(i) / rate;

        double phase = std::fmod(t, rr);
        if (phase > rr / 2.0) {
            phase -= rr;
        }

        state = state * 1103515245u + 12345u;
        const double rnd = double((state >> 16) & 0x7FFF) / 32768.0 - 0.5;

        const double uv = o.offsetUv
                        + o.rMv * 1000.0 * ecgWave(phase, o.tScale)
                        + o.driftUv * std::sin(2.0 * kPi * 0.25 * t)
                        + o.humUv * std::sin(2.0 * kPi * 50.0 * t)
                        + o.noiseUv * rnd;

        // Impulsive artifact: one sample wide, which is what an electrode
        // movement or a switching transient looks like after the AFE. Broadband
        // by construction, so a gentle low-pass leaves plenty of it behind - and
        // the derivative stage in the detector then amplifies what survives.
        double spike = 0.0;
        if (o.spikeUv > 0.0 && o.spikeHz > 0.0) {
            const auto period = static_cast<std::size_t>(rate / o.spikeHz);
            if (period > 0 && (i % period) == 0) {
                spike = o.spikeUv * ((i / period) % 2 ? 1.0 : -1.0);
            }
        }

        det.process(static_cast<float>(uv + spike));
    }
    return det.bpm();
}

} // namespace

class TstHeartRate : public QObject {
    Q_OBJECT

private slots:
    void readsBackKnownRate();
    void readsBackKnownRate_data();
    void survivesRealisticContamination();
    void countsTheRightNumberOfBeats();
    void rejectsSubRefractoryRetriggers();
    void followsARateChange();
    void reportsNothingWithoutSignal();
    void worksAtBothSampleRates();
    void worksAtBothSampleRates_data();
    void doesNotMultiplyCountAtBradycardia();
    void doesNotMultiplyCountAtBradycardia_data();
    void staysLockedThroughSlowBeats();
    void locksOnWhenSignalStartsLate();
    void inventsNoRateFromMainsPickup();
    void inventsNoRateFromMainsPickup_data();
    void survivesImpulsiveArtifacts();
    void survivesImpulsiveArtifacts_data();
};

void TstHeartRate::readsBackKnownRate_data()
{
    QTest::addColumn<double>("bpm");

    // The span an ECG simulator offers: bradycardia through tachycardia.
    for (double b : {30.0, 45.0, 60.0, 75.0, 100.0, 120.0, 180.0}) {
        QTest::newRow(qPrintable(QStringLiteral("%1 BPM").arg(b))) << b;
    }
}

void TstHeartRate::readsBackKnownRate()
{
    QFETCH(double, bpm);

    constexpr double rate = 4000.0;
    HeartRateDetector det;
    det.configure(rate);

    EcgOptions o;
    o.bpm = bpm;

    const double got = measure(det, rate, 20.0, o);

    // Within 1 BPM: the detector's timing resolution is one sample, and the
    // median over 8 intervals removes jitter, so this should be near exact.
    QVERIFY2(std::abs(got - bpm) < 1.0,
             qPrintable(QStringLiteral("set %1 BPM, read %2").arg(bpm).arg(got)));
}

void TstHeartRate::survivesRealisticContamination()
{
    // A 1 mV ECG on the contamination the DC-coupled front end actually
    // delivers: 5 mV electrode offset, 600 uV of drift, 40 uV of mains, plus
    // broadband noise.
    constexpr double rate = 4000.0;
    HeartRateDetector det;
    det.configure(rate);

    EcgOptions o;
    o.bpm = 75.0;
    o.offsetUv = 5000.0;
    o.driftUv = 600.0;
    o.humUv = 40.0;
    o.noiseUv = 60.0;

    const double got = measure(det, rate, 20.0, o);
    QVERIFY2(std::abs(got - 75.0) < 2.0,
             qPrintable(QStringLiteral("read %1 BPM under contamination").arg(got)));
}

void TstHeartRate::countsTheRightNumberOfBeats()
{
    constexpr double rate = 4000.0;
    HeartRateDetector det;
    det.configure(rate);

    EcgOptions o;
    o.bpm = 60.0;
    measure(det, rate, 20.0, o);

    // 20 s at 60 BPM, less 1 s settling and 2 s learning during which nothing
    // is reported, leaves about 17 beats. Allow a couple either side for where
    // the boundaries land relative to the complexes.
    QVERIFY2(det.beats() >= 15 && det.beats() <= 19,
             qPrintable(QStringLiteral("detected %1 beats in 20 s at 60 BPM")
                            .arg(det.beats())));
}

void TstHeartRate::rejectsSubRefractoryRetriggers()
{
    // The QRS is a multi-lobed complex; without a refractory period the
    // detector fires on the R upstroke and again on the S downstroke, and
    // reports double the true rate.
    constexpr double rate = 4000.0;
    HeartRateDetector det;
    det.configure(rate);

    EcgOptions o;
    o.bpm = 60.0;
    const double got = measure(det, rate, 20.0, o);

    QVERIFY2(got < 90.0,
             qPrintable(QStringLiteral("read %1 BPM - looks like double-triggering").arg(got)));
    QVERIFY2(det.lastRrMs() > kRefractoryMs,
             qPrintable(QStringLiteral("RR %1 ms is inside the refractory period")
                            .arg(det.lastRrMs())));
}

void TstHeartRate::followsARateChange()
{
    constexpr double rate = 4000.0;
    HeartRateDetector det;
    det.configure(rate);

    EcgOptions slow;
    slow.bpm = 50.0;
    const double first = measure(det, rate, 20.0, slow);
    QVERIFY(std::abs(first - 50.0) < 2.0);

    // Without resetting: the median window must migrate to the new rate.
    EcgOptions fast;
    fast.bpm = 120.0;
    const double second = measure(det, rate, 20.0, fast);
    QVERIFY2(std::abs(second - 120.0) < 2.0,
             qPrintable(QStringLiteral("after change read %1 BPM").arg(second)));
}

void TstHeartRate::reportsNothingWithoutSignal()
{
    constexpr double rate = 4000.0;
    HeartRateDetector det;
    det.configure(rate);

    // Flat line plus a little noise: no beats, and no invented rate.
    unsigned state = 999u;
    for (int i = 0; i < int(10 * rate); ++i) {
        state = state * 1103515245u + 12345u;
        const double rnd = double((state >> 16) & 0x7FFF) / 32768.0 - 0.5;
        det.process(static_cast<float>(4000.0 + 5.0 * rnd));
    }

    QCOMPARE(det.bpm(), 0.0);
}

void TstHeartRate::worksAtBothSampleRates_data()
{
    QTest::addColumn<double>("rate");
    QTest::newRow("1000 SPS") << 1000.0;
    QTest::newRow("4000 SPS") << 4000.0; // the firmware's configured rate
}

void TstHeartRate::worksAtBothSampleRates()
{
    QFETCH(double, rate);

    HeartRateDetector det;
    det.configure(rate);

    EcgOptions o;
    o.bpm = 72.0;
    o.offsetUv = 5000.0;

    const double got = measure(det, rate, 20.0, o);
    QVERIFY2(std::abs(got - 72.0) < 2.0,
             qPrintable(QStringLiteral("at %1 SPS read %2 BPM").arg(rate).arg(got)));
}

void TstHeartRate::doesNotMultiplyCountAtBradycardia_data()
{
    QTest::addColumn<double>("bpm");
    QTest::addColumn<double>("tScale");

    // A tall T wave is the classic double-count trap, and it only bites at slow
    // rates: the T lands 300-400 ms after the QRS, which is outside a fixed
    // 200 ms refractory. At 30 BPM a detector without T-wave rejection reads
    // 60 or 90 - a real simulator set to 30 was read back as exactly 90.
    QTest::newRow("30 bpm, normal T") << 30.0 << 1.0;
    QTest::newRow("30 bpm, tall T") << 30.0 << 2.0;
    QTest::newRow("30 bpm, very tall T") << 30.0 << 2.5;
    QTest::newRow("40 bpm, tall T") << 40.0 << 2.0;
    QTest::newRow("50 bpm, tall T") << 50.0 << 2.0;
}

void TstHeartRate::doesNotMultiplyCountAtBradycardia()
{
    QFETCH(double, bpm);
    QFETCH(double, tScale);

    constexpr double rate = 1000.0;
    HeartRateDetector det;
    det.configure(rate);

    EcgOptions o;
    o.bpm = bpm;
    o.tScale = tScale;
    o.rMv = 1.0;

    const double got = measure(det, rate, 60.0, o);

    // The failure this guards against is not a small error - it is an integer
    // multiple. Assert tightly enough that 2x or 3x cannot slip through.
    QVERIFY2(got > bpm * 0.85 && got < bpm * 1.15,
             qPrintable(QStringLiteral("expected ~%1 bpm, got %2 (ratio %3)")
                            .arg(bpm).arg(got).arg(got / bpm)));
}

void TstHeartRate::staysLockedThroughSlowBeats()
{
    // The second half of the same bug: after multiply-counting, the level
    // estimates thrash and the rate drops out entirely - the reading appears,
    // reads high, then disappears. A detector that is genuinely locked reports
    // the same rate late in a run as it does early.
    constexpr double rate = 1000.0;
    HeartRateDetector det;
    det.configure(rate);

    EcgOptions o;
    o.bpm = 30.0;
    o.tScale = 2.0;
    o.rMv = 1.0;

    const double early = measure(det, rate, 30.0, o);
    QVERIFY2(early > 25.0 && early < 35.0,
             qPrintable(QStringLiteral("early reading %1").arg(early)));

    // Keep going on the same detector: the rate must not decay away.
    const double late = measure(det, rate, 60.0, o);
    QVERIFY2(late > 25.0 && late < 35.0,
             qPrintable(QStringLiteral("late reading %1 (early was %2)")
                            .arg(late).arg(early)));
}

void TstHeartRate::locksOnWhenSignalStartsLate()
{
    // The actual bench sequence: flash the board, THEN switch on the simulator.
    //
    // That order matters. The detector's settling and learning phases run while
    // the channel is still signal-free, so the levels are learned from noise -
    // and then the signal arrives as a step into a DC-coupled input, which is
    // the "burst" seen on screen. Nothing re-learns afterwards.
    constexpr double rate = 1000.0;
    HeartRateDetector det;
    det.configure(rate);

    // 5 s with the simulator off: a little electrode offset, a little noise.
    feedQuiet(det, rate, 5.0, 2000.0, 20.0);

    // Simulator switched on: a 50 mV DC step, with 30 BPM ECG on top of it.
    EcgOptions o;
    o.bpm = 30.0;
    o.rMv = 1.0;
    o.offsetUv = 50000.0;

    const double got = measure(det, rate, 40.0, o);

    QVERIFY2(got > 25.0 && got < 35.0,
             qPrintable(QStringLiteral("late-starting signal read back as %1 bpm "
                                       "(expected ~30)").arg(got)));
}

void TstHeartRate::inventsNoRateFromMainsPickup_data()
{
    QTest::addColumn<double>("humHz");
    QTest::addColumn<double>("humUv");
    QTest::addColumn<double>("driftUv");

    // What a channel actually looks like with the source switched off but the
    // electrodes still attached: mains pickup, baseline wander, and nothing
    // else. The existing flat-line test uses white noise only, which is the
    // easy case - mains is PERIODIC, and periodic is what a beat detector is
    // looking for.
    QTest::newRow("50 Hz, small") << 50.0 << 50.0 << 0.0;
    QTest::newRow("50 Hz, large") << 50.0 << 500.0 << 0.0;
    QTest::newRow("60 Hz, large") << 60.0 << 500.0 << 0.0;
    QTest::newRow("50 Hz + drift") << 50.0 << 300.0 << 800.0;
    QTest::newRow("60 Hz + drift") << 60.0 << 300.0 << 800.0;
    QTest::newRow("mains only, huge") << 50.0 << 2000.0 << 0.0;
}

void TstHeartRate::inventsNoRateFromMainsPickup()
{
    QFETCH(double, humHz);
    QFETCH(double, humUv);
    QFETCH(double, driftUv);

    constexpr double rate = 1000.0;
    HeartRateDetector det;
    det.configure(rate);

    unsigned state = 8675309u;
    const auto n = static_cast<std::size_t>(40.0 * rate);
    for (std::size_t i = 0; i < n; ++i) {
        const double t = double(i) / rate;
        state = state * 1103515245u + 12345u;
        const double rnd = double((state >> 16) & 0x7FFF) / 32768.0 - 0.5;

        const double uv = 4000.0                                        // offset
                        + humUv * std::sin(2.0 * kPi * humHz * t)       // mains
                        + driftUv * std::sin(2.0 * kPi * 0.25 * t)      // wander
                        + 8.0 * rnd;                                    // noise
        det.process(static_cast<float>(uv));
    }

    // There is no heart here. Reporting a number would be worse than reporting
    // nothing: on a medical readout an invented rate is indistinguishable from
    // a measured one.
    QVERIFY2(det.bpm() == 0.0,
             qPrintable(QStringLiteral("invented %1 bpm from %2 Hz mains at %3 uV")
                            .arg(det.bpm()).arg(humHz).arg(humUv)));
}

void TstHeartRate::survivesImpulsiveArtifacts_data()
{
    QTest::addColumn<double>("spikeUv");
    QTest::addColumn<double>("spikeHz");

    // The reported field failure: a 30 BPM source read back as 115 BPM because
    // of "random pulses". A spike is broadband, so the detector's band-pass
    // passes some of it, and the derivative stage that follows amplifies
    // whatever gets through.
    QTest::newRow("no spikes") << 0.0 << 0.0;
    QTest::newRow("2 mV @ 1 Hz") << 2000.0 << 1.0;
    QTest::newRow("5 mV @ 1 Hz") << 5000.0 << 1.0;
    QTest::newRow("5 mV @ 2 Hz") << 5000.0 << 2.0;
    QTest::newRow("10 mV @ 2 Hz") << 10000.0 << 2.0;
}

void TstHeartRate::survivesImpulsiveArtifacts()
{
    QFETCH(double, spikeUv);
    QFETCH(double, spikeHz);

    constexpr double rate = 1000.0;
    HeartRateDetector det;
    det.configure(rate);

    EcgOptions o;
    o.bpm = 30.0;
    o.rMv = 1.0;
    o.spikeUv = spikeUv;
    o.spikeHz = spikeHz;

    const double got = measure(det, rate, 60.0, o);

    QVERIFY2(got > 25.0 && got < 35.0,
             qPrintable(QStringLiteral("read %1 bpm from a 30 bpm source with "
                                       "%2 uV spikes at %3 Hz")
                            .arg(got).arg(spikeUv).arg(spikeHz)));
}

QTEST_APPLESS_MAIN(TstHeartRate)
#include "tst_heartrate.moc"
