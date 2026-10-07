// A keyboard-shortcut chip: 10 px / 500, 1 px neutral-700 outline, radius 4
// (spec section 4).

import QtQuick
import EmgViewer

Rectangle {
    id: root

    property alias text: label.text

    implicitWidth: Math.max(20, label.implicitWidth + 12)
    implicitHeight: 18
    radius: Theme.radiusSm
    color: "transparent"
    border.width: 1
    border.color: Theme.neutral700

    Text {
        id: label
        anchors.centerIn: parent
        font.family: Theme.fontFamily
        font.pixelSize: Theme.fontKicker
        font.weight: Theme.weightMedium
        color: Theme.neutral200
    }
}
