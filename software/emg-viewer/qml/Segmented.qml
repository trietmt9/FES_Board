// The spec's segmented control (sections 5.1, 5.2): 1 px divider border, radius
// 8, options in 12 px text that never wrap or clip, the selected option marked by
// accent text and a 1 px accent inner outline.
//
//   model         [ { text: "2.5 s", value: 2.5, icon: "wave-sine" }, ... ]
//   currentValue  the selected option's value
//   activated(v)  the user picked an option
//
// Options are not checkable buttons. A checkable button breaks its own binding
// the first time it is clicked, after which the control stops following the
// model; here "selected" is only ever a comparison with currentValue.

import QtQuick
import QtQuick.Controls
import EmgViewer

Item {
    id: root

    property var model: []
    property var currentValue
    property int itemHeight: 30
    property int sidePadding: 12
    /// Split the control's width equally between the options, instead of sizing each
    /// to its text (the dialog's USB / Wi-Fi switch spans the full width).
    property bool equalWidths: false

    signal activated(var value)

    implicitHeight: itemHeight
    implicitWidth: row.implicitWidth + 2

    Rectangle {
        anchors.fill: parent
        radius: Theme.radiusMd
        color: "transparent"
        border.width: 1
        border.color: Theme.divider
    }

    Row {
        id: row
        anchors.fill: parent
        anchors.margins: 1

        Repeater {
            model: root.model

            delegate: AbstractButton {
                id: opt

                required property var modelData
                readonly property bool selected: root.currentValue === modelData.value

                height: row.height
                width: root.equalWidths ? row.width / Math.max(1, root.model.length)
                                        : content.implicitWidth + 2 * root.sidePadding
                padding: 0
                hoverEnabled: true
                focusPolicy: Qt.TabFocus

                Accessible.role: Accessible.RadioButton
                Accessible.name: modelData.text
                Accessible.checked: selected
                Accessible.onPressAction: opt.clicked()

                onClicked: root.activated(modelData.value)

                background: Rectangle {
                    radius: Theme.radiusMd - 1
                    color: opt.down ? Theme.pressedFill : opt.hovered ? Theme.hoverFill : "transparent"

                    // Selected: a 1 px accent outline just inside the segment.
                    Rectangle {
                        visible: opt.selected
                        anchors.fill: parent
                        anchors.margins: 2
                        radius: Theme.radiusMd - 3
                        color: "transparent"
                        border.width: 1
                        border.color: Theme.accent
                    }

                    Rectangle {
                        visible: opt.visualFocus
                        anchors.fill: parent
                        anchors.margins: -Theme.focusOffset
                        radius: parent.radius + Theme.focusOffset
                        color: "transparent"
                        border.width: Theme.focusWidth
                        border.color: Theme.accent
                    }
                }

                contentItem: Item {
                    Row {
                        id: content
                        anchors.centerIn: parent
                        spacing: 6

                        Icon {
                            visible: modelData.icon !== undefined
                            anchors.verticalCenter: parent.verticalCenter
                            name: modelData.icon !== undefined ? modelData.icon : ""
                            size: 14
                            color: opt.selected ? Theme.accent : Theme.neutral400
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData.text
                            wrapMode: Text.NoWrap
                            font.family: Theme.numberFamily
                            font.pixelSize: 12
                            font.weight: Theme.weightMedium
                            color: opt.selected ? Theme.accent : Theme.neutral300
                        }
                    }
                }
            }
        }
    }
}
