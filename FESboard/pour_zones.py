"""Build the copper pours from the zone-free backup.

Idempotent by construction: it always starts from bak_before_routing rather than
editing whatever is currently in the board. Removing zones from a loaded board
with b.Remove() leaves dangling state in these Python bindings ("memory leak of
type 'ZONE *'") and segfaults a few calls later.
"""
import pcbnew, sys

SRC = '/tmp/claude-1000/-home-cgu-Documents-FES-Board/67a89de9-4df2-4416-a4e9-50a38c025b79/scratchpad/base.kicad_pcb'   # LoadBoard returns None unless the name ends .kicad_pcb
DST = 'FESboard.kicad_pcb'
WANT_33V = '--with-3v3' in sys.argv

b = pcbnew.LoadBoard(SRC)
bb = b.GetBoardEdgesBoundingBox()
X0, Y0 = bb.GetX()/1e6-1, bb.GetY()/1e6-1
X1, Y1 = bb.GetRight()/1e6+1, bb.GetBottom()/1e6+1

# b.GetNetcodeFromNetname() and b.FindNet() both return a bare SwigPyObject in
# this KiCad 10 build, so take the code from an item that already has the net.
_nc = {}
def netcode(name):
    if name not in _nc:
        for fp in b.GetFootprints():
            for pad in fp.Pads():
                if pad.GetNetname() == name:
                    _nc[name] = pad.GetNetCode(); return _nc[name]
        raise SystemExit(f"net {name!r} not found on any pad")
    return _nc[name]

def add_zone(net, pts, prio, layer):
    z = pcbnew.ZONE(b)
    z.SetLayer(layer)
    z.SetNetCode(netcode(net))
    z.SetAssignedPriority(prio)
    z.SetLocalClearance(pcbnew.FromMM(0.3))
    z.SetMinThickness(pcbnew.FromMM(0.25))
    # Solid to SMD, thermal relief only for through-hole: U3's QFN pads are
    # narrower than a 0.5 mm spoke, so spokes could never form.
    z.SetPadConnection(pcbnew.ZONE_CONNECTION_THT_THERMAL)
    o = z.Outline(); o.NewOutline()
    for x, y in pts:
        o.Append(pcbnew.FromMM(x), pcbnew.FromMM(y))
    b.Add(z)

# Analog-ground region, shaped from where the GNDA pads actually are. Two things
# a plain vertical split gets wrong: J10 (SWD) sits at y 58-62, above the analog
# cloud; and U4, the analog LDO, sits just RIGHT of the analog block.
GNDA_POLY = [(X0, 78.0), (145.5, 78.0), (145.5, 112.0),
             (148.5, 112.0), (148.5, 126.0), (X0, 126.0)]

for layer in (pcbnew.F_Cu, pcbnew.B_Cu):
    add_zone('GND',  [(X0,Y0),(X1,Y0),(X1,Y1),(X0,Y1)], 0, layer)
    add_zone('GNDA', GNDA_POLY, 1, layer)

if WANT_33V:
    # +3.3V island on B.Cu only, highest priority so it carves out of GND.
    # Sized to cover the 20 dangling +3.3V vias (x 150.2-193.5, y 72.0-110.0)
    # and no more: every mm^2 is a hole in the bottom ground plane. Kept clear
    # of the analog side so GNDA is untouched. Delete this on the 4-layer build,
    # where the power plane returns on In2.Cu.
    add_zone('+3.3V', [(148.0,69.0),(195.5,69.0),(195.5,112.0),(148.0,112.0)],
             2, pcbnew.B_Cu)

# J9's through-hole GND pads only manage one thermal spoke against the required
# two, because neighbouring copper blocks the others. Give them a solid
# connection instead - the same per-pad override already used for U3's thermal
# vias, and honest: a one-spoke "thermal relief" is neither relief nor a
# connection worth having.
for fp in b.GetFootprints():
    if fp.GetReference() == 'J9':
        for pad in fp.Pads():
            if pad.GetNetname() == 'GND':
                pad.SetLocalZoneConnection(pcbnew.ZONE_CONNECTION_FULL)

pcbnew.ZONE_FILLER(b).Fill(b.Zones())
pcbnew.SaveBoard(DST, b)
for z in b.Zones():
    print(f"  {z.GetNetname():7s} {b.GetLayerName(z.GetLayer()):5s} prio {z.GetAssignedPriority()}  {z.GetFilledArea()/1e12:8.1f} mm^2")
