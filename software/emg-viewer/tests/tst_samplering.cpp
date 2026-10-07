// SampleRing::readEndingAt - the read the trace renderer depends on.
//
// The renderer captures the head index once and places every sample from it. If
// the data it reads came from a LATER head, the whole trace would be drawn
// shifted by however far the producer advanced in between: a few milliseconds of
// time error that no screenshot would reveal and no test of "does it draw" would
// catch. So the property under test is exact: the window ends where it was asked
// to, regardless of what the producer has done since.

#include <QTest>

#include "core/SampleRing.h"

#include <vector>

using emg::SampleRing;

namespace {
// Sample n holds the value n, so a window's position can be read straight off it.
void fill(SampleRing &r, int from, int count, int nCh = 1)
{
    std::vector<float> v(std::size_t(count) * nCh);
    for (int i = 0; i < count; ++i) {
        for (int c = 0; c < nCh; ++c) {
            v[std::size_t(i) * nCh + c] = float(from + i) + 1000.0f * c;
        }
    }
    r.write(v.data(), nCh, count);
}
} // namespace

class TstSampleRing : public QObject {
    Q_OBJECT

private slots:
    void windowEndsWhereAskedEvenAfterTheProducerAdvances();
    void matchesReadLatestWhenNothingHasMoved();
    void clampsAnEndBeyondTheWriteCounter();
    void shortensRatherThanReadLappedSlots();
    void emptyAndInvalidRequestsReturnNothing();
    void channelsStayIndependent();
};

void TstSampleRing::windowEndsWhereAskedEvenAfterTheProducerAdvances()
{
    SampleRing r;
    r.configure(1, 1000);
    fill(r, 0, 500);

    const std::uint64_t head = r.written();          // the renderer captures this
    fill(r, 500, 37);                                // ...then the producer moves on

    std::vector<float> w(100);
    QCOMPARE(r.readEndingAt(0, head, w.data(), w.size()), std::size_t(100));
    QCOMPARE(w.front(), 400.0f);                     // samples 400..499, NOT 437..536
    QCOMPARE(w.back(), 499.0f);

    // readLatest, by contrast, follows the producer - which is exactly why it is
    // the wrong call for something that has already committed to a head.
    QCOMPARE(r.readLatest(0, w.data(), w.size()), std::size_t(100));
    QCOMPARE(w.back(), 536.0f);
}

void TstSampleRing::matchesReadLatestWhenNothingHasMoved()
{
    SampleRing r;
    r.configure(1, 256);
    fill(r, 0, 700);                                 // wraps the ring twice

    std::vector<float> a(200), b(200);
    r.readLatest(0, a.data(), a.size());
    QCOMPARE(r.readEndingAt(0, r.written(), b.data(), b.size()), std::size_t(200));
    QCOMPARE(a, b);
}

void TstSampleRing::clampsAnEndBeyondTheWriteCounter()
{
    SampleRing r;
    r.configure(1, 1000);
    fill(r, 0, 300);

    std::vector<float> w(10);
    QCOMPARE(r.readEndingAt(0, 99999, w.data(), w.size()), std::size_t(10));
    QCOMPARE(w.back(), 299.0f);                      // clamped to what exists
}

void TstSampleRing::shortensRatherThanReadLappedSlots()
{
    SampleRing r;
    r.configure(1, 100);
    fill(r, 0, 250);                                 // oldest intact sample is 150

    std::vector<float> w(100);
    // Ask for a window ending at 180: only 150..179 survive. Reading further back
    // would return the NEWER samples that overwrote those slots - plausible-looking
    // data from the wrong time.
    QCOMPARE(r.readEndingAt(0, 180, w.data(), 100), std::size_t(30));
    QCOMPARE(w.front(), 150.0f);
    QCOMPARE(w[29], 179.0f);

    // A window that ended entirely before the oldest intact sample is gone.
    QCOMPARE(r.readEndingAt(0, 120, w.data(), 10), std::size_t(0));
}

void TstSampleRing::emptyAndInvalidRequestsReturnNothing()
{
    SampleRing r;
    r.configure(2, 50);
    std::vector<float> w(10);
    QCOMPARE(r.readEndingAt(0, 0, w.data(), 10), std::size_t(0));      // nothing written
    fill(r, 0, 20, 2);
    QCOMPARE(r.readEndingAt(-1, 20, w.data(), 10), std::size_t(0));    // bad channel
    QCOMPARE(r.readEndingAt(2, 20, w.data(), 10), std::size_t(0));
    QCOMPARE(r.readEndingAt(0, 20, nullptr, 10), std::size_t(0));      // no buffer
    QCOMPARE(r.readEndingAt(0, 20, w.data(), 0), std::size_t(0));      // zero count
}

void TstSampleRing::channelsStayIndependent()
{
    SampleRing r;
    r.configure(3, 64);
    fill(r, 0, 40, 3);

    std::vector<float> w(5);
    for (int c = 0; c < 3; ++c) {
        QCOMPARE(r.readEndingAt(c, 30, w.data(), 5), std::size_t(5));
        QCOMPARE(w.front(), 25.0f + 1000.0f * c);
        QCOMPARE(w.back(), 29.0f + 1000.0f * c);
    }
}

QTEST_APPLESS_MAIN(TstSampleRing)
#include "tst_samplering.moc"
