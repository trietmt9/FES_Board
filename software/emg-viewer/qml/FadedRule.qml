// A 1 px horizontal rule that fades out over its last 48 px at both ends
// (spec section 1.3).
//
// It fades to a transparent copy of ITS OWN colour, not to "transparent". Qt
// interpolates straight (non-premultiplied) colour, so fading to transparent
// black passes through a dark grey and leaves a visible smudge at each end.

import QtQuick
import EmgViewer

Rectangle {
    id: root

    property color ruleColor: Theme.divider
    readonly property color clear: Theme.withAlpha(ruleColor, 0)
    readonly property real edge: width > 0 ? Math.min(0.5, Theme.fadeLength / width) : 0.5

    height: 1
    color: "transparent"

    gradient: Gradient {
        orientation: Gradient.Horizontal
        GradientStop { position: 0.0;            color: root.clear }
        GradientStop { position: root.edge;      color: root.ruleColor }
        GradientStop { position: 1.0 - root.edge; color: root.ruleColor }
        GradientStop { position: 1.0;            color: root.clear }
    }
}
