// Small caps label above a title: 10 px / 500, 0.1 em tracking, accent colour
// (spec section 1.2).

import QtQuick
import EmgViewer

Text {
    // Off for a kicker whose text is already composed with its own casing, such as
    // "TIME DOMAIN . ECG . 250 Hz sampling" - uppercasing that would give "250 HZ".
    property bool uppercase: true

    font.family: Theme.fontFamily
    font.pixelSize: Theme.fontKicker
    font.weight: Theme.weightMedium
    font.letterSpacing: Theme.kickerSpacing
    font.capitalization: uppercase ? Font.AllUppercase : Font.MixedCase
    color: Theme.accent
    elide: Text.ElideRight
}
