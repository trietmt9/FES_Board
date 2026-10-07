// Device-side conversion loss, inferred from the stream's own t_ms.
//
// This is the number that tells you whether a "noisy" trace is actually a
// short one - lost conversions do not leave a gap on screen, because the host
// lays samples down at exactly 1/rate. They compress time instead: the QRS
// comes out narrower and taller and the baseline reads as jitter. So the
// measurement has to be trustworthy in both directions - it must not invent
// loss on a clean stream, and it must not miss real loss.

#include <QTest>

#include "core/SampleLoss.h"

using namespace emg;

namespace {
constexpr double kRate = 1000.0;
constexpr int kNSamp = 32;
} // namespace

class TstSampleLoss : public QObject {
    Q_OBJECT

private slots:
    void reportsNothingOnAPerfectStream();
    void countsMissingConversions();
    void ignoresTheFirstFrame();
    void rebasesOnDeviceRestart();
    void quantisationJitterDoesNotAccumulate();
};

void TstSampleLoss::reportsNothingOnAPerfectStream()
{
    SampleLossTracker t;
    for (int b = 0; b < 100; ++b) {
        t.note(static_cast<std::uint32_t>(b * kNSamp), kNSamp, kRate);
    }
    QCOMPARE(t.lost(), std::uint64_t(0));
}

void TstSampleLoss::countsMissingConversions()
{
    // 100 clean blocks, then one that took 40 ms of device time to gather its
    // 32 samples: 8 conversions never arrived.
    SampleLossTracker t;
    std::uint32_t ms = 0;
    t.note(ms, kNSamp, kRate);          // origin; carries no samples
    for (int b = 0; b < 100; ++b) {
        ms += kNSamp;
        t.note(ms, kNSamp, kRate);
    }
    QCOMPARE(t.lost(), std::uint64_t(0));

    ms += kNSamp + 8;                   // this block took 8 ms too long
    t.note(ms, kNSamp, kRate);
    QCOMPARE(t.lost(), std::uint64_t(8));

    // Loss is cumulative, not per-frame: further clean blocks must not erase it.
    for (int b = 0; b < 10; ++b) {
        ms += kNSamp;
        t.note(ms, kNSamp, kRate);
    }
    QCOMPARE(t.lost(), std::uint64_t(8));
}

void TstSampleLoss::ignoresTheFirstFrame()
{
    // The first frame only establishes the origin. A device that has been up
    // for an hour before the host attaches must not be reported as having lost
    // an hour of samples.
    SampleLossTracker t;
    t.note(3600u * 1000u, kNSamp, kRate);
    QCOMPARE(t.lost(), std::uint64_t(0));

    t.note(3600u * 1000u + kNSamp, kNSamp, kRate);
    QCOMPARE(t.lost(), std::uint64_t(0));
}

void TstSampleLoss::rebasesOnDeviceRestart()
{
    // t_ms is a uint32 of milliseconds and the board can be reset under the
    // host. A backwards step must rebase, not report a four-billion-sample loss.
    SampleLossTracker t;
    for (int b = 0; b < 50; ++b) {
        t.note(static_cast<std::uint32_t>(100000 + b * kNSamp), kNSamp, kRate);
    }
    QCOMPARE(t.lost(), std::uint64_t(0));

    t.note(0, kNSamp, kRate);            // board rebooted
    QCOMPARE(t.lost(), std::uint64_t(0));

    for (int b = 1; b < 50; ++b) {
        t.note(static_cast<std::uint32_t>(b * kNSamp), kNSamp, kRate);
    }
    QCOMPARE(t.lost(), std::uint64_t(0));
}

void TstSampleLoss::quantisationJitterDoesNotAccumulate()
{
    // t_ms is a whole number of milliseconds, so a block close is reported a
    // millisecond early or late even when nothing was lost. The property that
    // makes this measurement usable is that such jitter does NOT accumulate:
    // lost() is computed from the two endpoint timestamps, so bounded jitter on
    // each report stays bounded no matter how long the run - it does not grow
    // into a false loss figure over a session.
    SampleLossTracker t;

    for (int b = 0; b <= 2000; ++b) {
        // Nominal close every 32 ms, reported with up to 1 ms of jitter that
        // averages out rather than drifting.
        const auto ms = static_cast<std::uint32_t>(b * kNSamp + (b % 3 == 0 ? 1 : 0));
        t.note(ms, kNSamp, kRate);
    }

    // 2000 blocks is 64000 conversions; anything that accumulated per-block
    // would be in the thousands here.
    QVERIFY2(t.lost() <= 2, qPrintable(QString::number(t.lost())));
}

QTEST_APPLESS_MAIN(TstSampleLoss)
#include "tst_sampleloss.moc"
