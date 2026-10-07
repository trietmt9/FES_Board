// Spec section 5.3: one channel row - a 96 px label column, then the plot.
//
// The plot is a WaveItem in the time domain and a SpectrumItem in the frequency
// domain; both exist and one is hidden, so switching domain is instant and the
// hidden one does no work (its repaint tick checks visibility).

import QtQuick
import QtQuick.Layouts
import EmgViewer

Item {
    id: root

    required property int rowIndex
    required property var rowData       // { name, location, hasData, scale }

    readonly property bool timeDomain: SignalModel.domain === SignalModel.Time

    // ---- label column ----------------------------------------------------------
    ColumnLayout {
        id: labels
        x: Theme.s5
        width: 96                            // the spec's 96 px label column
        anchors.verticalCenter: parent.verticalCenter
        spacing: 2

        Text {
            text: root.rowData.name
            font.family: Theme.fontFamily
            font.pixelSize: 14
            font.weight: Theme.weightMedium
            color: Theme.text
        }
        Text {
            text: root.rowData.location
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSmall
            color: Theme.neutral500
        }
        Text {
            // The vertical scale, with its unit: "show units and scale on every
            // channel" (spec section 10).
            text: root.rowData.scale
            font.family: Theme.numberFamily
            font.pixelSize: Theme.fontKicker
            color: Theme.accent300
        }
    }

    // ---- plot --------------------------------------------------------------------
    Item {
        id: plot
        anchors.left: labels.right
        anchors.leftMargin: 0
        anchors.right: parent.right
        anchors.rightMargin: Theme.s5
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        // The trace and its glow, drawn by the GPU (WaveTrace)...
        WaveTrace {
            anchors.fill: parent
            visible: root.timeDomain
            model: SignalModel
            row: root.rowIndex

            trace: Theme.accent400
            glow: Theme.accent
            head: Theme.accent200
        }

        // ...over the static layer: grid, markers, "device offline" (WaveItem).
        WaveItem {
            anchors.fill: parent
            z: -1
            visible: root.timeDomain
            model: SignalModel
            row: root.rowIndex
            markerLabels: root.rowIndex === 0     // the "M1" chip sits on the top row only
            fontFamily: Theme.fontFamily

            gridMinor: Theme.neutral900
            gridMajor: Theme.neutral800
            gridCenter: Theme.neutral700
            markerLine: Theme.accent300
            markerFill: Theme.accent800
            markerText: Theme.accent200
            emptyText: Theme.neutral500
        }

        // The curve, fill, glow and peak dot, drawn by the GPU (SpectrumTrace)...
        SpectrumTrace {
            anchors.fill: parent
            visible: !root.timeDomain
            model: SignalModel
            row: root.rowIndex

            accent: Theme.accent
            line: Theme.accent400
            peakDot: Theme.accent200
        }

        // ...over the static layer: grid, Hz labels, EEG bands (SpectrumItem, Grid)...
        SpectrumItem {
            anchors.fill: parent
            z: -1
            part: SpectrumItem.Grid
            visible: !root.timeDomain
            model: SignalModel
            row: root.rowIndex
            fontFamily: Theme.fontFamily
            numberFamily: Theme.numberFamily

            gridMinor: Theme.neutral900
            gridMajor: Theme.neutral800
            axisText: Theme.neutral600
            accent: Theme.accent
            peakText: Theme.accent300
            peakChip: Theme.neutral900
            bandText: Theme.neutral500
            emptyText: Theme.neutral500
        }

        // ...and the peak's chip ABOVE the curve, so the line never cuts through it.
        SpectrumItem {
            anchors.fill: parent
            z: 1
            part: SpectrumItem.Chip
            visible: !root.timeDomain
            model: SignalModel
            row: root.rowIndex
            fontFamily: Theme.fontFamily
            numberFamily: Theme.numberFamily
            peakText: Theme.accent300
            peakChip: Theme.neutral900
        }
    }

    FadedRule {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
}
