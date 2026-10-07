// Spec section 3: the 52 px header.
//
//   [logo] BioView Monitor   user Patient / Session      [Wi-Fi v] battery  clock  [Record]
//
// Everything on it is real or honestly empty. The mockup's "82 %" battery and its
// patient number are static placeholders (the spec says as much in its clinical
// notes); nothing measures a battery here, so it reads "--", and the patient and
// session text come from settings rather than a made-up record.

import QtQuick
import QtQuick.Layouts
import EmgViewer

Item {
    id: root

    signal openDevices()

    implicitHeight: Theme.headerHeight

    // ---- clock: 1 s tick -----------------------------------------------------
    property string clockText: Qt.formatTime(new Date(), "HH:mm:ss")
    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: root.clockText = Qt.formatTime(new Date(), "HH:mm:ss")
    }

    function mmss(totalSeconds) {
        const m = Math.floor(totalSeconds / 60)
        const s = totalSeconds % 60
        return (m < 10 ? "0" : "") + m + ":" + (s < 10 ? "0" : "") + s
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.bodyPadding
        anchors.rightMargin: Theme.bodyPadding
        spacing: Theme.s5

        // ---- 1. logo ----------------------------------------------------------
        RowLayout {
            spacing: Theme.s3

            Rectangle {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                radius: Theme.radiusSm
                color: "transparent"
                border.width: 1
                border.color: Theme.accent

                Icon {
                    anchors.centerIn: parent
                    name: "pulse"
                    size: 16
                    color: Theme.accent
                }
            }
            Row {
                spacing: 5
                Text {
                    text: "BioView"
                    font.family: Theme.fontFamily
                    font.pixelSize: 14
                    font.weight: Theme.weightMedium
                    color: Theme.text
                }
                Text {
                    text: "Monitor"
                    font.family: Theme.fontFamily
                    font.pixelSize: 14
                    font.weight: Theme.weightRegular
                    color: Theme.neutral600
                }
            }
        }

        // ---- 2. patient context -------------------------------------------------
        RowLayout {
            spacing: 6
            Icon { name: "user"; size: 14; color: Theme.neutral400 }
            Text {
                text: SignalModel.patientLabel
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontBase
                color: Theme.neutral400
            }
            Text {
                text: "/"
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontBase
                color: Theme.neutral700
            }
            Text {
                text: SignalModel.sessionLabel
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontBase
                color: Theme.neutral400
            }
        }

        // ---- 3. spacer ------------------------------------------------------------
        Item { Layout.fillWidth: true }

        // ---- 4. Wi-Fi / device button -----------------------------------------------
        AppButton {
            id: deviceButton
            variant: "secondary"
            iconName: DeviceManager.iconName
            trailingIcon: "caret-down"
            text: DeviceManager.label
            labelMaxWidth: 140
            accessibleName: "Device connection: " + DeviceManager.label
            contentColor: DeviceManager.state === DeviceManager.Connected
                          ? Theme.text : Theme.neutral300
            // Accent when connected, neutral-500 otherwise (spec section 3).
            iconColor: DeviceManager.state === DeviceManager.Connected
                       ? Theme.accent : Theme.neutral500
            onClicked: root.openDevices()
        }

        // ---- 5. battery ----------------------------------------------------------------
        RowLayout {
            spacing: 5
            Icon { name: "battery-high"; size: 16; color: Theme.neutral500 }
            Text {
                // No fuel gauge is wired to the host, so there is no honest number.
                text: "—"
                font.family: Theme.numberFamily
                font.pixelSize: Theme.fontBase
                color: Theme.neutral500
            }
        }

        // ---- 6. clock ---------------------------------------------------------------------
        Text {
            text: root.clockText
            font.family: Theme.numberFamily
            font.pixelSize: Theme.fontBase
            color: Theme.neutral300
        }

        // ---- 7. record ----------------------------------------------------------------------
        AppButton {
            id: recordButton
            variant: "primary"
            Layout.minimumWidth: 118
            enabled: SignalModel.recording || SignalModel.canRecord
            iconName: SignalModel.recording ? "" : "record"
            pulseDot: SignalModel.recording
            text: SignalModel.recording ? ("Stop \u00B7 " + root.mmss(SignalModel.recordSeconds)) : "Record"
            accessibleName: SignalModel.recording ? "Stop recording" : "Start recording"
            onClicked: SignalModel.toggleRecording()
        }
    }

    FadedRule {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
}
