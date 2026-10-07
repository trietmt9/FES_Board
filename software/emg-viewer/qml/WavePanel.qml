// Spec section 5: the centre panel - toolbar, channel rows, footer.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import EmgViewer

Rectangle {
    id: root

    radius: Theme.radiusLg
    // surface -> 55 % surface / bg, top to bottom
    gradient: Gradient {
        GradientStop { position: 0.0; color: Theme.surface }
        GradientStop { position: 1.0; color: Theme.mix(Theme.bg, Theme.surface, 0.55) }
    }
    // sm elevation: a 1 px neutral-800 border
    border.width: 1
    border.color: Theme.neutral800

    readonly property bool timeDomain: SignalModel.domain === SignalModel.Time

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: Theme.s5
        anchors.bottomMargin: Theme.s3
        spacing: 0

        // ============================ toolbar row 1 ============================
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Theme.s5
            Layout.rightMargin: Theme.s5
            spacing: Theme.s5

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3

                Kicker {
                    Layout.fillWidth: true
                    uppercase: false               // composed in C++ with its own casing
                    text: SignalModel.kicker
                }
                Text {
                    Layout.fillWidth: true
                    text: SignalModel.title
                    elide: Text.ElideRight
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontTitle
                    font.weight: Theme.weightMedium
                    color: Theme.text
                }
            }

            Segmented {
                itemHeight: 34
                currentValue: SignalModel.domain
                model: [
                    { text: "Time domain",      value: SignalModel.Time,      icon: "wave-sine" },
                    { text: "Frequency domain", value: SignalModel.Frequency, icon: "chart-line" }
                ]
                onActivated: (v) => SignalModel.domain = v
            }
        }

        // ============================ toolbar row 2 ============================
        // Tags on the left, controls on the right. At the spec's 1200 px minimum the
        // panel is about 630 px wide and both do not fit on one line, so they stack
        // instead of overlapping (a RowLayout that runs out of width draws its
        // children on top of each other).
        GridLayout {
            id: toolbar2

            readonly property bool stacked:
                tags.implicitWidth + controls.implicitWidth + columnSpacing > width

            Layout.fillWidth: true
            Layout.leftMargin: Theme.s5
            Layout.rightMargin: Theme.s5
            Layout.topMargin: Theme.s4
            Layout.bottomMargin: Theme.s4
            columns: stacked ? 1 : 2
            columnSpacing: Theme.s5
            rowSpacing: Theme.s3

            Row {
                id: tags
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                spacing: Theme.s2

                Repeater {
                    model: SignalModel.filterTags

                    delegate: Tag {
                        id: tag
                        required property string modelData
                        required property int index
                        text: modelData
                        // The notch tag is the one filter setting that varies by
                        // country: click to step 50 Hz -> 60 Hz -> off.
                        interactive: index === 2
                        hovered: interactive && tagMouse.containsMouse

                        MouseArea {
                            id: tagMouse
                            anchors.fill: parent
                            enabled: tag.interactive
                            hoverEnabled: true
                            cursorShape: tag.interactive ? Qt.PointingHandCursor : Qt.ArrowCursor
                            onClicked: SignalModel.cycleNotch()

                            ToolTip.visible: containsMouse && tag.interactive
                            ToolTip.delay: 400
                            ToolTip.text: "Mains notch - click to switch 50 Hz / 60 Hz / off"
                        }
                    }
                }
            }

            Row {
                id: controls
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                spacing: Theme.s4

                // ---- Window (time) / Averaging (frequency) ------------------------
                Row {
                    spacing: Theme.s2
                    anchors.verticalCenter: parent.verticalCenter
                    visible: root.timeDomain

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Window"
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSmall
                        color: Theme.neutral500
                    }
                    Segmented {
                        currentValue: SignalModel.windowSeconds
                        model: [
                            { text: "2.5 s", value: 2.5 },
                            { text: "5 s",   value: 5.0 },
                            { text: "10 s",  value: 10.0 }
                        ]
                        onActivated: (v) => SignalModel.windowSeconds = v
                    }
                }

                Row {
                    spacing: Theme.s2
                    anchors.verticalCenter: parent.verticalCenter
                    visible: !root.timeDomain

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Averaging"
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSmall
                        color: Theme.neutral500
                    }
                    Segmented {
                        currentValue: SignalModel.averaging
                        model: [
                            { text: "Low", value: SignalModel.Low },
                            { text: "Med", value: SignalModel.Medium },
                            { text: "High", value: SignalModel.High }
                        ]
                        onActivated: (v) => SignalModel.averaging = v
                    }
                }

                // ---- Gain ------------------------------------------------------------
                Row {
                    spacing: Theme.s2
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Gain"
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSmall
                        color: Theme.neutral500
                    }
                    Segmented {
                        currentValue: SignalModel.gain
                        model: [
                            { text: "×0.5", value: 0.5 },
                            { text: "×1",   value: 1.0 },
                            { text: "×2",   value: 2.0 }
                        ]
                        onActivated: (v) => SignalModel.gain = v
                    }
                }

                // ---- Marker, Freeze ----------------------------------------------------
                AppButton {
                    anchors.verticalCenter: parent.verticalCenter
                    implicitHeight: 30
                    variant: "secondary"
                    iconName: "flag"
                    text: "Marker"
                    enabled: SignalModel.online
                    onClicked: SignalModel.addMarker()
                }
                AppButton {
                    anchors.verticalCenter: parent.verticalCenter
                    implicitHeight: 30
                    implicitWidth: 30
                    variant: "secondary"
                    iconName: SignalModel.paused ? "play" : "pause"
                    accessibleName: SignalModel.paused ? "Resume display" : "Freeze display"
                    onClicked: SignalModel.togglePause()
                }
            }
        }

        FadedRule { Layout.fillWidth: true }

        // ============================ channel rows ============================
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Repeater {
                model: SignalModel.rows

                delegate: ChannelRow {
                    required property int index
                    required property var modelData

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 80
                    rowIndex: index
                    rowData: modelData
                }
            }
        }

        // ============================ footer ============================
        // 96 px | plot, 11 px neutral-500
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Theme.s5
            Layout.rightMargin: Theme.s5
            Layout.topMargin: Theme.s2
            spacing: 0

            Text {
                Layout.preferredWidth: 96
                text: SignalModel.footerLeft
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSmall
                color: Theme.neutral500
            }

            // Time mode: tick labels spread evenly across the plot.
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: tickRow.implicitHeight
                visible: root.timeDomain

                Row {
                    id: tickRow
                    anchors.fill: parent
                    readonly property real labelsWidth: {
                        let w = 0
                        for (let i = 0; i < children.length; ++i) {
                            w += children[i].implicitWidth
                        }
                        return w
                    }
                    // space-between: first label flush left, last flush right.
                    spacing: Math.max(0, (width - labelsWidth) / Math.max(1, SignalModel.timeTicks.length - 1))

                    Repeater {
                        model: SignalModel.timeTicks
                        delegate: Text {
                            required property string modelData
                            text: modelData
                            font.family: Theme.numberFamily
                            font.pixelSize: Theme.fontSmall
                            color: Theme.neutral500
                        }
                    }
                }
            }

            // Frequency mode: the FFT description, right-aligned.
            Text {
                Layout.fillWidth: true
                visible: !root.timeDomain
                horizontalAlignment: Text.AlignRight
                text: SignalModel.footerRight
                font.family: Theme.numberFamily
                font.pixelSize: Theme.fontSmall
                color: Theme.neutral500
            }
        }
    }
}
