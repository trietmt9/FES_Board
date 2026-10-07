// A soft accent glow behind a selected card (spec section 4: "soft accent glow
// 0 0 24 -8"). Same technique as Shadow.qml - stacked translucent rounded
// rectangles standing in for a blur this Qt build cannot do - tinted with the
// accent and centred rather than dropped.

import QtQuick
import EmgViewer

Shadow {
    shadowColor: Theme.accent
    offsetY: 0
    blur: 14
    strength: 0.30
    layers: 10
}
