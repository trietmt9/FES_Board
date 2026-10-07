#include "WaveTrace.h"

#include "GpuLayers.h"
#include "core/SampleRing.h"
#include "model/SignalModel.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace {

// Spec section 6.1.
constexpr float kTraceHalfWidth = 0.7f;        // 1.4 px line
constexpr float kFeather = 0.75f;              // anti-aliasing skirt, each side
constexpr float kGlowCore = 1.5f;              // glow: soft, wide, translucent
constexpr float kGlowFeather = 3.6f;
constexpr float kGlowAlpha = 0.20f;
constexpr int kGlowStride = 3;                // glow is built from every 3rd vertex
constexpr float kHeadRadius = 1.3f;            // 2.6 px dot
constexpr float kHeadGlowRadius = 5.0f;        // 10 px glow
constexpr float kSweepBandAlpha = 0.18f;       // 18 % -> 0 %

// The five layers, back to front.
enum Layer { Band, Glow, Main, HeadGlow, HeadDot, LayerCount };
using Root = trace::GpuLayers<LayerCount>;

std::uint32_t rgba(const QColor &c)
{
    return c.rgba();   // QRgb is 0xAARRGGBB
}

} // namespace

WaveTrace::WaveTrace(QQuickItem *parent) : QQuickItem(parent)
{
    setFlag(ItemHasContents);
    // A crest beyond the range runs past the lane; it is clipped to it, which is the
    // honest rendering of an over-range signal (see trace::build).
    setClip(true);
    connect(this, &WaveTrace::styleChanged, this, [this] { update(); });
}

void WaveTrace::setModel(SignalModel *m)
{
    if (m == m_model) {
        return;
    }
    if (m_model) {
        m_model->disconnect(this);
    }
    m_model = m;
    if (m_model) {
        connect(m_model, &SignalModel::frame, this, &WaveTrace::onFrame);
        for (auto sig : {&SignalModel::gainChanged, &SignalModel::windowChanged,
                         &SignalModel::signalChanged, &SignalModel::domainChanged,
                         &SignalModel::displayModeChanged, &SignalModel::glowChanged,
                         &SignalModel::rowsChanged, &SignalModel::pausedChanged,
                         &SignalModel::statusChanged}) {
            connect(m_model, sig, this, [this] { update(); });
        }
    }
    emit modelChanged();
    update();
}

void WaveTrace::onFrame()
{
    if (!m_model || !isVisible()) {
        return;
    }
    // Live: every tick. Frozen or offline: a slow refresh rather than none, so the
    // trace is not handed back blank by a scene-graph invalidation.
    if (m_model->live() || (++m_idleFrames % 30) == 0) {
        update();
    }
}

trace::Built WaveTrace::buildCurrent()
{
    if (!m_model || !m_model->rowHasData(m_row) || width() < 2.0 || height() < 2.0) {
        return {};
    }

    emg::SampleRing *ring = m_model->displayRing();
    const std::size_t Ws = std::max<std::size_t>(2, m_model->windowSamples());

    // The head is the MODEL's, captured once per frame, so every row is placed
    // against the same instant. Rows reading the ring's write counter themselves were
    // up to ~115 ms apart on a live ECG, which is the one thing it is read for.
    const std::uint64_t head = m_model->displayHead();
    const std::uint64_t trusted = m_model->validFrom();
    const std::size_t available = head > trusted ? std::size_t(head - trusted) : 0;
    const std::size_t want = std::min({Ws, available, ring->capacity()});
    if (want < 2) {
        return {};
    }

    m_buf.resize(want);
    const std::size_t n = ring->readEndingAt(m_row, head, m_buf.data(), want);
    if (n < 2) {
        return {};
    }

    trace::Params p;
    p.width = width();
    p.height = height();
    p.halfRangeUv = m_model->halfRangeUv();
    p.windowSamples = Ws;
    p.sweep = (m_model->displayMode() == SignalModel::Sweep);
    return trace::build(m_buf.data(), n, head, p);
}

QSGNode *WaveTrace::updatePaintNode(QSGNode *old, UpdatePaintNodeData *)
{
    auto *root = static_cast<Root *>(old);
    if (!root) {
        root = new Root;
    }

    trace::Mesh mesh[LayerCount];
    const trace::Built b = buildCurrent();

    if (!b.lines.isEmpty()) {
        const bool sweep = m_model->displayMode() == SignalModel::Sweep;
        const float w = float(width()), h = float(height());

        // ---- sweep: the fading band ahead of the head ---------------------------------
        if (sweep && b.gapWidth > 0.0) {
            const float x0 = float(b.headX), gap = float(b.gapWidth);
            if (x0 + gap <= w) {
                trace::fadeBand(mesh[Band], x0, x0 + gap, h, rgba(m_glow), kSweepBandAlpha);
            } else {
                // the gap wraps past the right edge
                const float first = w - x0;
                trace::fadeBand(mesh[Band], x0, w, h, rgba(m_glow), kSweepBandAlpha);
                trace::fadeBand(mesh[Band], 0, gap - first, h, rgba(m_glow),
                     kSweepBandAlpha * (1.0f - first / gap));
            }
        }

        // ---- glow: one wide, soft, translucent stroke. Qt Quick Effects (a real blur)
        // is Qt 6.5+; the spec's own alternative is a wider low-alpha stroke under the
        // real one. No end extension: overlap would double the alpha at every joint.
        if (m_model->glow()) {
            // A halo this soft (3.6 px skirt) cannot show per-pixel detail, so it is
            // built from every kGlowStride-th vertex: a third of the geometry for no
            // visible difference. The first and last are always kept.
            for (const QPolygonF &line : b.lines) {
                QPolygonF coarse;
                coarse.reserve(line.size() / kGlowStride + 2);
                for (int i = 0; i < line.size(); i += kGlowStride) {
                    coarse << line[i];
                }
                if (line.size() > 1 && ((line.size() - 1) % kGlowStride) != 0) {
                    coarse << line.last();
                }
                trace::stroke(coarse, kGlowCore, kGlowFeather, 0.0f, rgba(m_glow), mesh[Glow]);
            }
            // the whole layer's alpha is scaled once, below
        }

        // ---- the line: 1.4 px, square-capped so joins have no gap ---------------------------
        for (const QPolygonF &line : b.lines) {
            trace::stroke(line, kTraceHalfWidth, kFeather, kTraceHalfWidth, rgba(m_trace), mesh[Main]);
        }

        // ---- head: 2.6 px dot with a 10 px glow --------------------------------------------
        if (b.haveHead) {
            trace::disc(mesh[HeadGlow], b.head, 0.0f, kHeadGlowRadius, rgba(m_glow), 0.55f);
            trace::disc(mesh[HeadDot], b.head, kHeadRadius, kHeadRadius + kFeather, rgba(m_head), 1.0f);
        }
    }

    // The glow stroke is built at full alpha and scaled here, so the same colour can
    // be reused without baking an alpha into the stroke() call.
    for (trace::Vertex &v : mesh[Glow].vertices) {
        v.r = std::uint8_t(v.r * kGlowAlpha);
        v.g = std::uint8_t(v.g * kGlowAlpha);
        v.b = std::uint8_t(v.b * kGlowAlpha);
        v.a = std::uint8_t(v.a * kGlowAlpha);
    }

    for (int i = 0; i < LayerCount; ++i) {
        root->upload(i, mesh[i]);
    }
    return root;
}
