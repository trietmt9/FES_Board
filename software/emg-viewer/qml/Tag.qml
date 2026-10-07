// A small status tag (spec sections 5.2 and 7).
//
//   neutral  filter tags            neutral-800 fill, light text
//   accent   "Contraction" etc.     accent-800 fill, accent-300 text
//   outline  marker ids "M3"        1 px outline, no fill
//
// The spec's neutral tag text is "neutral-100", which its own token table does
// not define (it runs 200-900); neutral-200 is the nearest and is used here.

import QtQuick
import EmgViewer

Rectangle {
    id: root

    property string text: ""
    property string kind: "neutral"      // neutral | accent | outline
    property bool interactive: false     // draws a hover fill when it is clickable
    property bool hovered: false

    implicitWidth: label.implicitWidth + 2 * Theme.s3
    implicitHeight: 22
    radius: 6

    color: kind === "accent" ? Theme.accent800
         : kind === "neutral" ? (hovered && interactive ? Theme.neutral700 : Theme.neutral800)
         : "transparent"
    border.width: kind === "outline" ? 1 : 0
    border.color: Theme.accent700

    Text {
        id: label
        anchors.centerIn: parent
        text: root.text
        font.family: Theme.numberFamily       // tags carry numbers: "HP 0.5 Hz"
        font.pixelSize: Theme.fontSmall
        font.weight: Theme.weightRegular
        color: root.kind === "neutral" ? Theme.neutral200 : Theme.accent300
    }
}
