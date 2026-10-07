#!/usr/bin/env python3
"""Build the bundled UI fonts from the system Inter install.

Produces, in assets/fonts/:

    Inter-Regular.otf, Inter-Medium.otf
        Inter, subset to the glyphs the UI uses. The spec calls for weights 400
        and 500 only - never bold - so those are the only two shipped.

    InterTabular-Regular.otf, InterTabular-Medium.otf
        The same fonts with tabular figures made the DEFAULT digit glyphs.

WHY A SECOND FAMILY. The spec wants tabular figures on every numeric readout
(`font.features: { "tnum": 1 }`), so a heart rate going 71 -> 72 -> 100 does not
make the whole card jitter sideways. That QML property arrived in Qt 6.6; this
project is on Qt 6.4.2 (see software/CLAUDE.md), and Qt 6.4 has no other way to
switch an OpenType feature on. The spec's own fallback is "a monospaced-digit
fallback" - this is a better one, because a real monospace font looks wrong next
to Inter, while this IS Inter with the tnum alternates remapped into the cmap.

LICENCE. Inter is SIL OFL 1.1 and declares no Reserved Font Name, so a modified
derivative is allowed. It is renamed "Inter Tabular" anyway so nobody mistakes
it for upstream. The OFL text ships beside the fonts (OFL-Inter.txt).

Usage:  python3 tools/make_fonts.py [path/to/inter/otf/dir]
Needs:  fontTools  (pip install fonttools)
"""

import sys
from pathlib import Path

from fontTools import subset
from fontTools.ttLib import TTFont

SRC = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("/usr/share/fonts/opentype/inter")
OUT = Path(__file__).resolve().parent.parent / "assets" / "fonts"

WEIGHTS = {"Regular": "Inter-Regular.otf", "Medium": "Inter-Medium.otf"}

# Everything the UI can render: Basic Latin, Latin-1 (µ, ±, ×, ·, Ω lives in
# 2126), general punctuation (en/em dash, ellipsis, minus), Greek (the EEG band
# labels delta..gamma), arrows. Layout features are kept - `tnum`, `zero` and
# `case` are the point of keeping them.
UNICODES = (
    list(range(0x0020, 0x007F))
    + list(range(0x00A0, 0x0100))
    + list(range(0x0370, 0x0400))
    + list(range(0x2010, 0x2028))
    + [0x2030, 0x2126, 0x2190, 0x2191, 0x2192, 0x2193, 0x2212, 0x2248, 0x2264, 0x2265]
)


def subset_font(font: TTFont) -> TTFont:
    opts = subset.Options()
    opts.layout_features = ["*"]
    opts.name_IDs = ["*"]
    opts.notdef_outline = True
    opts.glyph_names = False
    sub = subset.Subsetter(opts)
    sub.populate(unicodes=UNICODES)
    sub.subset(font)
    return font


def tabular_mapping(font: TTFont) -> dict:
    """Glyph -> tabular alternate, from every lookup the `tnum` feature uses."""
    gsub = font["GSUB"].table
    lookups = set()
    for rec in gsub.FeatureList.FeatureRecord:
        if rec.FeatureTag == "tnum":
            lookups.update(rec.Feature.LookupListIndex)

    mapping = {}
    for idx in lookups:
        lk = gsub.LookupList.Lookup[idx]
        for st in lk.SubTable:
            st = getattr(st, "ExtSubTable", st)
            m = getattr(st, "mapping", None)  # LookupType 1: single substitution
            if m:
                mapping.update(m)
    return mapping


def make_tabular(font: TTFont, style: str) -> TTFont:
    mapping = tabular_mapping(font)
    if not mapping:
        raise SystemExit("this Inter build has no tnum substitutions - cannot make tabular font")

    remapped = 0
    for table in font["cmap"].tables:
        for cp, glyph in list(table.cmap.items()):
            if glyph in mapping:
                table.cmap[cp] = mapping[glyph]
                remapped += 1

    family = "Inter Tabular"
    full = f"{family} {style}"
    for rec in font["name"].names:
        if rec.nameID in (1, 16):
            rec.string = family
        elif rec.nameID in (2, 17):
            rec.string = style
        elif rec.nameID == 4:
            rec.string = full
        elif rec.nameID == 6:
            rec.string = full.replace(" ", "")
        elif rec.nameID == 3:
            rec.string = f"InterTabular-{style};derived-from-Inter"
    print(f"    tabular: remapped {remapped} cmap entries ({len(mapping)} tnum glyphs)")
    return font


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)

    for style, filename in WEIGHTS.items():
        src = SRC / filename
        if not src.exists():
            print(f"missing {src}", file=sys.stderr)
            return 1

        print(f"{style}:")
        plain = subset_font(TTFont(src))
        plain.save(OUT / f"Inter-{style}.otf")

        tab = make_tabular(TTFont(src), style)
        subset_font(tab).save(OUT / f"InterTabular-{style}.otf")

    for p in sorted(OUT.glob("*.otf")):
        print(f"  {p.name:28s} {p.stat().st_size / 1024:7.1f} KiB")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
