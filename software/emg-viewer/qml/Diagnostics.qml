// Link and device diagnostics - NOT part of the spec.
//
// The spec's window has no place for this, but the project cannot do without it:
// "lost samples", CRC errors and the measured-versus-advertised rate are how the
// DRDY and timebase faults (B-020, B-026, B-033) were found, and the firmware's own
// console lines (DRDY alive, rate locked) arrive here. So it exists, but stays off
// the spec'd view: a drawer on the right, opened with D, closed with D or Escape.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import EmgViewer

Rectangle {
    id: root

    property bool shown: false
    signal closeRequested()

    width: 380
    visible: x < parent.width
    x: shown ? parent.width - width : parent.width
    Behavior on x { NumberAnimation { duration: 160; easing.type: Easing.OutCubic } }

    color: Theme.surface
    border.width: 1
    border.color: Theme.neutral800

    readonly property var stats: Acquisition.stats

    function fmt(n, d) { return Number(n).toLocaleString(Qt.locale("C"), "f", d === undefined ? 0 : d) }

    component Row2: RowLayout {
        property string label: ""
        property string value: ""
        property color valueColor: Theme.neutral200
        Layout.fillWidth: true
        Text {
            text: parent.label
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSmall
            color: Theme.neutral500
        }
        Item { Layout.fillWidth: true }
        Text {
            text: parent.value
            font.family: Theme.numberFamily
            font.pixelSize: Theme.fontSmall
            color: parent.valueColor
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.s5
        spacing: Theme.s3

        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3
                Kicker { text: "Diagnostics" }
                Text {
                    text: Acquisition.statusText
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    font.family: Theme.fontFamily
                    font.pixelSize: 12
                    color: Theme.neutral400
                }
            }
            AppButton {
                variant: "ghost"
                iconName: "x"
                implicitWidth: 30
                implicitHeight: 30
                accessibleName: "Close diagnostics"
                onClicked: root.closeRequested()
            }
        }

        // ---- link ----------------------------------------------------------------------
        Kicker { text: "Link"; color: Theme.neutral500 }
        Row2 { label: "Rate, measured / advertised"
               value: root.fmt(root.stats.measuredSps, 0) + " / " + root.fmt(root.stats.nominalSps, 0) + " SPS" }
        Row2 { label: "Throughput"; value: root.fmt(root.stats.bytesPerSecond / 1024, 1) + " KiB/s" }
        Row2 { label: "Data frames"; value: root.fmt(root.stats.dataFrames) }
        Row2 { label: "CRC errors"; value: root.fmt(root.stats.crcErrors)
               valueColor: root.stats.crcErrors > 0 ? Theme.accent300 : Theme.neutral200 }
        Row2 { label: "Resyncs"; value: root.fmt(root.stats.resyncs) }
        Row2 { label: "Dropped frames"; value: root.fmt(root.stats.droppedFrames)
               valueColor: root.stats.droppedFrames > 0 ? Theme.accent300 : Theme.neutral200 }
        Row2 { label: "Lost samples"
               value: root.fmt(root.stats.lostSamples) + "  (" + root.fmt(root.stats.lostPercent, 2) + " %)"
               valueColor: root.stats.lostPercent >= 0.1 ? Theme.accent300 : Theme.neutral200 }

        FadedRule { Layout.fillWidth: true }

        // ---- device ----------------------------------------------------------------------
        Kicker { text: "Device"; color: Theme.neutral500 }
        Row2 { label: "Chip"; value: Acquisition.haveInfo ? Acquisition.chipName : "—" }
        Row2 { label: "Firmware"; value: Acquisition.haveInfo && Acquisition.firmwareVersion !== "" ? Acquisition.firmwareVersion : "—" }
        Row2 { label: "Channels streamed"; value: Acquisition.haveInfo ? Acquisition.channelCount : "—" }
        Row2 { label: "PGA gain / VREF"
               value: Acquisition.haveInfo ? ("×" + Acquisition.gain + " / " + Acquisition.vrefVolts.toFixed(2) + " V") : "—" }

        FadedRule { Layout.fillWidth: true }

        // ---- console ----------------------------------------------------------------------
        RowLayout {
            Layout.fillWidth: true
            Kicker { text: "Console"; color: Theme.neutral500; Layout.fillWidth: true }
            AppButton {
                variant: "ghost"
                implicitHeight: 22
                text: "Clear"
                onClicked: Acquisition.clearLog()
            }
        }

        ListView {
            id: log
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: Acquisition.log
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

            // Follow the tail unless the user has scrolled up to read.
            property bool atTail: true
            onContentYChanged: atTail = (contentY + height >= contentHeight - 4)
            onCountChanged: if (atTail) Qt.callLater(positionViewAtEnd)

            // LogModel names its role "line", not "text": a role called "text" would
            // shadow Text.text in this delegate and render nothing.
            delegate: Text {
                width: ListView.view.width
                text: model.line
                wrapMode: Text.WrapAnywhere
                font.family: Theme.numberFamily
                font.pixelSize: 10
                color: model.severity === "err" ? Theme.accent200
                     : model.severity === "wrn" ? Theme.accent300 : Theme.neutral400
            }
        }
    }
}
