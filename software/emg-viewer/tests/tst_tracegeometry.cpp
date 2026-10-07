// trace::build and trace::stroke - the scale of a trace, as numbers.
//
// The amplitude and the time axis of a waveform are what a reader trusts, and the
// project has twice shipped them wrong while the picture looked right (a 1.8x
// amplitude, a stretched time axis). They live in trace::build(), which is a pure
// function, so they are checked here exactly - to a hundredth of a pixel - rather
// than by measuring a rendered image.

#include <QTest>

#include "ui/TraceGeometry.h"

#include <algorithm>
#include <cmath>
#include <vector>

using namespace trace;

namespace {

constexpr double kPi = 3.14159265358979323846;

Params scroll(double w = 800, double h = 200, double halfRange = 1600, std::size_t ws = 5000)
{
    Params p;
    p.width = w;
    p.height = h;
    p.halfRangeUv = halfRange;
    p.windowSamples = ws;
    return p;
}

std::vector<float> sine(std::size_t n, double freq, double rate, double amp)
{
    std::vector<float> v(n);
    for (std::size_t i = 0; i < n; ++i) {
        v[i] = float(amp * std::sin(2 * kPi * freq * double(i) / rate));
    }
    return v;
}

double minY(const Built &b)
{
    double m = 1e9;
    for (const auto &l : b.lines) {
        for (const QPointF &p : l) {
            m = std::min(m, p.y());
        }
    }
    return m;
}

double maxY(const Built &b)
{
    double m = -1e9;
    for (const auto &l : b.lines) {
        for (const QPointF &p : l) {
            m = std::max(m, p.y());
        }
    }
    return m;
}

std::size_t pointCount(const Built &b)
{
    std::size_t n = 0;
    for (const auto &l : b.lines) {
        n += std::size_t(l.size());
    }
    return n;
}

} // namespace

class TstTraceGeometry : public QObject {
    Q_OBJECT

private slots:
    // ---- amplitude -----------------------------------------------------------------
    void amplitudeIsExactAtTheCrests();
    void amplitudeScalesInverselyWithRange_data();
    void amplitudeScalesInverselyWithRange();
    void zeroIsOnTheCentreLine();
    void positiveIsUp();
    void overRangeIsNotRescaled();

    // ---- time axis ---------------------------------------------------------------------
    void newestSampleIsOnTheRightEdge();
    void samplesAreEvenlySpacedAcrossTheWindow();
    void aPartialWindowIsAnchoredRightNotStretched();
    void peakSpacingMatchesThePeriodAtEveryWindow();

    // ---- decimation --------------------------------------------------------------------
    void denseDataBecomesMinMaxPerColumn();
    void aSingleSampleSpikeSurvivesDecimation();
    void sparseDataIsNotDecimated();

    // ---- sweep ----------------------------------------------------------------------------
    void sweepPositionsAreModuloTheWindow();
    void sweepBreaksAtTheWrapAndAroundTheGap();

    // ---- stroke -------------------------------------------------------------------------------
    void strokeCoreIsOpaqueAndSkirtFadesToNothing();
    void strokeIsCentredOnTheLine();
    void strokeWidthMatchesTheRequest();
    void degenerateSegmentsProduceNothing();
    void extendClosesTheGapAtAJoint();
    void colourIsPremultiplied();
    void indicesStayInsideTheVertexArray();

    // ---- shapes ---------------------------------------------------------------------------
    void fillAlphaFallsLinearlyToTheBaseline();
    void fillBaselineVerticesAreTransparent();
    void fillOfNothingIsNothing();
    void fadeBandRunsFromOpaqueToClear();
    void discHasAnOpaqueCoreAndAClearRim();
    void rectIsSolidAndCorrectlySized();
    void sampleXAgreesWithTheTrace();
};

// =========================================================== amplitude

void TstTraceGeometry::amplitudeIsExactAtTheCrests()
{
    // 1 mV on a +/-1.6 mV, 200 px lane: crest at 100 - 1.0/1.6 * 100 = 37.5 px.
    // To a hundredth of a pixel - no fill factor, no halving.
    const auto s = sine(5000, 2.0, 1000.0, 1000.0);
    const Built b = build(s.data(), s.size(), 5000, scroll());
    QVERIFY(std::abs(minY(b) - 37.5) < 0.01);
    QVERIFY(std::abs(maxY(b) - 162.5) < 0.01);
}

void TstTraceGeometry::amplitudeScalesInverselyWithRange_data()
{
    QTest::addColumn<double>("halfRange");
    QTest::addColumn<double>("expectedTop");
    // The spec's gains: x1 -> 1600, x2 -> 800, x0.5 -> 3200 uV half-range.
    QTest::newRow("gain x1")   << 1600.0 << 37.5;
    QTest::newRow("gain x2")   << 800.0  << -25.0;    // over range: 125 px above centre
    QTest::newRow("gain x0.5") << 3200.0 << 68.75;
}

void TstTraceGeometry::amplitudeScalesInverselyWithRange()
{
    QFETCH(double, halfRange);
    QFETCH(double, expectedTop);
    const auto s = sine(5000, 2.0, 1000.0, 1000.0);
    const Built b = build(s.data(), s.size(), 5000, scroll(800, 200, halfRange));
    QVERIFY2(std::abs(minY(b) - expectedTop) < 0.01, qPrintable(QString::number(minY(b))));
}

void TstTraceGeometry::zeroIsOnTheCentreLine()
{
    const std::vector<float> zeros(1000, 0.0f);
    const Built b = build(zeros.data(), zeros.size(), 1000, scroll(800, 200, 1600, 1000));
    QCOMPARE(minY(b), 100.0);
    QCOMPARE(maxY(b), 100.0);
}

void TstTraceGeometry::positiveIsUp()
{
    const std::vector<float> up(100, 800.0f);
    const Built b = build(up.data(), up.size(), 100, scroll(800, 200, 1600, 100));
    QVERIFY(minY(b) < 100.0);                       // y grows downwards
    QVERIFY(std::abs(minY(b) - 50.0) < 0.01);       // half of 100 px up
}

void TstTraceGeometry::overRangeIsNotRescaled()
{
    // A signal beyond the range is drawn beyond the lane (and clipped there by the
    // item). It must not be squashed to fit: that would be an unstated gain.
    const std::vector<float> big(100, 4800.0f);
    const Built b = build(big.data(), big.size(), 100, scroll(800, 200, 1600, 100));
    QVERIFY(std::abs(minY(b) - (100.0 - 4800.0 / 1600.0 * 100.0)) < 0.01);   // -200
}

// =========================================================== time axis

void TstTraceGeometry::newestSampleIsOnTheRightEdge()
{
    const auto s = sine(5000, 2.0, 1000.0, 1000.0);
    const Built b = build(s.data(), s.size(), 5000, scroll());
    QVERIFY(b.haveHead);
    QVERIFY(std::abs(b.head.x() - 800.0) < 1e-6);
    // and the oldest of a full window is on the left edge
    QVERIFY(std::abs(b.lines.first().first().x() - 0.0) < 1.0);
}

void TstTraceGeometry::samplesAreEvenlySpacedAcrossTheWindow()
{
    // Fewer samples than 2 x width, so no decimation: one vertex per sample, and
    // the spacing is width / (window - 1) exactly.
    const auto s = sine(1000, 2.0, 1000.0, 1000.0);
    const Built b = build(s.data(), s.size(), 1000, scroll(800, 200, 1600, 1000));
    QCOMPARE(b.lines.size(), 1);
    const QPolygonF &l = b.lines.first();
    QCOMPARE(l.size(), 1000);
    const double step = 800.0 / 999.0;
    for (int i = 1; i < l.size(); ++i) {
        QVERIFY(std::abs((l[i].x() - l[i - 1].x()) - step) < 1e-9);
    }
}

void TstTraceGeometry::aPartialWindowIsAnchoredRightNotStretched()
{
    // 500 of a 1000-sample window: the trace covers the RIGHT half only. Stretching
    // it across the whole width would draw it at half the time scale the grid says.
    const auto s = sine(500, 2.0, 1000.0, 1000.0);
    const Built b = build(s.data(), s.size(), 500, scroll(800, 200, 1600, 1000));
    const QPolygonF &l = b.lines.first();
    QVERIFY2(std::abs(l.last().x() - 800.0) < 1e-6, "newest sample must be at the right edge");
    const double expectedLeft = 800.0 - 499.0 * (800.0 / 999.0);
    QVERIFY2(std::abs(l.first().x() - expectedLeft) < 1e-6,
             qPrintable(QStringLiteral("starts at %1, expected %2").arg(l.first().x()).arg(expectedLeft)));
    QVERIFY(l.first().x() > 390.0);                  // genuinely the right half
}

void TstTraceGeometry::peakSpacingMatchesThePeriodAtEveryWindow()
{
    // 2 Hz at 1000 SPS: a crest every 500 samples = 0.5 s. Wherever the window is,
    // crests are 0.5/window of the width apart. Measured on vertex positions.
    for (double window : {2.5, 5.0, 10.0}) {
        const auto ws = std::size_t(window * 1000.0);
        const auto s = sine(ws, 2.0, 1000.0, 1000.0);
        const Built b = build(s.data(), s.size(), ws, scroll(800, 200, 1600, ws));

        // Crests: every vertex within 0.3 px of the crest height, grouped into one
        // cluster per crest (a crest is wider than a vertex, and a min/max-reduced
        // trace has several vertices near it), then each cluster's mean x.
        const double period = 0.5 / window * 800.0;
        std::vector<double> near;
        for (const auto &line : b.lines) {
            for (const QPointF &pt : line) {
                if (pt.y() < 37.5 + 0.3) {
                    near.push_back(pt.x());
                }
            }
        }
        std::sort(near.begin(), near.end());
        std::vector<double> crestX;
        std::size_t i = 0;
        while (i < near.size()) {
            std::size_t j = i;
            double sum = 0.0;
            while (j < near.size() && near[j] - near[i] < period * 0.5) {
                sum += near[j++];
            }
            crestX.push_back(sum / double(j - i));
            i = j;
        }
        QVERIFY2(crestX.size() >= 4, qPrintable(QStringLiteral("window %1: %2 crests").arg(window).arg(crestX.size())));
        const double mean = (crestX.back() - crestX.front()) / double(crestX.size() - 1);
        const double expected = 0.5 / window * 800.0;
        QVERIFY2(std::abs(mean - expected) < 0.5,
                 qPrintable(QStringLiteral("window %1 s: spacing %2 px, expected %3").arg(window).arg(mean).arg(expected)));
    }
}

// =========================================================== decimation

void TstTraceGeometry::denseDataBecomesMinMaxPerColumn()
{
    // 20 000 samples across 800 px: 25 per column. Two vertices per column, and the
    // vertex count tracks the WIDTH, not the data.
    const auto s = sine(20000, 7.0, 2000.0, 1000.0);
    const Built b = build(s.data(), s.size(), 20000, scroll(800, 200, 1600, 20000));
    QVERIFY2(pointCount(b) <= std::size_t(2 * 800 + 8), qPrintable(QString::number(pointCount(b))));
    QVERIFY2(pointCount(b) >= std::size_t(2 * 800 - 8), qPrintable(QString::number(pointCount(b))));
    // and the amplitude is unchanged by decimating
    QVERIFY(std::abs(minY(b) - 37.5) < 0.2);
}

void TstTraceGeometry::aSingleSampleSpikeSurvivesDecimation()
{
    // One-sample 1.5 mV spike in 20 000 samples of 10 uV noise: the thing stride
    // sampling drops and an EMG is read for.
    std::vector<float> s(20000);
    for (std::size_t i = 0; i < s.size(); ++i) {
        s[i] = float(10.0 * std::sin(0.3 * double(i)));
    }
    s[12345] = 1500.0f;
    const Built b = build(s.data(), s.size(), 20000, scroll(800, 200, 1600, 20000));
    QVERIFY2(std::abs(minY(b) - (100.0 - 1500.0 / 1600.0 * 100.0)) < 0.01,
             qPrintable(QStringLiteral("spike reached y=%1").arg(minY(b))));
}

void TstTraceGeometry::sparseDataIsNotDecimated()
{
    // 1000 samples across 800 px is under 2 per pixel: every sample is a vertex.
    const auto s = sine(1000, 5.0, 1000.0, 500.0);
    const Built b = build(s.data(), s.size(), 1000, scroll(800, 200, 1600, 1000));
    QCOMPARE(pointCount(b), std::size_t(1000));
}

// =========================================================== sweep

void TstTraceGeometry::sweepPositionsAreModuloTheWindow()
{
    Params p = scroll(800, 200, 1600, 1000);
    p.sweep = true;
    const auto s = sine(400, 2.0, 1000.0, 1000.0);
    // head = 2400: the 400 newest samples are indices 2000..2399, i.e. positions
    // 0..399 of the 1000-sample sweep -> x in the left 40 %.
    const Built b = build(s.data(), s.size(), 2400, p);
    QVERIFY(b.haveHead);
    QVERIFY(std::abs(b.headX - 2400 % 1000 / 1000.0 * 800.0) < 1e-9);    // 320
    QVERIFY(std::abs(b.head.x() - 399.0 / 1000.0 * 800.0) < 1e-6);
    for (const auto &l : b.lines) {
        for (const QPointF &pt : l) {
            QVERIFY(pt.x() >= 0.0 && pt.x() <= 320.0 + 1e-6);
        }
    }
}

void TstTraceGeometry::sweepBreaksAtTheWrapAndAroundTheGap()
{
    Params p = scroll(800, 200, 1600, 1000);
    p.sweep = true;
    const auto s = sine(1000, 2.0, 1000.0, 1000.0);
    // A full window ending mid-sweep: head 1300 -> write position 300 -> x 240.
    const Built b = build(s.data(), s.size(), 1300, p);

    // One continuous line would draw a stroke straight across the plot at the wrap.
    QVERIFY2(b.lines.size() >= 2, "a sweep trace must break where it wraps");
    for (const auto &l : b.lines) {
        for (int i = 1; i < l.size(); ++i) {
            QVERIFY2(l[i].x() >= l[i - 1].x() - 1e-9, "x must never run backwards inside one line");
        }
    }

    // And nothing is drawn inside the erase gap just ahead of the head.
    const double gapEnd = b.headX + b.gapWidth;
    QVERIFY(b.gapWidth > 0.0);
    for (const auto &l : b.lines) {
        for (const QPointF &pt : l) {
            QVERIFY2(!(pt.x() > b.headX + 0.5 && pt.x() < gapEnd - 0.5),
                     qPrintable(QStringLiteral("vertex at x=%1 inside the gap [%2,%3]")
                                    .arg(pt.x()).arg(b.headX).arg(gapEnd)));
        }
    }
}

// =========================================================== stroke

void TstTraceGeometry::strokeCoreIsOpaqueAndSkirtFadesToNothing()
{
    QPolygonF line;
    line << QPointF(10, 50) << QPointF(60, 50);
    Mesh m;
    stroke(line, 0.7f, 0.8f, 0.0f, 0xFFFFFFFFu, m);

    // Four vertices across at each end: skirt(0), core(1), core(1), skirt(0).
    QCOMPARE(m.vertices.size(), 8);
    int opaque = 0, clear = 0;
    for (const Vertex &v : m.vertices) {
        if (v.a == 255) {
            ++opaque;
        } else if (v.a == 0) {
            ++clear;
        }
    }
    QCOMPARE(opaque, 4);
    QCOMPARE(clear, 4);
}

void TstTraceGeometry::strokeIsCentredOnTheLine()
{
    QPolygonF line;
    line << QPointF(10, 50) << QPointF(60, 80);
    Mesh m;
    stroke(line, 0.7f, 0.8f, 0.0f, 0xFFFFFFFFu, m);
    double cx = 0, cy = 0;
    for (const Vertex &v : m.vertices) {
        cx += v.x;
        cy += v.y;
    }
    cx /= m.vertices.size();
    cy /= m.vertices.size();
    QVERIFY(std::abs(cx - 35.0) < 1e-4);
    QVERIFY(std::abs(cy - 65.0) < 1e-4);
}

void TstTraceGeometry::strokeWidthMatchesTheRequest()
{
    // A horizontal line: the opaque core is exactly 2 * halfWidth tall, the skirt
    // reaches halfWidth + feather out.
    QPolygonF line;
    line << QPointF(10, 50) << QPointF(60, 50);
    Mesh m;
    stroke(line, 0.7f, 0.8f, 0.0f, 0xFFFFFFFFu, m);

    float coreTop = 1e9f, coreBottom = -1e9f, edgeTop = 1e9f, edgeBottom = -1e9f;
    for (const Vertex &v : m.vertices) {
        edgeTop = std::min(edgeTop, v.y);
        edgeBottom = std::max(edgeBottom, v.y);
        if (v.a == 255) {
            coreTop = std::min(coreTop, v.y);
            coreBottom = std::max(coreBottom, v.y);
        }
    }
    QVERIFY(std::abs((coreBottom - coreTop) - 1.4f) < 1e-4f);
    QVERIFY(std::abs((edgeBottom - edgeTop) - 2.0f * 1.5f) < 1e-4f);   // 2 * (0.7 + 0.8)
}

void TstTraceGeometry::degenerateSegmentsProduceNothing()
{
    // Two coincident points have no direction; a NaN normal would put garbage
    // triangles on screen.
    QPolygonF line;
    line << QPointF(10, 50) << QPointF(10, 50) << QPointF(10, 50);
    Mesh m;
    stroke(line, 0.7f, 0.8f, 0.0f, 0xFFFFFFFFu, m);
    QCOMPARE(m.vertices.size(), 0);
    QCOMPARE(m.indices.size(), 0);

    // and a single point, and an empty line
    QPolygonF one;
    one << QPointF(5, 5);
    stroke(one, 0.7f, 0.8f, 0.0f, 0xFFFFFFFFu, m);
    stroke(QPolygonF(), 0.7f, 0.8f, 0.0f, 0xFFFFFFFFu, m);
    QCOMPARE(m.vertices.size(), 0);

    for (const Vertex &v : m.vertices) {
        QVERIFY(std::isfinite(v.x) && std::isfinite(v.y));
    }
}

void TstTraceGeometry::extendClosesTheGapAtAJoint()
{
    QPolygonF line;
    line << QPointF(10, 50) << QPointF(60, 50);
    Mesh plain, capped;
    stroke(line, 0.7f, 0.8f, 0.0f, 0xFFFFFFFFu, plain);
    stroke(line, 0.7f, 0.8f, 0.7f, 0xFFFFFFFFu, capped);

    auto xExtent = [](const Mesh &m) {
        float lo = 1e9f, hi = -1e9f;
        for (const Vertex &v : m.vertices) {
            lo = std::min(lo, v.x);
            hi = std::max(hi, v.x);
        }
        return std::make_pair(lo, hi);
    };
    QVERIFY(std::abs(xExtent(plain).first - 10.0f) < 1e-4f);
    QVERIFY(std::abs(xExtent(capped).first - 9.3f) < 1e-4f);      // 0.7 beyond each end
    QVERIFY(std::abs(xExtent(capped).second - 60.7f) < 1e-4f);
}

void TstTraceGeometry::colourIsPremultiplied()
{
    // QSGVertexColorMaterial takes premultiplied colour. A straight 50 %-alpha white
    // is (128,128,128,128), not (255,255,255,128), or the line comes out too bright.
    QPolygonF line;
    line << QPointF(10, 50) << QPointF(60, 50);
    Mesh m;
    stroke(line, 0.7f, 0.8f, 0.0f, 0x80FFFFFFu, m);       // 0xAARRGGBB
    for (const Vertex &v : m.vertices) {
        if (v.a == 0) {
            continue;                                      // the skirt
        }
        QVERIFY2(std::abs(int(v.a) - 128) <= 1, qPrintable(QString::number(v.a)));
        QVERIFY2(std::abs(int(v.r) - 128) <= 1, qPrintable(QString::number(v.r)));
        QVERIFY(std::abs(int(v.g) - 128) <= 1 && std::abs(int(v.b) - 128) <= 1);
    }
}

void TstTraceGeometry::indicesStayInsideTheVertexArray()
{
    QPolygonF line;
    for (int i = 0; i < 200; ++i) {
        line << QPointF(i * 3.0, 100 + 40 * std::sin(i * 0.3));
    }
    Mesh m;
    stroke(line, 0.7f, 0.8f, 0.7f, 0xFFB5ABFCu, m);
    QVERIFY(m.vertices.size() > 0);
    QCOMPARE(m.indices.size() % 3, 0);
    for (std::uint32_t i : m.indices) {
        QVERIFY(i < std::uint32_t(m.vertices.size()));
    }
    // 199 segments x (8 vertices, 18 indices)
    QCOMPARE(m.vertices.size(), 199 * 8);
    QCOMPARE(m.indices.size(), 199 * 18);
}

// ============================================================ shapes

void TstTraceGeometry::fillAlphaFallsLinearlyToTheBaseline()
{
    // The spectrum's "accent 32 % at the top fading to 0 under the curve". A vertex
    // ON the curve carries the gradient value for ITS height: 32 % at the plot top,
    // 0 at the baseline, linear between.
    QPolygonF curve;
    curve << QPointF(0, 8) << QPointF(10, 89) << QPointF(20, 170);   // top, middle, baseline
    Mesh m;
    fillBelow(curve, 170.0f, 8.0f, 0xFFFFFFFFu, 0.32f, m);

    auto alphaOf = [&](int curveIndex) { return m.vertices[2 * curveIndex].a / 255.0f; };
    QVERIFY(std::abs(alphaOf(0) - 0.32f) < 0.005f);                  // at the gradient top
    QVERIFY(std::abs(alphaOf(1) - 0.16f) < 0.005f);                  // halfway down: half
    QVERIFY(std::abs(alphaOf(2) - 0.0f) < 0.005f);                   // on the baseline: none
}

void TstTraceGeometry::fillBaselineVerticesAreTransparent()
{
    QPolygonF curve;
    for (int i = 0; i < 50; ++i) {
        curve << QPointF(i * 4.0, 60 + 30 * std::sin(i * 0.4));
    }
    Mesh m;
    fillBelow(curve, 170.0f, 8.0f, 0xFFB5ABFCu, 0.32f, m);

    QCOMPARE(m.vertices.size(), 100);
    for (int i = 0; i < 50; ++i) {
        const Vertex &onBaseline = m.vertices[2 * i + 1];
        QCOMPARE(int(onBaseline.a), 0);
        QVERIFY(std::abs(onBaseline.y - 170.0f) < 1e-4f);
        QVERIFY(std::abs(onBaseline.x - i * 4.0f) < 1e-4f);        // straight below its curve vertex
    }
    QCOMPARE(m.indices.size(), 49 * 6);
    for (std::uint32_t idx : m.indices) {
        QVERIFY(idx < std::uint32_t(m.vertices.size()));
    }
}

void TstTraceGeometry::fillOfNothingIsNothing()
{
    Mesh m;
    fillBelow(QPolygonF(), 170.0f, 8.0f, 0xFFFFFFFFu, 0.32f, m);
    QPolygonF one;
    one << QPointF(1, 2);
    fillBelow(one, 170.0f, 8.0f, 0xFFFFFFFFu, 0.32f, m);
    QCOMPARE(m.vertices.size(), 0);
}

void TstTraceGeometry::fadeBandRunsFromOpaqueToClear()
{
    Mesh m;
    fadeBand(m, 100.0f, 128.0f, 170.0f, 0xFF9184D9u, 0.18f);
    QCOMPARE(m.vertices.size(), 4);
    // The sweep erase gap: "18 % -> 0 %" left to right.
    QVERIFY(std::abs(m.vertices[0].a - 0.18f * 255.0f) <= 1.0f);
    QVERIFY(std::abs(m.vertices[2].a - 0.18f * 255.0f) <= 1.0f);
    QCOMPARE(int(m.vertices[1].a), 0);
    QCOMPARE(int(m.vertices[3].a), 0);
    QVERIFY(std::abs(m.vertices[0].x - 100.0f) < 1e-4f && std::abs(m.vertices[1].x - 128.0f) < 1e-4f);
    QVERIFY(std::abs(m.vertices[2].y - 170.0f) < 1e-4f);
}

void TstTraceGeometry::discHasAnOpaqueCoreAndAClearRim()
{
    Mesh m;
    disc(m, QPointF(50, 50), 1.3f, 2.1f, 0xFFE7E5FEu, 1.0f);
    QCOMPARE(m.vertices.size(), 1 + 20 + 20);
    QCOMPARE(int(m.vertices[0].a), 255);                                  // centre
    for (int i = 0; i < 20; ++i) {
        const Vertex &core = m.vertices[1 + i], &rim = m.vertices[21 + i];
        QCOMPARE(int(core.a), 255);
        QCOMPARE(int(rim.a), 0);
        QVERIFY(std::abs(std::hypot(core.x - 50.0f, core.y - 50.0f) - 1.3f) < 1e-3f);
        QVERIFY(std::abs(std::hypot(rim.x - 50.0f, rim.y - 50.0f) - 2.1f) < 1e-3f);
    }
    // a glow is a disc with a zero core and partial alpha
    Mesh g;
    disc(g, QPointF(50, 50), 0.0f, 5.0f, 0xFF9184D9u, 0.55f);
    QVERIFY(std::abs(g.vertices[0].a - 0.55f * 255.0f) <= 1.0f);
}

void TstTraceGeometry::rectIsSolidAndCorrectlySized()
{
    Mesh m;
    rect(m, 10, 20, 30, 50, 0xFFFFFFFFu, 0.5f);
    QCOMPARE(m.vertices.size(), 4);
    QCOMPARE(m.indices.size(), 6);
    float lo = 1e9f, hi = -1e9f;
    for (const Vertex &v : m.vertices) {
        QVERIFY(std::abs(v.a - 128) <= 1);
        lo = std::min(lo, v.x);
        hi = std::max(hi, v.x);
    }
    QVERIFY(std::abs(lo - 10.0f) < 1e-4f && std::abs(hi - 30.0f) < 1e-4f);
}

void TstTraceGeometry::sampleXAgreesWithTheTrace()
{
    // Markers use sampleX() and the trace uses it too, so a marker lands exactly on
    // the vertex of the sample it annotates - in both modes.
    const auto s = sine(1000, 2.0, 1000.0, 1000.0);
    for (bool sweep : {false, true}) {
        Params p = scroll(800, 200, 1600, 1000);
        p.sweep = sweep;
        const std::uint64_t head = 2300;
        const Built b = build(s.data(), s.size(), head, p);

        int checked = 0;
        for (const auto &line : b.lines) {
            for (const QPointF &pt : line) {
                // find which sample this vertex is: try the one at that x
                for (std::uint64_t g = head - 1000; g < head; g += 97) {
                    if (std::abs(sampleX(g, head, 1000, 800.0, sweep) - pt.x()) < 1e-9) {
                        ++checked;
                    }
                }
            }
        }
        QVERIFY2(checked > 0, sweep ? "no sweep vertex matched sampleX" : "no scroll vertex matched sampleX");
    }

    // The two defining points.
    QVERIFY(std::abs(sampleX(999, 1000, 1000, 800.0, false) - 800.0) < 1e-9);          // newest -> right edge
    QVERIFY(std::abs(sampleX(0, 1000, 1000, 800.0, false) - 0.0) < 1e-9);              // oldest -> left edge
    QVERIFY(std::abs(sampleX(1250, 0, 1000, 800.0, true) - 250.0 / 1000.0 * 800.0) < 1e-9);
}

QTEST_APPLESS_MAIN(TstTraceGeometry)
#include "tst_tracegeometry.moc"
