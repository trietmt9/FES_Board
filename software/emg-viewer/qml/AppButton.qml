// The spec's three button styles (section 1.4), one component.
//
//   primary    outline in the accent colour           (Record, Connect)
//   secondary  1 px divider outline                   (Wi-Fi, Marker, Disconnect)
//   ghost      no outline                             (the dialog's x and Rescan)
//
// States: hover 7 % text tint (12 % accent tint for primary), pressed 14 % / 22 %,
// keyboard focus a 2 px accent ring 2 px outside, disabled 45 % opacity.
//
// Built on AbstractButton with a hand-made background and content, so nothing
// here depends on the Qt Quick Controls style or its palette - the Basic style
// otherwise draws its own near-black text on this dark surface.

import QtQuick
import QtQuick.Controls
import EmgViewer

AbstractButton {
    id: root

    property string variant: "secondary"      // primary | secondary | ghost
    property string iconName: ""
    property real iconSize: 14
    property string trailingIcon: ""          // e.g. the Wi-Fi button's caret
    property color contentColor: variant === "primary" ? Theme.accent : Theme.text
    property color iconColor: contentColor    // the leading icon can differ from the label
    // A pulsing dot ahead of the label (the Record button while recording): 7 px,
    // accent-300, opacity 1 -> 0.25 -> 1 over 1.2 s (spec section 3).
    property bool pulseDot: false
    property int labelMaxWidth: 0             // > 0: elide the label past this width
    property string accessibleName: ""
    readonly property bool iconOnly: text === ""
    readonly property bool isPrimary: variant === "primary"

    implicitHeight: 32
    implicitWidth: iconOnly ? implicitHeight : row.implicitWidth + 2 * 12
    padding: 0

    hoverEnabled: true
    // Tab reaches it, but a mouse click does not leave a focus ring behind.
    focusPolicy: Qt.TabFocus
    opacity: enabled ? 1.0 : Theme.disabledOpacity

    Accessible.role: Accessible.Button
    Accessible.name: accessibleName !== "" ? accessibleName : text
    Accessible.onPressAction: root.clicked()

    background: Rectangle {
        radius: Theme.radiusMd
        color: root.down ? (root.isPrimary ? Theme.accentPressedFill : Theme.pressedFill)
             : root.hovered ? (root.isPrimary ? Theme.accentHoverFill : Theme.hoverFill)
             : "transparent"
        border.width: root.variant === "ghost" ? 0 : 1
        border.color: root.isPrimary ? Theme.accent : Theme.divider

        Behavior on color { ColorAnimation { duration: 90 } }

        // Keyboard focus ring: 2 px, offset 2 px.
        Rectangle {
            visible: root.visualFocus
            anchors.fill: parent
            anchors.margins: -(Theme.focusOffset + Theme.focusWidth)
            radius: parent.radius + Theme.focusOffset + Theme.focusWidth
            color: "transparent"
            border.width: Theme.focusWidth
            border.color: Theme.accent
        }
    }

    contentItem: Item {
        Row {
            id: row
            anchors.centerIn: parent
            spacing: 6

            Rectangle {
                id: dot
                visible: root.pulseDot
                anchors.verticalCenter: parent.verticalCenter
                width: 7
                height: 7
                radius: 3.5
                color: Theme.accent300

                SequentialAnimation on opacity {
                    running: root.pulseDot
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.25; duration: 600; easing.type: Easing.InOutSine }
                    NumberAnimation { to: 1.0;  duration: 600; easing.type: Easing.InOutSine }
                }
            }
            Icon {
                visible: root.iconName !== ""
                anchors.verticalCenter: parent.verticalCenter
                name: root.iconName
                size: root.iconSize
                color: root.iconColor
            }
            Text {
                visible: !root.iconOnly
                anchors.verticalCenter: parent.verticalCenter
                text: root.text
                width: root.labelMaxWidth > 0 ? Math.min(implicitWidth, root.labelMaxWidth) : implicitWidth
                elide: Text.ElideRight
                font.family: Theme.fontFamily
                font.pixelSize: 12
                font.weight: Theme.weightMedium
                color: root.contentColor
            }
            Icon {
                visible: root.trailingIcon !== ""
                anchors.verticalCenter: parent.verticalCenter
                name: root.trailingIcon
                size: 11
                color: Theme.neutral400
            }
        }
    }
}
