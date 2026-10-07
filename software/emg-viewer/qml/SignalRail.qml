// Spec section 4: the signal rail.
//
// Three exclusive cards - ECG, EEG, EMG - and the shortcuts box at the bottom.
// Only one signal is acquired at a time (the signal type also selects the filter
// band), so only the selected card carries a live value; the others read "--"
// rather than a number computed under a filter that is not running.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import EmgViewer

Item {
    id: root

    readonly property var cards: [
        { id: 0, title: "ECG", icon: "heartbeat" },
        { id: 1, title: "EEG", icon: "brain" },
        { id: 2, title: "EMG", icon: "hand-fist" }
    ]

    ColumnLayout {
        anchors.fill: parent
        spacing: Theme.s2

        Kicker {
            text: "Signal type"
            color: Theme.neutral500
            Layout.bottomMargin: Theme.s1
        }

        Repeater {
            model: root.cards

            delegate: AbstractButton {
                id: card

                required property var modelData
                readonly property bool selected: SignalModel.signal === modelData.id

                Layout.fillWidth: true
                Layout.preferredHeight: 52
                // padding 10 x 8: vertical 10, horizontal 8. The control places its
                // own contentItem from these, so the content must not anchor itself.
                topPadding: 10
                bottomPadding: 10
                leftPadding: 8
                rightPadding: 8
                hoverEnabled: true
                focusPolicy: Qt.TabFocus

                Accessible.role: Accessible.RadioButton
                Accessible.name: modelData.title + ", " + SignalModel.railMeta[modelData.id]
                Accessible.checked: selected
                Accessible.onPressAction: card.clicked()

                onClicked: SignalModel.selectSignal(modelData.id)

                background: Item {
                    // Soft accent glow behind the selected card.
                    Glow { visible: card.selected; radius: Theme.radiusMd }

                    Rectangle {
                        anchors.fill: parent
                        radius: Theme.radiusMd
                        color: card.selected ? Theme.accent900
                             : card.hovered ? Theme.neutral900 : "transparent"
                        border.width: card.selected ? 1 : 0
                        border.color: Theme.accent700
                    }

                    Rectangle {
                        visible: card.visualFocus
                        anchors.fill: parent
                        anchors.margins: -(Theme.focusOffset + Theme.focusWidth)
                        radius: Theme.radiusMd + Theme.focusOffset + Theme.focusWidth
                        color: "transparent"
                        border.width: Theme.focusWidth
                        border.color: Theme.accent
                    }
                }

                // grid: 32 px icon tile | text | value, padding 10 x 8
                contentItem: RowLayout {
                    spacing: Theme.s3

                    Rectangle {
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32
                        Layout.alignment: Qt.AlignVCenter
                        radius: Theme.radiusSm
                        color: Theme.neutral900

                        Icon {
                            anchors.centerIn: parent
                            name: card.modelData.icon
                            size: 18
                            color: card.selected ? Theme.accent : Theme.neutral400
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        spacing: 1

                        Text {
                            text: card.modelData.title
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontBase
                            font.weight: Theme.weightMedium
                            color: Theme.text
                        }
                        Text {
                            Layout.fillWidth: true
                            text: SignalModel.railMeta[card.modelData.id]
                            elide: Text.ElideRight
                            font.family: Theme.numberFamily
                            font.pixelSize: Theme.fontSmall
                            color: Theme.neutral500
                        }
                    }

                    Text {
                        Layout.alignment: Qt.AlignVCenter
                        text: SignalModel.railValues[card.modelData.id]
                        font.family: Theme.numberFamily
                        font.pixelSize: Theme.fontBase
                        font.weight: Theme.weightMedium
                        color: card.selected ? Theme.accent300 : Theme.neutral500
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }

        // ---- shortcuts box: hairline (sm) elevation -------------------------------
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: shortcuts.implicitHeight + 2 * Theme.s4
            radius: Theme.radiusMd
            color: "transparent"
            border.width: 1
            border.color: Theme.neutral800

            ColumnLayout {
                id: shortcuts
                anchors.fill: parent
                anchors.margins: Theme.s4
                spacing: Theme.s2

                Repeater {
                    model: [
                        { key: "1–3",  label: "Signal type" },
                        { key: "M",        label: "Add marker" },
                        { key: "F",        label: "Time / frequency" },
                        { key: "Space",    label: "Freeze" }
                    ]

                    delegate: RowLayout {
                        required property var modelData
                        Layout.fillWidth: true
                        spacing: Theme.s3

                        KbdChip {
                            Layout.preferredWidth: 44
                            text: parent.modelData.key
                        }
                        Text {
                            Layout.fillWidth: true
                            text: parent.modelData.label
                            font.family: Theme.fontFamily
                            font.pixelSize: 12
                            color: Theme.neutral400
                        }
                    }
                }
            }
        }
    }
}
