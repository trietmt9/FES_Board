// A soft drop shadow, built from stacked translucent rounded rectangles.
//
// The spec's elevations are CSS box-shadows (md: 0 6 18 rgba(0,0,0,.55); lg:
// 0 16 40 rgba(0,0,0,.65)). A real blur needs Qt Quick Effects (Qt 6.5+), which
// this Qt 6.4 build does not have, so this approximates one: N concentric layers
// whose combined alpha at the centre equals `strength` and falls off to the edge.
// Against the near-black background the result is indistinguishable at the
// sizes involved.

import QtQuick
import EmgViewer

Item {
    id: root

    property real radius: Theme.radiusMd
    property real offsetY: 6
    property real blur: 18
    property color shadowColor: "black"
    property real strength: 0.55
    property int layers: 12

    // Sits behind its parent's content and is as large as the parent.
    anchors.fill: parent
    z: -1

    Repeater {
        model: root.layers

        Rectangle {
            // Layer 0 is the widest and faintest; the last is tight to the card.
            readonly property real grow: root.blur * (1 - index / root.layers)

            x: -grow
            y: -grow + root.offsetY
            width: parent.width + 2 * grow
            height: parent.height + 2 * grow
            radius: root.radius + grow
            color: Theme.withAlpha(root.shadowColor, root.strength / root.layers)
        }
    }
}
