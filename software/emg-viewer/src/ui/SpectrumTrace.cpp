#include "SpectrumTrace.h"

#include "GpuLayers.h"
#include "SpectrumLayout.h"
#include "core/SignalProfile.h"
#include "core/SpectrumEngine.h"
#include "model/SignalModel.h"

#include <algorithm>
#include <cmath>

namespace {

// Spec section 6.2.
constexpr float kLineHalfWidth = 0.7f;           // 1.4 px
constexpr float kFeather = 0.75f;
constexpr float kGlowCore = 1.5f;
constexpr float kGlowFeather = 3.6f;
constexpr float kGlowAlpha = 0.20f;
constexpr float kFillTopAlpha = 0.32f;           // accent 32 % -> 0 % under the curve
constexpr float kBarsAlpha = 0.85f;
constexpr float kPeakDotRadius = 1.5f;           // 3 px
constexpr float kPeakGlowRadius = 6.0f;

enum Layer { Fill, Bars, Glow, Line, PeakGlow, PeakDot, LayerCount };
using Root = trace::GpuLayers<LayerCount>;

std::uint32_t rgba(const QColor &c) { return c.rgba(); }

} // namespace

SpectrumTrace::SpectrumTrace(QQuickItem *parent) : QQuickItem(parent)
{
    setFlag(ItemHasContents);
    connect(this, &SpectrumTrace::styleChanged, this, [this] { update(); });
}

void SpectrumTrace::setModel(SignalModel *m)
{
    if (m == m_model) {
        return;
    }
    if (m_model) {
        m_model->disconnect(this);
    }
    m_model = m;
    if (m_model) {
        // New spectra arrive at ~12 Hz; that is the only thing that moves.
        connect(m_model, &SignalModel::spectraUpdated, this, [this] { update(); });
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

QSGNode *SpectrumTrace::updatePaintNode(QSGNode *old, UpdatePaintNodeData *)
{
    auto *root = static_cast<Root *>(old);
    if (!root) {
        root = new Root;
    }

    trace::Mesh mesh[LayerCount];

    const double w = width(), h = height();
    if (m_model && w > 2.0 && spectrumlayout::plotHeight(h) > 2.0 && m_model->rowHasData(m_row)) {
        const emg::SpectrumEngine &eng = m_model->spectrum(m_row);

        if (eng.valid()) {
            const emg::SignalProfile &prof = m_model->profile();
            const std::vector<float> &db = eng.powerDb();
            const double gain = m_model->gain();
            const double top = eng.axisTopDb(gain);
            const double bottom = eng.axisBottomDb(gain);
            const double df = eng.binHz();
            const float baseline = float(spectrumlayout::plotBottom(h));
            const float plotTop = float(spectrumlayout::plotTop());

            auto yOf = [&](double v) { return spectrumlayout::yOfDb(v, bottom, top, h); };
            auto xOf = [&](double hz) { return spectrumlayout::xOfHz(hz, prof.displayMaxHz, w); };

            if (m_model->spectrumStyle() == SignalModel::Bars) {
                // The spec's alternative setting: one bar per bin.
                const float bw = std::max(1.0f, float(xOf(df) * 0.7));
                for (std::size_t k = 1; k < db.size(); ++k) {
                    const float x = float(xOf(double(k) * df));
                    trace::rect(mesh[Bars], x - bw / 2.0f, float(yOf(db[k])), x + bw / 2.0f, baseline,
                                rgba(m_line), kBarsAlpha);
                }
            } else {
                QPolygonF curve;
                curve.reserve(int(db.size()));
                for (std::size_t k = 0; k < db.size(); ++k) {
                    curve << QPointF(xOf(double(k) * df), yOf(db[k]));
                }

                trace::fillBelow(curve, baseline, plotTop, rgba(m_accent), kFillTopAlpha, mesh[Fill]);
                if (m_model->glow()) {
                    trace::stroke(curve, kGlowCore, kGlowFeather, 0.0f, rgba(m_accent), mesh[Glow]);
                    for (trace::Vertex &v : mesh[Glow].vertices) {
                        v.r = std::uint8_t(v.r * kGlowAlpha);
                        v.g = std::uint8_t(v.g * kGlowAlpha);
                        v.b = std::uint8_t(v.b * kGlowAlpha);
                        v.a = std::uint8_t(v.a * kGlowAlpha);
                    }
                }
                trace::stroke(curve, kLineHalfWidth, kFeather, kLineHalfWidth, rgba(m_line), mesh[Line]);
            }

            if (eng.peakHz() > 0.0) {
                const QPointF c(xOf(eng.peakHz()), yOf(eng.peakDb()));
                trace::disc(mesh[PeakGlow], c, 0.0f, kPeakGlowRadius, rgba(m_accent), 0.55f);
                trace::disc(mesh[PeakDot], c, kPeakDotRadius, kPeakDotRadius + kFeather, rgba(m_peakDot), 1.0f);
            }
        }
    }

    for (int i = 0; i < LayerCount; ++i) {
        root->upload(i, mesh[i]);
    }
    return root;
}
