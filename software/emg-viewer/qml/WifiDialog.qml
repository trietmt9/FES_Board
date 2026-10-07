// Spec section 8: the device-connection dialog, with two tabs.
//
//   USB     the spec's list, over the serial ports that really exist.
//   Wi-Fi   WIFI_DESIGN.md section 4: the amplifier is found by the address the user
//           types (automatic discovery comes later). The PC and the amplifier are both
//           stations on the lab network; the amplifier listens, this PC connects.
//
// Modal, centred, 420 px wide, radius 14, padding 22. States Off -> Connecting ->
// Connected; on success it closes itself after 0.6 s. While not connected the rest of
// the window shows "Device offline".
//
// The Wi-Fi tab is the interface before the link: it validates and remembers the
// address, and Connect says plainly that the link is not built yet. Nothing here
// invents a device; `--preview-wifi <phase>` shows the other states for layout review.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import EmgViewer

Popup {
    id: root

    modal: true
    dim: true
    focus: true
    anchors.centerIn: Overlay.overlay
    width: 420
    padding: Theme.s6
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    // Backdrop: neutral-900 at 50 %. The spec also asks for a 2 px blur, which
    // needs Qt Quick Effects (6.5+); dimming alone carries the modal cue.
    Overlay.modal: Rectangle { color: Theme.withAlpha(Theme.neutral900, 0.5) }

    onOpened: {
        DeviceManager.watching = true
        DeviceManager.scan()
    }
    onClosed: DeviceManager.watching = false

    // On success, close after 0.6 s.
    Connections {
        target: DeviceManager
        function onStateChanged() {
            if (DeviceManager.state === DeviceManager.Connected && root.opened) {
                closeTimer.restart()
            }
        }
    }
    Timer { id: closeTimer; interval: 600; onTriggered: root.close() }

    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 140 }
        NumberAnimation { property: "scale"; from: 0.97; to: 1; duration: 140; easing.type: Easing.OutCubic }
    }
    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 100 }
    }

    background: Item {
        // lg elevation: 1 px neutral-500 + shadow 0 16 40 rgba(0,0,0,.65)
        Shadow { radius: Theme.radiusLg; offsetY: 16; blur: 40; strength: 0.65 }
        Rectangle {
            anchors.fill: parent
            radius: Theme.radiusLg
            color: Theme.surface
            border.width: 1
            border.color: Theme.neutral500
        }
    }

    contentItem: ColumnLayout {
        spacing: Theme.s4

        // ---- header ---------------------------------------------------------------
        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.s3

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3
                Kicker { text: "Device connection" }
                Text {
                    Layout.fillWidth: true
                    text: DeviceManager.dialogTitle
                    elide: Text.ElideRight
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontDialogTitle
                    font.weight: Theme.weightMedium
                    color: Theme.text
                }
            }
            AppButton {
                Layout.alignment: Qt.AlignTop
                variant: "ghost"
                iconName: "x"
                implicitWidth: 30
                implicitHeight: 30
                accessibleName: "Close"
                onClicked: root.close()
            }
        }

        // ---- transport tabs -----------------------------------------------------------
        Segmented {
            Layout.fillWidth: true
            itemHeight: 32
            equalWidths: true
            model: [ { text: "USB", value: 0 }, { text: "Wi-Fi", value: 1 } ]
            currentValue: DeviceManager.transport
            onActivated: (v) => DeviceManager.transport = v
        }

        ColumnLayout {
            id: usbView
            Layout.fillWidth: true
            visible: DeviceManager.transport === 0
            spacing: Theme.s4

            // ---- status row ---------------------------------------------------------------
            RowLayout {
                Layout.fillWidth: true

                Text {
                    Layout.fillWidth: true
                    text: DeviceManager.statusLine
                    elide: Text.ElideRight
                    font.family: Theme.fontFamily
                    font.pixelSize: 12
                    color: Theme.neutral400
                }
                AppButton {
                    variant: "ghost"
                    implicitHeight: 28
                    iconName: "arrow-clockwise"
                    text: "Rescan"
                    enabled: !DeviceManager.scanning
                    onClicked: DeviceManager.scan()
                }
            }

            // ---- network list (single select) -----------------------------------------------
            ListView {
                id: list
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(contentHeight, 5 * 58)
                visible: count > 0
                clip: true
                spacing: Theme.s2
                model: DeviceManager.devices
                currentIndex: DeviceManager.selectedIndex
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                delegate: AbstractButton {
                    id: rowItem

                    required property int index
                    required property string name
                    required property string meta
                    required property string status
                    required property string iconName
                    readonly property bool selected: DeviceManager.selectedIndex === index

                    width: ListView.view.width
                    height: 52
                    topPadding: 10
                    bottomPadding: 10
                    leftPadding: Theme.s4
                    rightPadding: Theme.s4
                    hoverEnabled: true
                    focusPolicy: Qt.TabFocus

                    Accessible.role: Accessible.RadioButton
                    Accessible.name: name + ", " + meta + (status !== "" ? ", " + status : "")
                    Accessible.checked: selected
                    onClicked: DeviceManager.selectedIndex = index

                    // Same look as the selected signal card (spec section 8).
                    background: Item {
                        Glow { visible: rowItem.selected; radius: Theme.radiusMd }
                        Rectangle {
                            anchors.fill: parent
                            radius: Theme.radiusMd
                            color: rowItem.selected ? Theme.accent900
                                 : rowItem.hovered ? Theme.neutral900 : "transparent"
                            border.width: rowItem.selected ? 1 : 0
                            border.color: Theme.accent700
                        }
                        Rectangle {
                            visible: rowItem.visualFocus
                            anchors.fill: parent
                            anchors.margins: -(Theme.focusOffset + Theme.focusWidth)
                            radius: Theme.radiusMd + Theme.focusOffset + Theme.focusWidth
                            color: "transparent"
                            border.width: Theme.focusWidth
                            border.color: Theme.accent
                        }
                    }

                    contentItem: RowLayout {
                        spacing: Theme.s4

                        Icon {
                            name: rowItem.iconName
                            size: 20
                            color: rowItem.selected ? Theme.accent : Theme.neutral500
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 1
                            Text {
                                Layout.fillWidth: true
                                text: rowItem.name
                                elide: Text.ElideRight
                                font.family: Theme.fontFamily
                                font.pixelSize: Theme.fontBase
                                font.weight: Theme.weightMedium
                                color: Theme.text
                            }
                            Text {
                                Layout.fillWidth: true
                                text: rowItem.meta
                                elide: Text.ElideRight
                                font.family: Theme.fontFamily
                                font.pixelSize: Theme.fontSmall
                                color: Theme.neutral500
                            }
                        }
                        Text {
                            visible: rowItem.status !== ""
                            text: rowItem.status
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSmall
                            font.weight: Theme.weightMedium
                            color: rowItem.status === "Connected" ? Theme.accent300 : Theme.neutral400
                        }
                    }
                }
            }

            // ---- passkey: only when the selected device needs one and is not connected ------
            AppTextField {
                id: passkey
                Layout.fillWidth: true
                visible: DeviceManager.needsPasskey
                echoMode: TextInput.Password
                passwordCharacter: "•"
                placeholderText: "Printed on the amplifier label"
                text: DeviceManager.passkey
                accessibleLabel: "Passkey"
                onTextEdited: DeviceManager.passkey = text
                onAccepted: if (DeviceManager.canConnect) DeviceManager.connectSelected()
            }
        }

        // ============================== Wi-Fi tab ==================================
        ColumnLayout {
            id: wifiView
            Layout.fillWidth: true
            visible: DeviceManager.transport === 1
            spacing: Theme.s4

            readonly property string phase: DeviceManager.wifiPhase
            readonly property bool busy: phase === "searching" || phase === "connecting"
            readonly property string connectLabel:
                DeviceManager.state === DeviceManager.Connecting ? "Connecting…"
                : DeviceManager.state === DeviceManager.Connected ? "Connected" : "Connect"

            // status row: one line, what is happening or what is wrong
            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.s2

                // a quiet 3-dot "working" cue; opacity only, nothing spins
                Row {
                    visible: wifiView.busy
                    spacing: 3
                    Repeater {
                        model: 3
                        Rectangle {
                            required property int index
                            width: 4; height: 4; radius: 2
                            color: Theme.accent
                            anchors.verticalCenter: parent.verticalCenter
                            SequentialAnimation on opacity {
                                running: wifiView.busy && root.visible
                                loops: Animation.Infinite
                                PauseAnimation { duration: index * 160 }
                                NumberAnimation { to: 0.25; duration: 360 }
                                NumberAnimation { to: 1.0; duration: 360 }
                                PauseAnimation { duration: (2 - index) * 160 }
                            }
                        }
                    }
                }
                Text {
                    Layout.fillWidth: true
                    text: DeviceManager.wifiStatusLine
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    font.family: Theme.fontFamily
                    font.pixelSize: 12
                    // The palette has no warning colour: a problem is stated in the
                    // primary text colour, normal status in the muted one.
                    color: wifiView.phase === "error" ? Theme.text : Theme.neutral400
                }
            }

            // the device the PC is connected to, same look as a selected USB row
            AbstractButton {
                id: card
                Layout.fillWidth: true
                Layout.preferredHeight: 52
                visible: DeviceManager.wifiDeviceName !== ""
                enabled: false

                Accessible.role: Accessible.StaticText
                Accessible.name: DeviceManager.wifiDeviceName + ", " + DeviceManager.wifiDeviceMeta
                                 + ", " + DeviceManager.wifiDeviceStatus

                background: Item {
                    Glow { radius: Theme.radiusMd }
                    Rectangle {
                        anchors.fill: parent
                        radius: Theme.radiusMd
                        color: Theme.accent900
                        border.width: 1
                        border.color: Theme.accent700
                    }
                }
                contentItem: RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Theme.s4
                    anchors.rightMargin: Theme.s4
                    spacing: Theme.s4

                    Icon {
                        name: DeviceManager.iconName === "wifi-slash" ? "wifi-medium" : DeviceManager.iconName
                        size: 20
                        color: Theme.accent
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 1
                        Text {
                            Layout.fillWidth: true
                            text: DeviceManager.wifiDeviceName
                            elide: Text.ElideRight
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontBase
                            font.weight: Theme.weightMedium
                            color: Theme.text
                        }
                        Text {
                            Layout.fillWidth: true
                            text: DeviceManager.wifiDeviceMeta
                            elide: Text.ElideRight
                            font.family: Theme.numberFamily
                            font.pixelSize: Theme.fontSmall
                            color: Theme.neutral500
                        }
                    }
                    Text {
                        text: DeviceManager.wifiDeviceStatus
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSmall
                        font.weight: Theme.weightMedium
                        color: DeviceManager.wifiDeviceStatus === "Connected" ? Theme.accent300 : Theme.neutral400
                    }
                }
            }

            // manual address: the fallback that always works, and for now the only way
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Theme.s2

                Kicker { text: "Amplifier address" }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.s3

                    AppTextField {
                        id: hostField
                        Layout.fillWidth: true
                        placeholderText: "192.168.4.37  or  fes-nrf7002.local"
                        text: DeviceManager.wifiHost
                        enabled: !wifiView.busy && DeviceManager.state !== DeviceManager.Connected
                        inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhUrlCharactersOnly
                        accessibleLabel: "Amplifier address"
                        onTextEdited: DeviceManager.wifiHost = text
                        onAccepted: if (DeviceManager.canConnectWifi) DeviceManager.connectWifi()
                    }
                    AppTextField {
                        id: portField
                        Layout.preferredWidth: 84
                        placeholderText: "5000"
                        text: DeviceManager.wifiPort > 0 ? String(DeviceManager.wifiPort) : ""
                        enabled: hostField.enabled
                        horizontalAlignment: TextInput.AlignHCenter
                        font.family: Theme.numberFamily
                        inputMethodHints: Qt.ImhDigitsOnly
                        validator: IntValidator { bottom: 1; top: 65535 }
                        accessibleLabel: "Port"
                        onTextEdited: DeviceManager.wifiPort = text === "" ? 0 : parseInt(text)
                        onAccepted: if (DeviceManager.canConnectWifi) DeviceManager.connectWifi()
                    }
                }

                // what is wrong with it, only when something is
                Text {
                    Layout.fillWidth: true
                    visible: text !== ""
                    text: DeviceManager.wifiProblem
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSmall
                    color: Theme.text
                }
            }
        }

        // ---- actions, right-aligned ---------------------------------------------------------
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: Theme.s2
            spacing: Theme.s3

            Item { Layout.fillWidth: true }

            AppButton {
                visible: DeviceManager.canDisconnect
                variant: "secondary"
                text: "Disconnect"
                onClicked: DeviceManager.disconnectDevice()
            }
            AppButton {
                variant: "primary"
                Layout.minimumWidth: 104
                text: DeviceManager.transport === 1 ? wifiView.connectLabel : DeviceManager.connectLabel
                enabled: DeviceManager.transport === 1 ? DeviceManager.canConnectWifi && DeviceManager.state !== DeviceManager.Connected
                                                       : DeviceManager.canConnect
                onClicked: DeviceManager.transport === 1 ? DeviceManager.connectWifi()
                                                         : DeviceManager.connectSelected()
            }
        }
    }
}
