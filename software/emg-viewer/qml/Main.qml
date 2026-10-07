// Spec section 2: window layout.
//
//   Header (52)
//   [ signal rail 232 | wave panel (fills) | metrics panel 272 ]
//
// Body padding and column gap are 17. Below 1200 px the columns become
// 200 | fill, and the metrics panel drops underneath as a wrapping row of cards;
// the window then scrolls vertically and the wave panel keeps a 620 px minimum.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import EmgViewer

ApplicationWindow {
    id: win

    title: "BioView Monitor"
    visible: true
    // Designed for 1440 x 900; fit a smaller screen rather than open off its edge.
    width: Math.min(1440, Screen.desktopAvailableWidth)
    height: Math.min(900, Screen.desktopAvailableHeight)
    minimumWidth: 640
    minimumHeight: 560
    color: Theme.bg

    readonly property bool wide: width >= Theme.minDesignWidth
    readonly property bool modalOpen: wifiDialog.opened

    // Called from main() for --open-devices / --diagnostics.
    function openDevices() { wifiDialog.open() }
    function openDiagnostics() { diagnostics.shown = true }

    // ---- background: bg plus a soft accent-900 glow from the top-left ---------------
    // CSS: radial-gradient(1200px 600px at 0 0, accent-900, transparent 60%).
    // Drawn once; it does not depend on the window size.
    Canvas {
        x: 0; y: 0
        width: 1200; height: 600
        z: -10
        renderTarget: Canvas.Image
        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            // Scale x by 2 so a circle of radius 600 becomes a 1200 x 600 ellipse.
            ctx.scale(2, 1)
            const g = ctx.createRadialGradient(0, 0, 0, 0, 0, 600)
            const c = Theme.accent900
            g.addColorStop(0.0, Qt.rgba(c.r, c.g, c.b, 1.0))
            g.addColorStop(0.6, Qt.rgba(c.r, c.g, c.b, 0.0))
            ctx.fillStyle = g
            ctx.fillRect(0, 0, 600, 600)
        }
    }

    Header {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onOpenDevices: win.openDevices()
    }

    Flickable {
        id: scroller
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        clip: true

        contentWidth: width
        contentHeight: win.wide ? height : grid.implicitHeight + 2 * Theme.bodyPadding
        interactive: !win.wide
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar { policy: win.wide ? ScrollBar.AlwaysOff : ScrollBar.AsNeeded }

        GridLayout {
            id: grid
            x: Theme.bodyPadding
            y: Theme.bodyPadding
            width: scroller.width - 2 * Theme.bodyPadding
            // Wide: fill the window exactly. Narrow: as tall as the content needs.
            height: win.wide ? scroller.height - 2 * Theme.bodyPadding : implicitHeight

            columns: win.wide ? 3 : 2
            columnSpacing: Theme.columnGap
            rowSpacing: Theme.columnGap

            SignalRail {
                Layout.preferredWidth: win.wide ? Theme.railWidth : Theme.railWidthNarrow
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignTop
            }

            WavePanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: win.wide ? 0 : Theme.wavePanelMinHeight
            }

            // Metrics: a 272 px column in the wide layout (scrolling if it is taller
            // than the window), a full-width wrapping row of cards below otherwise.
            Flickable {
                id: metricsScroll
                Layout.columnSpan: win.wide ? 1 : 2
                Layout.preferredWidth: win.wide ? Theme.metricsWidth : -1
                Layout.fillWidth: !win.wide
                Layout.fillHeight: win.wide
                Layout.preferredHeight: win.wide ? -1 : panel.implicitHeight

                contentWidth: width
                contentHeight: panel.implicitHeight
                clip: true
                interactive: win.wide && contentHeight > height
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                MetricsPanel {
                    id: panel
                    width: metricsScroll.width
                    wrap: !win.wide
                }
            }
        }
    }

    WifiDialog {
        id: wifiDialog
        parent: Overlay.overlay
    }

    Diagnostics {
        id: diagnostics
        parent: Overlay.overlay
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        z: 20
        onCloseRequested: shown = false
    }

    // ---- keyboard (spec sections 4 and 9) ----------------------------------------------
    // Off while the dialog is open, so typing a passkey cannot change the signal.
    //
    // Space is also how a keyboard user activates a focused button, so it stands
    // down while a control has keyboard focus. Controls only take focus from Tab
    // (focusPolicy: TabFocus), so a mouse user never loses the global Freeze.
    readonly property bool controlHasFocus:
        activeFocusItem !== null && activeFocusItem.visualFocus !== undefined && activeFocusItem.visualFocus

    Shortcut { sequence: "1"; enabled: !win.modalOpen; onActivated: SignalModel.selectSignal(0) }
    Shortcut { sequence: "2"; enabled: !win.modalOpen; onActivated: SignalModel.selectSignal(1) }
    Shortcut { sequence: "3"; enabled: !win.modalOpen; onActivated: SignalModel.selectSignal(2) }
    Shortcut { sequence: "M"; enabled: !win.modalOpen; onActivated: SignalModel.addMarker() }
    Shortcut { sequence: "F"; enabled: !win.modalOpen; onActivated: SignalModel.toggleDomain() }
    Shortcut { sequence: "Space"; enabled: !win.modalOpen && !win.controlHasFocus; onActivated: SignalModel.togglePause() }
    Shortcut { sequence: "D"; enabled: !win.modalOpen; onActivated: diagnostics.shown = !diagnostics.shown }
    Shortcut { sequence: "Escape"; enabled: diagnostics.shown && !win.modalOpen; onActivated: diagnostics.shown = false }
}
