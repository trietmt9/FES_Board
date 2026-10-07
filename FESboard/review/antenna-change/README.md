# Antenna keepout draft — not fabrication ready

The draft PCB adds a GND pour on F.Cu/B.Cu and U7_ANTENNA_KEEPOUT at x=166.1–179.0 mm, y=57.9–84.5 mm. It conservatively covers the antenna extension of the currently embedded module footprint while retaining the first 17.6 mm of the module body ground.

Refill revealed that existing clock/power circuitry on F.Cu occupies this region. The full keepout flags 80 forbidden items and removes ground paths, leaving 25 unconnected items. The existing routing must be moved, or U7 relocated and rerouted, before this draft can be used. Do not fabricate the draft.

On 2026-09-15, the newly saved main `FESboard.kicad_pcb` was used as the source of truth. Separate solid-connected GND pours were added to F.Cu and B.Cu, together with a copper-pour-only keepout for U7's antenna. Existing tracks and vias beneath the antenna were retained so functional nets were not cut. DRC after refill reports one disconnected section within the F.Cu GND zone, one starved thermal, and the pre-existing spacing/width/drill/silkscreen issues. No schematic changes were made.

Manufacturer drawing checked: WT02C40C Product Specifications, January 2026, pp. 8 and 19. The embedded footprint outline differs from the current manufacturer drawing; confirm the purchased module revision when relocating it.
