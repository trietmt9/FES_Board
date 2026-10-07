#include "TraceGeometry.h"

#include <algorithm>
#include <cmath>

namespace trace {

// ========================================================================= build

double sampleX(std::uint64_t index, std::uint64_t head, std::size_t windowSamples, double width,
               bool sweep)
{
    const std::size_t Ws = std::max<std::size_t>(2, windowSamples);
    if (sweep) {
        // x = (t mod W) / W
        return double(index % Ws) / double(Ws) * width;
    }
    // Scroll: the newest sample (head - 1) is at x == width and the window spans the
    // full width.
    return width - double(head - 1 - index) * (width / double(Ws - 1));
}

Built build(const float *samples, std::size_t n, std::uint64_t head, const Params &p)
{
    Built out;
    if (!samples || n == 0 || p.width < 2.0 || p.height < 2.0 || head < n) {
        return out;
    }

    const std::size_t Ws = std::max<std::size_t>(2, p.windowSamples);
    const double w = p.width;
    const double h = p.height;
    const double mid = h / 2.0;
    const double pxPerUv = mid / p.halfRangeUv;    // no fill factor: a stated scale must be exact
    const std::uint64_t first = head - n;

    auto yOf = [&](float v) { return mid - double(v) * pxPerUv; };

    out.headX = p.sweep ? double(head % Ws) / double(Ws) * w : w;
    out.gapWidth = p.sweep ? p.gapFraction * w : 0.0;

    auto inGap = [&](double x) {
        double d = x - out.headX;
        if (d < 0.0) {
            d += w;
        }
        return d < out.gapWidth;
    };
    auto xOf = [&](std::uint64_t g) { return sampleX(g, head, Ws, w, p.sweep); };

    QPolygonF cur;
    auto brk = [&] {
        if (!cur.isEmpty()) {
            out.lines << cur;
            cur.clear();
        }
    };

    // "samples > 2 x pixel width" is the spec's threshold for min/max reduction.
    const bool decimate = double(n) > 2.0 * w;

    if (!decimate) {
        double prevX = -1.0;
        for (std::size_t j = 0; j < n; ++j) {
            const std::uint64_t g = first + j;
            const double x = xOf(g);
            if (p.sweep && inGap(x)) {
                brk();
                prevX = -1.0;
                continue;
            }
            if (p.sweep && x < prevX) {
                brk();   // wrapped from the right edge to the left
            }
            const QPointF pt(x, yOf(samples[j]));
            cur << pt;
            prevX = x;
            out.head = pt;
            out.haveHead = true;
        }
    } else {
        // Min/max per pixel column. Emission alternates min->max and max->min so the
        // path zigzags through each column rather than jumping back across it.
        int col = -1;
        float mn = 0.0f, mx = 0.0f, lastV = 0.0f;
        bool flip = false;
        const int maxCol = std::max(0, int(w) - 1);

        auto flush = [&] {
            if (col < 0) {
                return;
            }
            const double cx = col + 0.5;
            const double yLo = yOf(mn), yHi = yOf(mx);   // yHi < yLo: a larger value is higher up
            if (flip) {
                cur << QPointF(cx, yHi) << QPointF(cx, yLo);
            } else {
                cur << QPointF(cx, yLo) << QPointF(cx, yHi);
            }
            flip = !flip;
            out.head = QPointF(cx, yOf(lastV));
            out.haveHead = true;
        };

        for (std::size_t j = 0; j < n; ++j) {
            const std::uint64_t g = first + j;
            const double x = xOf(g);
            if (p.sweep && inGap(x)) {
                flush();
                brk();
                col = -1;
                continue;
            }
            const int c = std::clamp(int(x), 0, maxCol);
            const float v = samples[j];
            if (c != col) {
                const bool wrapped = (c < col);
                flush();
                if (wrapped) {
                    brk();
                }
                col = c;
                mn = mx = v;
            } else {
                mn = std::min(mn, v);
                mx = std::max(mx, v);
            }
            lastV = v;
        }
        flush();
    }
    brk();

    // In scroll mode the head dot belongs on the right edge at the newest sample's
    // height, however the last column was reduced.
    if (!p.sweep && out.haveHead) {
        out.head.setX(w);
        out.head.setY(yOf(samples[n - 1]));
    }
    return out;
}

// ======================================================================== stroke

Vertex premultiplied(float x, float y, std::uint32_t rgba, float alphaScale)
{
    const float a = float((rgba >> 24) & 0xFFu) / 255.0f * alphaScale;
    auto ch = [&](int shift) {
        const float c = float((rgba >> shift) & 0xFFu) / 255.0f * a;
        return std::uint8_t(std::clamp(int(std::lround(c * 255.0f)), 0, 255));
    };
    return {x, y, ch(16), ch(8), ch(0),
            std::uint8_t(std::clamp(int(std::lround(a * 255.0f)), 0, 255))};
}

void stroke(const QPolygonF &line, float halfWidth, float feather, float extend,
            std::uint32_t rgba, Mesh &out)
{
    const int segMax = std::max(0, int(line.size()) - 1);
    if (segMax == 0) {
        return;
    }

    // Cross-section, outer skirt to outer skirt: transparent, opaque, opaque,
    // transparent. The two opaque vertices are the visible line; the skirts are the
    // anti-aliasing. The colours are the same for every vertex of a kind, so they are
    // worked out once - doing it per vertex was most of the cost of this function.
    const float off[4] = {-(halfWidth + feather), -halfWidth, halfWidth, halfWidth + feather};
    const Vertex tmpl[4] = {premultiplied(0, 0, rgba, 0.0f), premultiplied(0, 0, rgba, 1.0f),
                            premultiplied(0, 0, rgba, 1.0f), premultiplied(0, 0, rgba, 0.0f)};

    // Size once, write through pointers, then trim to what was actually written
    // (degenerate segments are skipped).
    const int v0 = int(out.vertices.size());
    const int i0 = int(out.indices.size());
    out.vertices.resize(v0 + segMax * 8);
    out.indices.resize(i0 + segMax * 18);
    Vertex *vp = out.vertices.data() + v0;
    std::uint32_t *ip = out.indices.data() + i0;
    std::uint32_t base = std::uint32_t(v0);
    int written = 0;

    const QPointF *pts = line.constData();
    for (int i = 0; i < segMax; ++i) {
        double ax = pts[i].x(), ay = pts[i].y();
        double bx = pts[i + 1].x(), by = pts[i + 1].y();
        double dx = bx - ax, dy = by - ay;
        const double len = std::hypot(dx, dy);
        if (!(len > 1e-6)) {
            continue;   // no direction, no normal: a NaN here would draw garbage
        }
        const double inv = 1.0 / len;
        dx *= inv;
        dy *= inv;
        const double nx = -dy, ny = dx;

        ax -= dx * extend;
        ay -= dy * extend;
        bx += dx * extend;
        by += dy * extend;

        for (int k = 0; k < 4; ++k) {
            Vertex v = tmpl[k];
            v.x = float(ax + nx * off[k]);
            v.y = float(ay + ny * off[k]);
            vp[k] = v;
            v.x = float(bx + nx * off[k]);
            v.y = float(by + ny * off[k]);
            vp[4 + k] = v;
        }
        // Three quads across: [a_k, a_k+1, b_k, b_k+1] for k = 0..2.
        for (std::uint32_t k = 0; k < 3; ++k) {
            const std::uint32_t a0 = base + k, a1 = a0 + 1, b0 = base + 4 + k, b1 = b0 + 1;
            ip[0] = a0; ip[1] = a1; ip[2] = b0;
            ip[3] = a1; ip[4] = b1; ip[5] = b0;
            ip += 6;
        }
        vp += 8;
        base += 8;
        ++written;
    }

    out.vertices.resize(v0 + written * 8);
    out.indices.resize(i0 + written * 18);
}


// ========================================================================= shapes

void disc(Mesh &m, QPointF c, float core, float outer, std::uint32_t rgba, float peakAlpha)
{
    constexpr int kSegments = 20;
    const auto base = std::uint32_t(m.vertices.size());
    m.vertices << premultiplied(float(c.x()), float(c.y()), rgba, peakAlpha);
    for (int ring = 0; ring < 2; ++ring) {
        const float r = ring == 0 ? core : outer;
        const float a = ring == 0 ? peakAlpha : 0.0f;
        for (int i = 0; i < kSegments; ++i) {
            const float t = 6.2831853f * float(i) / float(kSegments);
            m.vertices << premultiplied(float(c.x()) + r * std::cos(t),
                                        float(c.y()) + r * std::sin(t), rgba, a);
        }
    }
    for (int i = 0; i < kSegments; ++i) {
        const std::uint32_t n = std::uint32_t((i + 1) % kSegments);
        const std::uint32_t c0 = base + 1 + std::uint32_t(i), c1 = base + 1 + n;
        const std::uint32_t o0 = base + 1 + kSegments + std::uint32_t(i), o1 = base + 1 + kSegments + n;
        m.indices << base << c0 << c1;                       // centre -> core ring
        m.indices << c0 << o0 << c1 << c1 << o0 << o1;       // core ring -> outer ring
    }
}

void fadeBand(Mesh &m, float x0, float x1, float h, std::uint32_t rgba, float a0)
{
    const auto b = std::uint32_t(m.vertices.size());
    m.vertices << premultiplied(x0, 0, rgba, a0) << premultiplied(x1, 0, rgba, 0.0f)
               << premultiplied(x0, h, rgba, a0) << premultiplied(x1, h, rgba, 0.0f);
    m.indices << b << b + 1 << b + 2 << b + 1 << b + 3 << b + 2;
}

void fillBelow(const QPolygonF &curve, float baseline, float gradTop, std::uint32_t rgba,
               float topAlpha, Mesh &out)
{
    if (curve.size() < 2) {
        return;
    }
    const float span = std::max(1e-3f, baseline - gradTop);
    const auto base = std::uint32_t(out.vertices.size());
    out.vertices.reserve(out.vertices.size() + curve.size() * 2);
    for (const QPointF &p : curve) {
        // alpha is linear in y: topAlpha at gradTop, 0 at the baseline
        const float f = std::clamp((float(p.y()) - gradTop) / span, 0.0f, 1.0f);
        out.vertices << premultiplied(float(p.x()), float(p.y()), rgba, topAlpha * (1.0f - f));
        out.vertices << premultiplied(float(p.x()), baseline, rgba, 0.0f);
    }
    for (int i = 0; i + 1 < curve.size(); ++i) {
        const std::uint32_t t0 = base + std::uint32_t(2 * i), b0 = t0 + 1;
        const std::uint32_t t1 = t0 + 2, b1 = t0 + 3;
        out.indices << t0 << b0 << t1 << t1 << b0 << b1;
    }
}

void rect(Mesh &m, float x0, float y0, float x1, float y1, std::uint32_t rgba, float alpha)
{
    const auto b = std::uint32_t(m.vertices.size());
    m.vertices << premultiplied(x0, y0, rgba, alpha) << premultiplied(x1, y0, rgba, alpha)
               << premultiplied(x0, y1, rgba, alpha) << premultiplied(x1, y1, rgba, alpha);
    m.indices << b << b + 1 << b + 2 << b + 1 << b + 3 << b + 2;
}

} // namespace trace
