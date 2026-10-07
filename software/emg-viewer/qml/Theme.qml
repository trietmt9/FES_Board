pragma Singleton

// Every design token from Biosignal Monitor - Qt Spec.md, section 1, in one place.
//
// Nothing else in the UI hard-codes a colour, size or radius: if a number appears
// in a component it is either layout arithmetic or it came from here. That is the
// whole point of the file, and it is why the render items take their colours as
// properties instead of knowing any of these.
//
// Rules from the spec, kept in view because they are easy to break by accident:
//   * no pure black, no pure white
//   * the accent is a line and a glow, never a large fill
//   * paragraph-size accent text uses accent-300
//   * weights 400 and 500 only - never bold

import QtQuick

QtObject {
    // ---- 1.1 colours ----------------------------------------------------------
    readonly property color bg:        "#161826"
    readonly property color surface:   "#232532"
    readonly property color text:      "#E9E9ED"
    readonly property color divider:   "#29E9E9ED"     // text at 16 % alpha (ARGB)

    readonly property color accent:    "#9184D9"
    readonly property color accent200: "#E7E5FE"
    readonly property color accent300: "#D2CEFD"
    readonly property color accent400: "#B5ABFC"
    readonly property color accent600: "#796CBF"
    readonly property color accent700: "#5D5294"
    readonly property color accent800: "#423A6A"
    readonly property color accent900: "#2B2741"

    readonly property color neutral200: "#E4E7F5"
    readonly property color neutral300: "#CFD3E5"
    readonly property color neutral400: "#B2B6CA"
    readonly property color neutral500: "#9397AB"
    readonly property color neutral600: "#75798C"
    readonly property color neutral700: "#595D6C"
    readonly property color neutral800: "#3F424D"
    readonly property color neutral900: "#292B31"

    // ---- 1.2 typography ---------------------------------------------------------
    // Inter, weights 400 and 500. Numbers use "Inter Tabular": the same font with
    // the tnum alternates made the default digits (tools/make_fonts.py), because
    // Qt 6.4 has no font.features to switch the OpenType feature on.
    readonly property string fontFamily:   "Inter"
    readonly property string numberFamily: "Inter Tabular"
    readonly property string iconFamily:   "Phosphor"

    readonly property int fontBase:        13
    readonly property int fontSmall:       11
    readonly property int fontKicker:      10
    readonly property int fontTitle:       17
    readonly property int fontDialogTitle: 18
    readonly property int fontHero:        56
    readonly property int fontMetric:      20

    readonly property int weightRegular: Font.Normal     // 400
    readonly property int weightMedium:  Font.Medium     // 500

    readonly property real kickerSpacing: 1.0            // 0.1 em at 10 px
    readonly property real heroSpacing:   -1.12          // -0.02 em at 56 px

    // ---- 1.3 spacing / radius ----------------------------------------------------
    readonly property int s1: 3
    readonly property int s2: 6
    readonly property int s3: 8
    readonly property int s4: 11
    readonly property int s5: 17
    readonly property int s6: 22

    readonly property int radiusSm: 4
    readonly property int radiusMd: 8
    readonly property int radiusLg: 14

    // ---- 2 window layout ------------------------------------------------------------
    readonly property int headerHeight:  52
    readonly property int railWidth:     232
    readonly property int railWidthNarrow: 200
    readonly property int metricsWidth:  272
    readonly property int bodyPadding:   17
    readonly property int columnGap:     17
    readonly property int minDesignWidth: 1200
    readonly property int minDesignHeight: 720
    readonly property int wavePanelMinHeight: 620      // narrow layout
    readonly property int metricCardMinWidth: 240      // narrow layout
    readonly property int fadeLength: 48                // faded rules

    // ---- 1.4 interaction --------------------------------------------------------------
    readonly property real hoverText:     0.07
    readonly property real pressedText:   0.14
    readonly property real hoverAccent:   0.12
    readonly property real pressedAccent: 0.22
    readonly property int  focusWidth:    2
    readonly property int  focusOffset:   2
    readonly property real disabledOpacity: 0.45

    function withAlpha(c, a) { return Qt.rgba(c.r, c.g, c.b, a) }
    /// `t` of the way from colour `a` to colour `b`.
    function mix(a, b, t) {
        return Qt.rgba(a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t,
                       a.b + (b.b - a.b) * t, a.a + (b.a - a.a) * t)
    }
    readonly property color hoverFill:          withAlpha(text, hoverText)
    readonly property color pressedFill:        withAlpha(text, pressedText)
    readonly property color accentHoverFill:    withAlpha(accent, hoverAccent)
    readonly property color accentPressedFill:  withAlpha(accent, pressedAccent)

    // ---- 1.5 icons: Phosphor Icons, regular weight --------------------------------------
    // The font is bundled (qrc:/fonts/Phosphor.ttf) rather than SVGs: this Qt build
    // has no SVG image plugin and no Qt Quick Shapes, so an icon FONT is the one
    // option in the spec that works here. Codepoints are from Phosphor's own CSS.
    readonly property var icons: ({
        "pulse":           0xE000,
        "user":            0xE4C2,
        "wifi-high":       0xE4EA,
        "wifi-medium":     0xE4EE,
        "wifi-low":        0xE4EC,
        "wifi-slash":      0xE4F2,
        "caret-down":      0xE136,
        "battery-high":    0xE0C2,
        "record":          0xE3EE,
        "heartbeat":       0xE2AC,
        "brain":           0xE74E,
        "hand-fist":       0xE57A,
        "wave-sine":       0xEA9A,
        "chart-line":      0xE154,
        "flag":            0xE244,
        "pause":           0xE39E,
        "play":            0xE3D0,
        "x":               0xE4F6,
        "arrow-clockwise": 0xE036
    })
}
