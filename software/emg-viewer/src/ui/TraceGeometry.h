#pragma once

// The geometry of a waveform trace, with no scene graph and no painter in sight.
//
// Two jobs, both pure functions so they can be tested exactly:
//
//   build()   samples -> polylines in plot pixels. This is where the scale lives:
//             which pixel a microvolt and a sample index land on. The spec's
//             "exact" amplitude and time axis are properties of THIS function, so
//             they are tested here, on numbers, instead of by hunting for coloured
//             pixels in an image.
//
//   stroke()  a polyline -> triangles with a feathered edge. Anti-aliasing without
//             a shader (this Qt build has none): the core of the line is opaque,
//             and a one-pixel skirt fades to transparent through per-vertex alpha.
//
// Why not QPainter: measured on this machine, QPainter needed 77 ms to stroke one
// 1480-vertex row with its glow (475 ms at 5000 vertices) against a 16 ms frame
// budget. A trace is a few thousand vertices; the GPU wants them as triangles.

#include <QPointF>
#include <QPolygonF>
#include <QVector>

#include <cstddef>
#include <cstdint>

namespace trace {

struct Params {
    double width = 0.0;           ///< plot width, pixels
    double height = 0.0;          ///< plot height, pixels
    double halfRangeUv = 1600.0;  ///< microvolts at the top and bottom edge
    std::size_t windowSamples = 0;///< samples the window spans
    bool sweep = false;           ///< Sweep mode, else Scroll
    double gapFraction = 0.035;   ///< sweep erase gap, as a fraction of the width
};

struct Built {
    QVector<QPolygonF> lines;     ///< polylines, y down, in plot pixels
    QPointF head;                 ///< where the newest sample landed
    bool haveHead = false;
    double headX = 0.0;           ///< sweep: x of the write head (the gap starts here)
    double gapWidth = 0.0;        ///< sweep: width of the erase gap, pixels
};

/// Place @p n samples - the last of which is sample index head-1 - on the plot.
///
///   Scroll  the newest sample is at x == width and the window spans the full
///           width. With fewer samples than a window the trace is anchored to the
///           right and occupies only its share: it is never stretched, which would
///           put it at a different time scale from the grid.
///   Sweep   x = (index mod window) / window * width, broken where it wraps and
///           around the erase gap ahead of the head.
///
/// Vertical: v microvolts is at y = height/2 - v * (height/2) / halfRangeUv. No
/// fill factor and no halving - a stated scale has to be exact.
///
/// When samples outnumber twice the width they are reduced to a min/max pair per
/// pixel column, alternating direction, so a one-sample EMG spike survives. Stride
/// sampling would drop exactly those.
Built build(const float *samples, std::size_t n, std::uint64_t head, const Params &p);

/// x position of sample @p index on a plot @p width pixels wide - the single
/// definition used both to place the trace and to place markers on it, so a marker
/// can never drift from the sample it annotates.
double sampleX(std::uint64_t index, std::uint64_t head, std::size_t windowSamples, double width,
               bool sweep);

/// One vertex of a stroke: position, and premultiplied RGBA.
struct Vertex {
    float x, y;
    std::uint8_t r, g, b, a;
};

struct Mesh {
    QVector<Vertex> vertices;
    QVector<std::uint32_t> indices;
    void clear()
    {
        vertices.clear();
        indices.clear();
    }
};

/// Append a feathered stroke of @p line to @p out.
///
///   halfWidth  half the visible line width, pixels. The opaque core.
///   feather    width of the fade-out skirt each side, pixels.
///   extend     lengthen each segment at both ends by this much. A square cap that
///              closes the gaps at joins; leave 0 for a translucent glow, where the
///              overlap it creates would double the alpha at every joint.
///   rgba       straight (non-premultiplied) colour; alpha scales the whole stroke.
///
/// Each segment is three quads across - skirt, core, skirt - sharing vertices.
void stroke(const QPolygonF &line, float halfWidth, float feather, float extend,
            std::uint32_t rgba, Mesh &out);

/// A disc, opaque out to @p core and fading to nothing at @p outer. @p peakAlpha
/// scales the whole disc (a glow is a disc with core 0).
void disc(Mesh &m, QPointF centre, float core, float outer, std::uint32_t rgba, float peakAlpha);

/// A rectangle from x0 to x1, full height, fading from @p alphaLeft on the left edge
/// to nothing on the right.
void fadeBand(Mesh &m, float x0, float x1, float height, std::uint32_t rgba, float alphaLeft);

/// The area between @p curve and the horizontal line y = @p baseline, filled with a
/// vertical gradient: alpha @p topAlpha at y = @p gradTop, falling linearly to 0 at
/// y = @p baseline. This is the spectrum's "accent 32 % -> 0 %" area fill; because
/// alpha is linear in y, interpolating it between a vertex on the curve and one on
/// the baseline reproduces a true gradient exactly.
void fillBelow(const QPolygonF &curve, float baseline, float gradTop, std::uint32_t rgba,
               float topAlpha, Mesh &out);

/// One solid rectangle.
void rect(Mesh &m, float x0, float y0, float x1, float y1, std::uint32_t rgba, float alpha);

/// Premultiply a straight RGBA colour, as QSGVertexColorMaterial expects.
Vertex premultiplied(float x, float y, std::uint32_t rgba, float alphaScale);

} // namespace trace
