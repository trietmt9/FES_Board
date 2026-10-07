// Spec section 7: the metrics panel.
//
// Refreshed every 400 ms from SignalModel.metrics. Every value on it is either
// measured or an em dash; the three that the mockup left static and the spec says
// must come from real measurement (PR, QRS, QTc, electrode impedance, battery)
// have no measurement behind them here, and say so.
//
// Wide layout: one 272 px column of cards. Narrow layout (< 1200 px window): the
// same cards in a wrapping row, each at least 240 px.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import EmgViewer

Flow {
    id: root

    property bool wrap: false                     // narrow layout
    readonly property int gap: Theme.s4

    spacing: gap

    // Number of cards per line in the wrapping layout, and the width that makes
    // them fill it: "min 240 px each".
    readonly property int columns: wrap ? Math.max(1, Math.floor((width + gap) / (Theme.metricCardMinWidth + gap))) : 1
    readonly property real cardWidth: (width - (columns - 1) * gap) / columns
    readonly property real halfWidth: (cardWidth - gap) / 2

    readonly property int sig: SignalModel.signal

    // A missing key (the map differs per signal) reads as "", not undefined, so
    // hidden cards do not log a binding warning on every refresh.
    function m(key) {
        const v = SignalModel.metrics[key]
        return v === undefined || v === null ? "" : v
    }

    // The same for a list. During the instant between a signal switch and the next
    // metrics refresh the map still belongs to the previous signal; a Repeater
    // handed a string there would misbehave, so give it an empty list instead.
    function list(key) {
        const v = SignalModel.metrics[key]
        return Array.isArray(v) ? v : []
    }

    // ---- building blocks -----------------------------------------------------------
    component Card: Rectangle {
        id: card
        default property alias content: inner.data
        property alias spacing: inner.spacing
        property int pad: Theme.s5

        color: Theme.surface
        radius: Theme.radiusMd
        implicitHeight: inner.implicitHeight + 2 * pad

        ColumnLayout {
            id: inner
            anchors.fill: parent
            anchors.margins: card.pad
            spacing: Theme.s2
        }
    }

    // The big number: 56 px / 500, -0.02 em, tabular figures.
    component HeroCard: Card {
        id: hero
        property string kicker: ""
        property string value: ""
        property string unit: ""
        property string tagText: ""
        property bool tagAccent: false
        property string footnote: ""

        Kicker { Layout.fillWidth: true; text: hero.kicker }

        RowLayout {
            spacing: Theme.s3
            Text {
                text: value === "" ? "—" : value
                font.family: Theme.numberFamily
                font.pixelSize: Theme.fontHero
                font.weight: Theme.weightMedium
                font.letterSpacing: Theme.heroSpacing
                color: value === "" || value === "—" ? Theme.neutral600 : Theme.text
            }
            Text {
                Layout.alignment: Qt.AlignBottom
                Layout.bottomMargin: 10
                text: unit
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontBase
                color: Theme.neutral400
            }
        }

        RowLayout {
            spacing: Theme.s3
            Tag {
                visible: tagText !== ""
                kind: tagAccent ? "accent" : "neutral"
                text: tagText
            }
            Text {
                visible: footnote !== ""
                text: footnote
                font.family: Theme.numberFamily
                font.pixelSize: Theme.fontSmall
                color: Theme.neutral500
            }
        }
    }

    component StatCard: Card {
        property string label: ""
        property string value: ""
        property string unit: ""

        Text {
            text: label
            Layout.fillWidth: true
            elide: Text.ElideRight
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSmall
            color: Theme.neutral500
        }
        RowLayout {
            spacing: 4
            Text {
                text: value === "" ? "—" : value
                font.family: Theme.numberFamily
                font.pixelSize: Theme.fontMetric
                font.weight: Theme.weightMedium
                color: value === "" || value === "—" ? Theme.neutral600 : Theme.text
            }
            Text {
                Layout.alignment: Qt.AlignBottom
                Layout.bottomMargin: 3
                visible: value !== "" && value !== "—"
                text: unit
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSmall
                color: Theme.neutral400
            }
        }
    }

    // A horizontal bar on a neutral-900 track; the fill animates over 0.4 s.
    component Bar: Item {
        property real fraction: 0
        property color fillColor: Theme.accent
        property int barHeight: 4
        property bool glow: false

        implicitHeight: barHeight
        Layout.fillWidth: true
        Layout.preferredHeight: barHeight

        Rectangle {
            anchors.fill: parent
            radius: height / 2
            color: Theme.neutral900
        }
        Rectangle {
            id: fill
            visible: parent.fraction > 0
            height: parent.height
            width: Math.max(height, parent.width * Math.min(1, parent.fraction))
            radius: height / 2
            color: parent.fillColor

            Behavior on width { NumberAnimation { duration: 400; easing.type: Easing.OutCubic } }
            Behavior on color { ColorAnimation { duration: 400 } }

            // Glow: a wider, fainter copy underneath.
            Rectangle {
                visible: parent.parent.glow
                z: -1
                x: -3; y: -3
                width: parent.width + 6
                height: parent.height + 6
                radius: height / 2
                color: Theme.withAlpha(parent.color, 0.25)
            }
        }
    }

    // =========================== ECG ===========================
    HeroCard {
        visible: root.sig === SignalModel.Ecg
        width: root.cardWidth
        kicker: "Heart rate"
        value: root.m("hr")
        unit: "bpm"
        tagText: root.m("tag")
        tagAccent: root.m("tagAccent") === true
        footnote: root.m("limits")
    }
    Flow {
        visible: root.sig === SignalModel.Ecg
        width: root.cardWidth
        spacing: root.gap

        StatCard { width: root.halfWidth; label: "RR interval";   value: root.m("rr");  unit: "ms" }
        StatCard { width: root.halfWidth; label: "PR interval";   value: root.m("pr");  unit: "ms" }
        StatCard { width: root.halfWidth; label: "QRS duration";  value: root.m("qrs"); unit: "ms" }
        StatCard { width: root.halfWidth; label: "QTc";           value: root.m("qtc"); unit: "ms" }
    }

    // =========================== EEG ===========================
    HeroCard {
        visible: root.sig === SignalModel.Eeg
        width: root.cardWidth
        kicker: "Dominant frequency · O1"
        value: root.m("dominant")
        unit: "Hz"
        tagText: root.m("bandTag")
        tagAccent: root.m("bandTagAccent") === true
    }
    Card {
        visible: root.sig === SignalModel.Eeg
        width: root.cardWidth
        spacing: Theme.s3

        Kicker { text: "Relative band power"; color: Theme.neutral500 }

        Repeater {
            model: root.sig === SignalModel.Eeg ? root.list("bands") : []

            delegate: ColumnLayout {
                required property var modelData
                Layout.fillWidth: true
                spacing: 5

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.s3

                    Text {
                        Layout.preferredWidth: 18
                        text: modelData.symbol
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontTitle - 3
                        color: Theme.accent300
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0
                        Text {
                            text: modelData.name
                            font.family: Theme.fontFamily
                            font.pixelSize: 12
                            color: Theme.text
                        }
                        Text {
                            text: modelData.range
                            font.family: Theme.numberFamily
                            font.pixelSize: Theme.fontKicker
                            color: Theme.neutral500
                        }
                    }
                    Text {
                        text: modelData.percentText
                        font.family: Theme.numberFamily
                        font.pixelSize: 12
                        font.weight: Theme.weightMedium
                        color: modelData.dominant ? Theme.accent300 : Theme.neutral300
                    }
                }
                Bar {
                    fraction: modelData.percent / 100.0
                    // the dominant band is accent, the others neutral-600
                    fillColor: modelData.dominant ? Theme.accent : Theme.neutral600
                }
            }
        }
    }

    // =========================== EMG ===========================
    HeroCard {
        visible: root.sig === SignalModel.Emg
        width: root.cardWidth
        kicker: "RMS amplitude · Biceps"
        value: root.m("rms")
        unit: "mV"
        tagText: root.m("state")
        tagAccent: root.m("stateAccent") === true
    }
    Card {
        visible: root.sig === SignalModel.Emg
        width: root.cardWidth

        RowLayout {
            Layout.fillWidth: true
            Kicker { text: "Activation"; color: Theme.neutral500; Layout.fillWidth: true }
            AppButton {
                variant: "ghost"
                implicitHeight: 22
                text: root.m("mvcSet") === true ? "Reset reference" : "Set reference"
                accessibleName: "Take the recent biceps RMS peak as 100 percent MVC"
                onClicked: SignalModel.captureMvc()
            }
        }
        RowLayout {
            spacing: 4
            Text {
                text: root.m("mvcText") === "" ? "—" : root.m("mvcText")
                font.family: Theme.numberFamily
                font.pixelSize: Theme.fontMetric
                font.weight: Theme.weightMedium
                color: root.m("mvcSet") === true ? Theme.text : Theme.neutral600
            }
            Text {
                Layout.alignment: Qt.AlignBottom
                Layout.bottomMargin: 3
                // Always shown: the card's kicker says "Activation", and "% MVC" is
                // what the number is a percentage OF - including while it is a dash.
                text: "% MVC"
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSmall
                color: Theme.neutral400
            }
        }
        Bar {
            barHeight: 6
            glow: true
            fraction: root.m("mvcFraction") === "" ? 0 : root.m("mvcFraction")
        }
        Text {
            visible: root.m("mvcSet") !== true
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: "No reference yet. Contract fully, then press Set reference."
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSmall
            color: Theme.neutral500
        }
    }
    Flow {
        visible: root.sig === SignalModel.Emg
        width: root.cardWidth
        spacing: root.gap

        StatCard { width: root.halfWidth; label: "Median frequency"; value: root.m("median");  unit: "Hz" }
        StatCard { width: root.halfWidth; label: "Triceps RMS";      value: root.m("triceps"); unit: "mV" }
    }

    // =========================== markers (all signals) ===========================
    Card {
        width: root.cardWidth
        spacing: Theme.s3

        RowLayout {
            Layout.fillWidth: true
            Text {
                Layout.fillWidth: true
                text: "Markers"
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontBase
                font.weight: Theme.weightMedium
                color: Theme.text
            }
            Text {
                text: SignalModel.markerCount + " this view"
                font.family: Theme.numberFamily
                font.pixelSize: Theme.fontSmall
                color: Theme.neutral500
            }
        }

        Text {
            visible: SignalModel.markerCount === 0
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: "Press M or Marker to annotate the trace."
            font.family: Theme.fontFamily
            font.pixelSize: 12
            color: Theme.neutral500
        }

        Repeater {
            model: SignalModel.recentMarkers

            delegate: RowLayout {
                required property var modelData
                Layout.fillWidth: true
                spacing: Theme.s3

                Tag { kind: "outline"; text: modelData.tag }
                Text {
                    Layout.fillWidth: true
                    text: modelData.label
                    elide: Text.ElideRight
                    font.family: Theme.fontFamily
                    font.pixelSize: 12
                    color: Theme.neutral300
                }
                Text {
                    text: modelData.time
                    font.family: Theme.numberFamily
                    font.pixelSize: Theme.fontSmall
                    color: Theme.neutral400
                }
            }
        }
    }

    // =========================== footer ===========================
    Item {
        width: root.cardWidth
        implicitHeight: footerCol.implicitHeight + Theme.s3

        ColumnLayout {
            id: footerCol
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: Theme.s2

            FadedRule { Layout.fillWidth: true }

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 2
                Text {
                    text: "Status"
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSmall
                    color: Theme.neutral500
                }
                Item { Layout.fillWidth: true }
                Text {
                    text: SignalModel.statusText
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSmall
                    font.weight: Theme.weightMedium
                    color: SignalModel.online ? Theme.neutral200 : Theme.neutral400
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: "Electrode impedance"
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSmall
                    color: Theme.neutral500
                }
                Item { Layout.fillWidth: true }
                Text {
                    // Lead-off detection is not enabled in the firmware (B-014), so
                    // there is no impedance to report - not even a "< 5 kOhm" bound.
                    text: "—"
                    font.family: Theme.numberFamily
                    font.pixelSize: Theme.fontSmall
                    color: Theme.neutral500
                }
            }
        }
    }
}
