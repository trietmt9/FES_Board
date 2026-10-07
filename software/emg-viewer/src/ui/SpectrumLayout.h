#pragma once

// Where the spectrum's axes sit inside its item (spec section 6.2: "Plot area: 8 px
// top, 18 px bottom for labels"). SpectrumItem paints the grid and labels and
// SpectrumTrace draws the curve; both need to agree on these exactly, or the curve
// floats off its own grid. They are defined once, here.

#include <algorithm>

namespace spectrumlayout {

constexpr double kTopPad = 8.0;
constexpr double kBottomPad = 18.0;

inline double plotHeight(double itemHeight) { return itemHeight - kTopPad - kBottomPad; }
inline double plotTop() { return kTopPad; }
inline double plotBottom(double itemHeight) { return itemHeight - kBottomPad; }

/// y for a dB value, given the axis [bottomDb, topDb]. Clamped to the plot area.
inline double yOfDb(double db, double bottomDb, double topDb, double itemHeight)
{
    const double f = std::clamp((db - bottomDb) / (topDb - bottomDb), 0.0, 1.0);
    return plotBottom(itemHeight) - f * plotHeight(itemHeight);
}

inline double xOfHz(double hz, double maxHz, double itemWidth) { return hz / maxHz * itemWidth; }

} // namespace spectrumlayout
