import json
from pathlib import Path

PATH = Path('/home/cgu/Documents/FES_Board/FESboard/FESboard.kicad_pro')
with PATH.open() as f:
    j = json.load(f)

ds = j['board']['design_settings']

# leading 0.0 = "use netclass width" placeholder KiCad expects
ds['track_widths'] = [0.0, 0.15, 0.20, 0.25, 0.30, 0.40, 0.50, 0.60]

# vias: (finished diameter, drill) in mm.  Small (signal), standard, power.
ds['via_dimensions'] = [
    {'diameter': 0.0, 'drill': 0.0},
    {'diameter': 0.5, 'drill': 0.25},   # signal via
    {'diameter': 0.6, 'drill': 0.3},    # general via
    {'diameter': 0.8, 'drill': 0.4},    # power via
]

# diff pairs: (trace width, gap).  USB 2.0 90 ohm on this stackup only.
ds['diff_pair_dimensions'] = [
    {'width': 0.0,  'gap': 0.0,  'via_gap': 0.0},
    {'width': 0.20, 'gap': 0.15, 'via_gap': 0.15},   # USB 90 ohm differential
]

with PATH.open('w') as f:
    json.dump(j, f, indent=2)

print("pre-defined track widths :", ds['track_widths'])
print("pre-defined via sizes    :", ds['via_dimensions'])
print("pre-defined diff pairs   :", ds['diff_pair_dimensions'])
