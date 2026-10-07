#include "WaveItem.h"

#include "TraceGeometry.h"
#include "model/SignalModel.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPen>

#include <algorithm>
#include <cmath>

namespace {

// Spec section 6.1.
constexpr int kVDivisions = 25;          // vertical grid: 25 across, every 5th major
constexpr int kMajorEvery = 5;
constexpr int kHDivisions = 8;           // horizontal grid
constexpr qreal kMarkerAlpha = 0.70;

} // namespace

WaveItem::WaveItem(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    setAntialiasing(true);
    setOpaquePainting(false);
    // QImage target: rasterised on the CPU, so it is identical on every backend
    // and the software one can render it.
    setRenderTarget(QQuickPaintedItem::Image);

    connect(this, &WaveItem::styleChanged, this, [this] { update(); });
}

void WaveItem::setModel(SignalModel *m)
{
    if (m == m_model) {
        return;
    }
    if (m_model) {
        m_model->disconnect(this);
    }
    m_model = m;
    if (m_model) {
        // The shared 60 Hz tick drives live repaints; anything that changes what
        // the trace LOOKS like repaints at once, so a frozen view still reacts.
        connect(m_model, &SignalModel::frame, this, &WaveItem::onFrame);
        for (auto sig : {&SignalModel::gainChanged, &SignalModel::windowChanged,
                         &SignalModel::signalChanged, &SignalModel::domainChanged,
                         &SignalModel::displayModeChanged, &SignalModel::glowChanged,
                         &SignalModel::markersChanged, &SignalModel::rowsChanged,
                         &SignalModel::pausedChanged, &SignalModel::statusChanged}) {
            connect(m_model, sig, this, [this] { update(); });
        }
    }
    emit modelChanged();
    update();
}

void WaveItem::onFrame()
{
    if (!m_model || !isVisible()) {
        return;
    }
    // This layer is static: grid, marker lines, "device offline". Repainting it every
    // frame would rasterise and upload a lane-sized image 60 times a second for
    // nothing. Markers are the one thing in it that MOVES (they scroll with the
    // trace), so only while a live marker is on screen does it follow the frame tick.
    // Otherwise a slow refresh - not none - so that a scene-graph invalidation does
    // not hand it back blank.
    const bool markersMoving = m_model->live() && m_model->markerCount() > 0;
    if (markersMoving || (++m_idleFrames % 30) == 0) {
        update();
    }
}

// ------------------------------------------------------------------ grid

void WaveItem::drawGrid(QPainter *p, qreal w, qreal h) const
{
    p->setRenderHint(QPainter::Antialiasing, false);

    auto vline = [&](qreal x, const QColor &c) {
        p->setPen(QPen(c, 1));
        const qreal px = std::floor(x) + 0.5;   // land on a pixel, not between two
        p->drawLine(QPointF(px, 0), QPointF(px, h));
    };
    auto hline = [&](qreal y, const QColor &c) {
        p->setPen(QPen(c, 1));
        const qreal py = std::floor(y) + 0.5;
        p->drawLine(QPointF(0, py), QPointF(w, py));
    };

    // Vertical: minors first so the majors are drawn over them.
    for (int i = 1; i < kVDivisions; ++i) {
        if (i % kMajorEvery != 0) {
            vline(w * i / kVDivisions, m_gridMinor);
        }
    }
    for (int i = kMajorEvery; i < kVDivisions; i += kMajorEvery) {
        vline(w * i / kVDivisions, m_gridMajor);
    }

    // Horizontal: odd lines faintest, even lines stronger, centre strongest.
    for (int i = 1; i < kHDivisions; ++i) {
        if (i == kHDivisions / 2) {
            continue;
        }
        hline(h * i / kHDivisions, (i % 2 == 0) ? m_gridMajor : m_gridMinor);
    }
    hline(h / 2.0, m_gridCenter);
}

void WaveItem::drawEmpty(QPainter *p, qreal w, qreal h) const
{
    if (!m_model) {
        return;
    }
    // "Device-offline state must be visually obvious" (spec section 10). The
    // footer and the header icon say it too; this is the one on the trace itself.
    QFont f(m_fontFamily);
    f.setPixelSize(11);
    p->setFont(f);
    p->setPen(m_emptyText);
    p->setRenderHint(QPainter::TextAntialiasing, true);
    const QString text = m_model->online() ? QStringLiteral("No signal on this channel")
                                           : QStringLiteral("Device offline");
    p->drawText(QRectF(0, 0, w, h), Qt::AlignCenter, text);
}

// ------------------------------------------------------------------ paint

void WaveItem::paint(QPainter *p)
{
    const qreal w = width();
    const qreal h = height();
    if (w < 2.0 || h < 2.0) {
        return;
    }

    drawGrid(p, w, h);
    p->setRenderHint(QPainter::Antialiasing, true);
    p->setClipRect(QRectF(0, 0, w, h));

    if (!m_model) {
        return;
    }
    if (!m_model->rowHasData(m_row)) {
        drawEmpty(p, w, h);
        return;
    }

    // Markers. (The trace itself is WaveTrace's: drawing it here cost 77 ms a frame.)
    const auto markers = m_model->markersForView();
    if (markers.isEmpty()) {
        return;
    }

    const std::size_t Ws = std::max<std::size_t>(2, m_model->windowSamples());
    const std::uint64_t head = m_model->displayHead();     // the same head the trace uses
    const bool sweep = (m_model->displayMode() == SignalModel::Sweep);

    QColor lineColor = m_markerLine;
    lineColor.setAlphaF(kMarkerAlpha);
    QPen dash(lineColor, 1);
    dash.setDashPattern({3.0, 3.0});

    QFont f(m_fontFamily);
    f.setPixelSize(10);
    f.setWeight(QFont::Medium);
    p->setFont(f);
    const QFontMetricsF fm(f);

    for (const auto &mk : markers) {
        const auto idx = static_cast<std::uint64_t>(std::max<qint64>(0, mk.sampleIndex));
        if (idx >= head || head - idx > Ws) {
            continue;   // scrolled off the window
        }

        // The newest sample sits at x == w, the plot's right edge, where a 1 px line at
        // floor(w) + 0.5 would be wholly outside it. Keep the line on the last pixel
        // column instead, so a marker on the newest sample - what pressing M while
        // frozen produces - is visible.
        const qreal x = std::clamp(std::floor(trace::sampleX(idx, head, Ws, w, sweep)) + 0.5,
                                   0.5, w - 0.5);

        p->setRenderHint(QPainter::Antialiasing, false);
        p->setPen(dash);
        p->drawLine(QPointF(x, 0), QPointF(x, h));
        p->setRenderHint(QPainter::Antialiasing, true);

        if (m_markerLabels) {
            const QString label = QStringLiteral("M%1").arg(mk.number);
            const QRectF chip(x + 3, 4, fm.horizontalAdvance(label) + 8, 14);
            p->setPen(Qt::NoPen);
            p->setBrush(m_markerFill);
            p->drawRoundedRect(chip, 4, 4);
            p->setBrush(Qt::NoBrush);
            p->setPen(m_markerText);
            p->drawText(chip, Qt::AlignCenter, label);
        }
    }
}
