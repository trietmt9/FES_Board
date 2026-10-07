import json
from pathlib import Path

PATH = Path('/home/cgu/Documents/FES_Board/FESboard/FESboard.kicad_pro')

with PATH.open() as f:
    j = json.load(f)

ns = j.setdefault('net_settings', {})
classes = ns.setdefault('classes', [])

def find_class(name):
    for c in classes:
        if c.get('name') == name:
            return c
    return None

# 1. Update USB class for 90 ohm differential on this stackup
usb = find_class('USB')
if usb is None:
    raise SystemExit("USB class not found — aborting so nothing is silently added")
before = dict(usb)
usb['track_width']       = 0.20
usb['clearance']         = 0.15
usb['diff_pair_width']   = 0.20
usb['diff_pair_gap']     = 0.15
print("USB updated:")
for k in ('track_width', 'clearance', 'diff_pair_width', 'diff_pair_gap'):
    print(f"  {k}: {before.get(k)} -> {usb[k]}")

# 2. Add a new RF class if not present
rf = find_class('RF')
if rf is None:
    default = find_class('Default') or {}
    rf = {k: v for k, v in default.items()}
    rf['name']             = 'RF'
    rf['track_width']      = 0.15
    rf['clearance']        = 0.20
    rf['diff_pair_width']  = 0.15
    rf['diff_pair_gap']    = 0.20
    classes.append(rf)
    print("\nRF class added:")
    for k in ('track_width', 'clearance', 'diff_pair_width', 'diff_pair_gap'):
        print(f"  {k}: {rf[k]}")
else:
    print("\nRF class already exists — leaving in place")

with PATH.open('w') as f:
    json.dump(j, f, indent=2)
print("\nSaved.")
