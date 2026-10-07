// The dialog's text input: bg fill, radius 8, 1 px divider border that becomes a 2 px
// accent border on focus (spec sections 1.4 and 8). One component so the passkey, the
// Wi-Fi address and the port cannot drift apart.
//
// Built on TextField with an explicit background and colours: the Basic style
// otherwise draws near-black text from the application palette on this dark surface.

import QtQuick
import QtQuick.Controls
import EmgViewer

TextField {
    id: root

    property string accessibleLabel: ""

    implicitHeight: 36
    leftPadding: Theme.s4
    rightPadding: Theme.s4
    font.family: Theme.fontFamily
    font.pixelSize: Theme.fontBase
    color: Theme.text
    placeholderTextColor: Theme.neutral600
    selectionColor: Theme.accent700
    selectedTextColor: Theme.text
    selectByMouse: true

    Accessible.name: accessibleLabel

    background: Rectangle {
        radius: Theme.radiusMd
        color: Theme.bg
        border.width: root.activeFocus ? 2 : 1
        border.color: root.activeFocus ? Theme.accent : Theme.divider
    }
}
