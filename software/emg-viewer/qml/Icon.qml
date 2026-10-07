// One Phosphor glyph. Sized and coloured like text, because it IS text: the
// bundled Phosphor font carries every icon the spec lists (section 1.5).

import QtQuick
import EmgViewer

Text {
    id: root

    property string name: ""
    property real size: 16

    width: size
    height: size
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter

    font.family: Theme.iconFamily
    font.pixelSize: size
    color: Theme.neutral400

    text: Theme.icons[name] !== undefined ? String.fromCodePoint(Theme.icons[name]) : ""

    // Icons are decoration next to a text label or are labelled by their button.
    Accessible.ignored: true
}
