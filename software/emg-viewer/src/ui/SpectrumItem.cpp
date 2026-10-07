#include "SpectrumItem.h"

#include "SpectrumLayout.h"
#include "core/SignalProfile.h"
#include "core/SpectrumEngine.h"
#include "model/SignalModel.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPen>

#include <algorithm>
#include <cmath>

namespace {

// Spec section 6.2.
constexpr qreal kTopPad = 8.0;
constexpr qreal kBottomPad = 18.0;       // room for the Hz labels
constexpr int kHDivisions = 4;
constexpr qreal kBandAlphaA = 0.07;       // EEG band columns alternate 7 % / 3.5 %
constexpr qreal kBandAlphaB = 0.035;
constexpr qreal kPeakChipAlpha = 0.85;

} // namespace

SpectrumItem::SpectrumItem(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    setAntialiasing(true);
    setOpaquePainting(false);
    setRenderTarget(QQuickPaintedItem::Image);
    connect(this, &SpectrumItem::styleChanged, this, [this] { update(); });
}

void SpectrumItem::setModel(SignalModel *m)
{
    if (m == m_model) {
        return;
    }
    if (m_model) {
        m_model->disconnect(this);
    }
    m_model = m;
    if (m_model) {
        // New spectra move the peak, so the chip layer follows them; the grid layer is
        // static and is repainted only by the changes listed below.
        connect(m_model, &SignalModel::spectraUpdated, this, [this] {
            if (m_part == Chip) {
                update();
            }
        });
        connect(m_model, &SignalModel::frame, this, &SpectrumItem::onFrame);
        for (auto sig : {&SignalModel::gainChanged, &SignalModel::signalChanged,
                         &SignalModel::domainChanged, &SignalModel::glowChanged,
                         &SignalModel::spectrumStyleChanged, &SignalModel::rowsChanged,
                         &SignalModel::averagingChanged, &SignalModel::statusChanged}) {
            connect(m_model, sig, this, [this] { update(); });
        }
    }
    emit modelChanged();
    update();
}

void SpectrumItem::onFrame()
{
    if (!m_model || !isVisible()) {
        return;
    }
    // Fresh spectra repaint through spectraUpdated(). This slow tick only keeps a
    // frozen or offline plot from being handed back blank after an invalidation.
    if (!m_model->live() && (++m_idleFrames % 30) == 0) {
        update();
    }
}

void SpectrumItem::paint(QPainter *p)
{
    const qreal w = width();
    const qreal h = height();
    if (w < 2.0 || h < kTopPad + kBottomPad + 2.0 || !m_model) {
        return;
    }

    // The chip layer paints nothing but the peak's label; skip straight to it.
    const bool gridPart = (m_part == Grid);
    const emg::SignalProfile &prof = m_model->profile();
    const qreal plotTop = kTopPad;
    const qreal plotH = h - kTopPad - kBottomPad;
    const qreal plotBottom = plotTop + plotH;
    const double maxHz = prof.displayMaxHz;
    auto xOfHz = [&](double hz) { return qreal(hz / maxHz) * w; };

    QFont numFont(m_numberFamily);
    numFont.setPixelSize(10);
    QFont textFont(m_fontFamily);
    textFont.setPixelSize(10);

    // ---- EEG band columns (drawn first, behind everything) ------------------------
    if (gridPart && prof.showBands) {
        p->setPen(Qt::NoPen);
        p->setFont(textFont);
        int i = 0;
        for (const auto &b : emg::kEegBands) {
            QColor c = m_accent;
            c.setAlphaF((i % 2 == 0) ? kBandAlphaA : kBandAlphaB);
            const qreal x0 = xOfHz(b.loHz);
            const qreal x1 = xOfHz(std::min(b.hiHz, maxHz));
            p->fillRect(QRectF(x0, plotTop, x1 - x0, plotH), c);
            ++i;
        }
        // Greek labels, top-left of each column.
        p->setPen(m_bandText);
        for (const auto &b : emg::kEegBands) {
            p->drawText(QPointF(xOfHz(b.loHz) + 4, plotTop + 11), QString::fromUtf8(b.symbol));
        }
    }

    // ---- grid -------------------------------------------------------------------------
    const double step = prof.gridStepHz;
    const int steps = int(std::floor(maxHz / step + 1e-9));
    QFontMetricsF fm(numFont);
    if (gridPart) {
    p->setRenderHint(QPainter::Antialiasing, false);

    for (int k = 1; k <= steps; ++k) {              // vertical, every `step` Hz
        const bool major = (k % 2 == 0);
        p->setPen(QPen(major ? m_gridMajor : m_gridMinor, 1));
        const qreal x = std::floor(xOfHz(k * step)) + 0.5;
        p->drawLine(QPointF(x, plotTop), QPointF(x, plotBottom));
    }
    p->setPen(QPen(m_gridMinor, 1));
    for (int i = 0; i <= kHDivisions; ++i) {         // horizontal, 4 divisions
        const qreal y = std::floor(plotTop + plotH * i / kHDivisions) + 0.5;
        p->drawLine(QPointF(0, y), QPointF(w, y));
    }

    // Labels on the major steps: "0 Hz", "10", "20", ... Left-aligned at 0 and
    // right-aligned at the far edge so neither is clipped.
    p->setRenderHint(QPainter::TextAntialiasing, true);
    p->setFont(numFont);
    p->setPen(m_axisText);
    for (int k = 0; k <= steps; k += 2) {
        const double hz = k * step;
        const QString text = (k == 0) ? QStringLiteral("0 Hz") : QString::number(hz, 'f', 0);
        const qreal tw = fm.horizontalAdvance(text);
        qreal x = xOfHz(hz) - tw / 2.0;
        x = std::clamp(x, 0.0, w - tw);
        p->drawText(QPointF(x, h - 4), text);
    }
    } // gridPart

    // ---- spectrum ------------------------------------------------------------------------
    p->setRenderHint(QPainter::Antialiasing, true);

    if (!m_model->rowHasData(m_row)) {
        if (gridPart) {
            p->setFont(textFont);
            p->setPen(m_emptyText);
            p->drawText(QRectF(0, plotTop, w, plotH), Qt::AlignCenter,
                        m_model->online() ? QStringLiteral("No signal on this channel")
                                          : QStringLiteral("Device offline"));
        }
        return;
    }

    const emg::SpectrumEngine &eng = m_model->spectrum(m_row);
    if (!eng.valid()) {
        return;   // still collecting the first 512 samples
    }

    // The curve, fill, glow and bars are SpectrumTrace's (GPU). What is left here is
    // the peak's label, which is text and changes only when the peak moves.
    const double gain = m_model->gain();
    const double top = eng.axisTopDb(gain);
    const double bottom = eng.axisBottomDb(gain);
    auto yOfDb = [&](double v) { return qreal(spectrumlayout::yOfDb(v, bottom, top, h)); };

    // ---- peak marker ------------------------------------------------------------------------
    if (!gridPart && eng.peakHz() > 0.0) {
        const QPointF c(xOfHz(eng.peakHz()), yOfDb(eng.peakDb()));

        // (the dot and its glow are SpectrumTrace's)

        // "10.2 Hz" on a chip, above the dot unless that would leave the plot.
        p->setClipping(false);
        p->setFont(numFont);
        const QString label = QStringLiteral("%1 Hz").arg(eng.peakHz(), 0, 'f', 1);
        const qreal tw = fm.horizontalAdvance(label);
        const qreal cw = tw + 10, ch = 15;
        qreal cx = std::clamp(c.x() - cw / 2.0, 1.0, w - cw - 1.0);
        qreal cy = c.y() - ch - 6;
        if (cy < 1.0) {
            cy = c.y() + 7;       // peak is at the top of the plot: label goes below it
        }
        QColor chip = m_peakChip;
        chip.setAlphaF(kPeakChipAlpha);
        p->setPen(Qt::NoPen);
        p->setBrush(chip);
        p->drawRoundedRect(QRectF(cx, cy, cw, ch), 4, 4);
        p->setBrush(Qt::NoBrush);
        p->setPen(m_peakText);
        p->drawText(QRectF(cx, cy, cw, ch), Qt::AlignCenter, label);
    }
}
