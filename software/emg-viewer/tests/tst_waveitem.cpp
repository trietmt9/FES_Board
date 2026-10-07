// WaveItem and WaveTrace, end to end.
//
// What is checked where:
//
//   tst_tracegeometry   the SCALE - which pixel a microvolt and a sample land on - as
//                       numbers, to a hundredth of a pixel. That is where the
//                       project's two earlier scale bugs (1.8x amplitude, stretched
//                       time axis) would be caught.
//   this file           that the live model actually REACHES that geometry: a sine goes
//                       through the real replay, filter and ring, and what WaveTrace
//                       builds from the model's gain, window and head is checked; and
//                       the static layer WaveItem paints (markers, offline) is measured
//                       as pixels.
//
// Nothing is mocked.

#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>

#include "model/Acquisition.h"
#include "model/SignalModel.h"
#include "ui/WaveItem.h"
#include "ui/WaveTrace.h"

extern "C" {
#include "emg_frame.h"
}

#include <algorithm>
#include <cmath>
#include <vector>

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr int kNSamp = 32;
constexpr int kRate = 1000;
constexpr double kUvPerCode = 2400000.0 / 6.0 / 8388608.0;

constexpr int kW = 800;
constexpr int kH = 200;

// Token colours, spelled out so the test does not depend on Theme.qml.
const QColor kBg(0x23, 0x25, 0x32);
const QColor kMarker(0xD2, 0xCE, 0xFD);         // accent-300

QByteArray sineCapture(int seconds, double freqHz, double ampUv, int nCh = 1)
{
    QByteArray out;
    std::vector<std::uint8_t> frame(EMG_FRAME_MAX_SIZE);
    std::vector<std::int32_t> block(std::size_t(nCh) * kNSamp);

    emg_info info{};
    info.sample_rate_hz = kRate;
    info.vref_uv = 2400000;
    info.gain = 6;
    info.chip_id = 0x92;
    info.n_ch_active = std::uint8_t(nCh);
    info.hr_mode = 1;
    int n = emg_frame_build_info(frame.data(), frame.size(), &info);
    out.append(reinterpret_cast<const char *>(frame.data()), n);

    const long blocks = long(seconds) * kRate / kNSamp;
    const long perSec = kRate / kNSamp;
    std::uint32_t seq = 0;
    for (long b = 0; b < blocks; ++b) {
        if (b > 0 && b % perSec == 0) {
            n = emg_frame_build_info(frame.data(), frame.size(), &info);
            out.append(reinterpret_cast<const char *>(frame.data()), n);
        }
        for (int s = 0; s < kNSamp; ++s) {
            const double t = double(b * kNSamp + s) / kRate;
            for (int c = 0; c < nCh; ++c) {
                block[std::size_t(c) * kNSamp + std::size_t(s)] =
                    std::int32_t((2000.0 + ampUv * std::sin(2 * kPi * freqHz * t)) / kUvPerCode);
            }
        }
        n = emg_frame_build_data(frame.data(), frame.size(), seq++,
                                 std::uint32_t(b * kNSamp * 1000 / kRate), 0, block.data(), kNSamp,
                                 std::uint8_t(nCh), kNSamp, EMG_CH_MASK_CONTIGUOUS);
        out.append(reinterpret_cast<const char *>(frame.data()), n);
    }
    return out;
}

double colourDistance(const QColor &a, const QColor &b)
{
    return std::sqrt(std::pow(a.redF() - b.redF(), 2) + std::pow(a.greenF() - b.greenF(), 2) +
                     std::pow(a.blueF() - b.blueF(), 2));
}

double minY(const trace::Built &b)
{
    double m = 1e9;
    for (const auto &l : b.lines) {
        for (const QPointF &p : l) {
            m = std::min(m, p.y());
        }
    }
    return m;
}

double maxY(const trace::Built &b)
{
    double m = -1e9;
    for (const auto &l : b.lines) {
        for (const QPointF &p : l) {
            m = std::max(m, p.y());
        }
    }
    return m;
}

} // namespace

class TstWaveItem : public QObject {
    Q_OBJECT

private:
    Acquisition *acq = nullptr;
    SignalModel *model = nullptr;
    WaveItem *item = nullptr;
    WaveTrace *trace = nullptr;
    QTemporaryDir dir;

    QImage render(WaveItem *it = nullptr)
    {
        QImage img(kW, kH, QImage::Format_ARGB32_Premultiplied);
        img.fill(kBg);
        QPainter p(&img);
        (it ? it : item)->paint(&p);
        return img;
    }

    void replay(double freq, double ampUv, int nCh, double speed, int seconds = 40)
    {
        const QString path = QDir(dir.path()).filePath(QStringLiteral("sine.emgraw"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(sineCapture(seconds, freq, ampUv, nCh));
        f.close();

        acq->openReplay(QUrl::fromLocalFile(path));
        acq->setReplaySpeed(speed);
        QTRY_VERIFY_WITH_TIMEOUT(acq->isRunning() && acq->haveInfo(), 5000);
    }

    // Replay the sine, wait for 10 s of trusted samples, freeze: a static picture.
    void startAndFreeze(double freq, double ampUv)
    {
        replay(freq, ampUv, 1, 6.0);
        if (QTest::currentTestFailed()) {
            return;
        }
        QTRY_VERIFY_WITH_TIMEOUT(model->availableSamples() >= 10500 || !acq->isRunning(), 15000);
        QVERIFY(model->availableSamples() >= 10500);
        model->setPaused(true);                         // also pauses the replay
        QTest::qWait(100);                              // a frame tick refreshes the cached head
    }

private slots:
    void initTestCase();
    void init();
    void cleanup();

    void modelGainAndWindowReachTheGeometry();
    void markerLandsOnItsSampleAndNewestSampleIsVisible();
    void offlineDrawsNoMarkersAndNoTrace();
    void rowsAreBuiltAgainstOneSharedHead();
    void theStaticLayerDoesNotDrawTheTrace();
};

void TstWaveItem::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("FES_Board_Test"));
    QCoreApplication::setApplicationName(QStringLiteral("waveitem-test"));
    qRegisterMetaType<emg::InfoFrame>("emg::InfoFrame");
    qRegisterMetaType<emg::ParserStats>("emg::ParserStats");
    qRegisterMetaType<emg::FilterConfig>("emg::FilterConfig");
    QVERIFY(dir.isValid());
}

void TstWaveItem::init()
{
    QSettings().clear();
    acq = new Acquisition(nullptr);
    model = new SignalModel(acq);
    model->setSignal(SignalModel::Ecg);
    model->setGlow(false);
    model->setWindowSeconds(5.0);
    model->setGain(1.0);

    item = new WaveItem;
    item->setSize(QSizeF(kW, kH));
    item->setModel(model);
    item->setProperty("row", 0);
    item->setProperty("markerLine", kMarker);

    trace = new WaveTrace;
    trace->setSize(QSizeF(kW, kH));
    trace->setModel(model);
    trace->setProperty("row", 0);
}

void TstWaveItem::cleanup()
{
    delete trace;
    trace = nullptr;
    delete item;
    item = nullptr;
    delete model;
    model = nullptr;
    delete acq;
    acq = nullptr;
}

void TstWaveItem::modelGainAndWindowReachTheGeometry()
{
    // 1 mV, 2 Hz, through the real replay, filter and ring. The display filter passes
    // 2 Hz untouched, so the crests are exactly 1000 uV and the geometry's job is the
    // only thing between that and a pixel.
    startAndFreeze(2.0, 1000.0);
    if (QTest::currentTestFailed()) {
        return;
    }

    // Amplitude: ECG half-range 1.6 mV at gain 1, so crests at 100 -/+ 62.5.
    model->setGain(1.0);
    trace::Built b = trace->buildCurrent();
    QVERIFY2(!b.lines.isEmpty(), "the model produced no geometry");
    QVERIFY2(std::abs(minY(b) - 37.5) < 0.1, qPrintable(QStringLiteral("gain x1: crest at %1").arg(minY(b))));
    QVERIFY(std::abs(maxY(b) - 162.5) < 0.1);

    // The spec's gains: x2 halves the half-range, x0.5 doubles it.
    model->setGain(2.0);
    b = trace->buildCurrent();
    QVERIFY2(std::abs(minY(b) - (-25.0)) < 0.1, qPrintable(QStringLiteral("gain x2: crest at %1").arg(minY(b))));
    model->setGain(0.5);
    b = trace->buildCurrent();
    QVERIFY2(std::abs(minY(b) - 68.75) < 0.1, qPrintable(QStringLiteral("gain x0.5: crest at %1").arg(minY(b))));
    model->setGain(1.0);

    // Time axis: the window is what the model says it is. 2 Hz has a 0.5 s period, so
    // the number of crests visible is window / 0.5.
    for (double window : {2.5, 5.0, 10.0}) {
        model->setWindowSeconds(window);
        const trace::Built w = trace->buildCurrent();
        // Count crests by clustering the vertices near the crest height by x. (On
        // min/max-reduced data the vertices zig-zag between a column's low and high,
        // so "a run of vertices at the top" would count one crest several times.)
        const double periodPx = 0.5 / window * kW;
        std::vector<double> near;
        for (const auto &l : w.lines) {
            for (const QPointF &pt : l) {
                // Within 6 px of the crest, not 0.4: at 10 s the window reaches back to the
                // first second of the recording, where the 0.5 Hz high-pass has not yet
                // settled and early crests are a few percent short. This test counts
                // crests (the TIME axis); their exact height is checked above and in
                // tst_tracegeometry.
                if (pt.y() < 37.5 + 6.0) {
                    near.push_back(pt.x());
                }
            }
        }
        std::sort(near.begin(), near.end());
        int crests = 0;
        for (std::size_t i = 0; i < near.size();) {
            ++crests;
            std::size_t j = i;
            while (j < near.size() && near[j] - near[i] < periodPx * 0.5) {
                ++j;
            }
            i = j;
        }
        const int expected = int(window / 0.5);
        QVERIFY2(std::abs(crests - expected) <= 1,
                 qPrintable(QStringLiteral("window %1 s: %2 crests, expected about %3")
                                .arg(window).arg(crests).arg(expected)));
    }
}

void TstWaveItem::markerLandsOnItsSampleAndNewestSampleIsVisible()
{
    startAndFreeze(2.0, 1000.0);
    if (QTest::currentTestFailed()) {
        return;
    }

    // A marker on the newest displayed sample - what pressing M while frozen
    // produces. It must be drawn at the right edge, not one sample past it.
    model->addMarker();
    const QImage img = render();

    // The marker is accent-300 at 70 % alpha over the background, so on screen it is
    // a BLEND (158, 155, 192), not the token (210, 206, 253): 0.37 apart in colour
    // space. A first version of this test matched the pure token with a 0.34
    // tolerance, saw nothing, and failed for that reason rather than for the bug it
    // was written to catch.
    const QColor blended(qRound(0.7 * kMarker.red() + 0.3 * kBg.red()),
                         qRound(0.7 * kMarker.green() + 0.3 * kBg.green()),
                         qRound(0.7 * kMarker.blue() + 0.3 * kBg.blue()));

    std::vector<int> cols;
    for (int x = 0; x < img.width(); ++x) {
        int hits = 0;
        for (int y = 0; y < img.height(); ++y) {
            // a dashed 1 px line: about half the rows carry it
            if (colourDistance(img.pixelColor(x, y), blended) < 0.06) {
                ++hits;
            }
        }
        if (hits > img.height() / 4) {
            cols.push_back(x);
        }
    }
    QVERIFY2(!cols.empty(), "a marker on the newest sample was not drawn at all");
    QVERIFY2(cols.back() >= kW - 3, qPrintable(QStringLiteral("marker at x=%1, expected ~%2")
                                                    .arg(cols.back()).arg(kW - 1)));

    // The stamp is the newest SAMPLE (head - 1), not the head: the head is the next
    // sample to arrive, one position past the right edge.
    const auto markers = model->markersForView();
    QCOMPARE(markers.size(), 1);
    QCOMPARE(std::uint64_t(markers.first().sampleIndex), model->displayHead() - 1);
}

void TstWaveItem::offlineDrawsNoMarkersAndNoTrace()
{
    // Nothing connected: grid and a label, never a trace and never a marker - an
    // offline channel that drew stale data would read as live.
    QVERIFY(trace->buildCurrent().lines.isEmpty());
    const QImage img = render();
    const QColor blended(158, 155, 192);
    for (int x = 0; x < img.width(); ++x) {
        int hits = 0;
        for (int y = 0; y < img.height(); ++y) {
            if (colourDistance(img.pixelColor(x, y), blended) < 0.06) {
                ++hits;
            }
        }
        QVERIFY2(hits < img.height() / 4, "a marker-coloured line was drawn while offline");
    }
}

// Every row must be built against the SAME head. Each row used to read the ring's
// write counter at its own paint time, so with the producer running the rows were
// positioned against heads a few milliseconds apart: leads that carry the same beat
// came out staggered by up to ~115 ms on a live ECG, which is the one thing an ECG is
// read for. Identical channels, built back to back while the producer runs fast, must
// give identical geometry.
void TstWaveItem::rowsAreBuiltAgainstOneSharedHead()
{
    replay(2.0, 1000.0, 2, 25.0, 150);               // ~25 samples per millisecond
    if (QTest::currentTestFailed()) {
        return;
    }
    QTRY_VERIFY_WITH_TIMEOUT(model->availableSamples() >= 5500, 15000);

    WaveTrace second;
    second.setSize(QSizeF(kW, kH));
    second.setModel(model);
    second.setProperty("row", 1);

    int differing = 0;
    const int trials = 12;
    for (int i = 0; i < trials; ++i) {
        QTest::qWait(40);                            // let the model's frame tick run
        QVERIFY2(acq->isRunning(), "the replay ended before the comparison finished");
        const trace::Built a = trace->buildCurrent();
        const trace::Built b = second.buildCurrent();
        bool same = a.lines.size() == b.lines.size() && a.haveHead == b.haveHead;
        for (int k = 0; same && k < a.lines.size(); ++k) {
            same = a.lines[k] == b.lines[k];
        }
        if (!same) {
            ++differing;
        }
    }
    QVERIFY2(differing == 0, qPrintable(QStringLiteral("%1 of %2 trials built identical channels "
                                                       "at different positions").arg(differing).arg(trials)));
}

// WaveItem is the static layer. If it started drawing the trace again it would cost
// 77 ms a frame and put the application back at 100 % of a core.
void TstWaveItem::theStaticLayerDoesNotDrawTheTrace()
{
    startAndFreeze(2.0, 1000.0);
    if (QTest::currentTestFailed()) {
        return;
    }
    const QImage img = render();
    const QColor traceColour(0xB5, 0xAB, 0xFC);
    int traceLike = 0;
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            if (colourDistance(img.pixelColor(x, y), traceColour) < 0.18) {
                ++traceLike;
            }
        }
    }
    QVERIFY2(traceLike == 0, qPrintable(QStringLiteral("%1 trace-coloured pixels in the static layer").arg(traceLike)));
}

int main(int argc, char **argv)
{
    // Paints into an image; needs a GUI application but no real display.
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }
    QGuiApplication app(argc, argv);
    TstWaveItem tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_waveitem.moc"
