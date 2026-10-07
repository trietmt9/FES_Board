import json
from pathlib import Path

PATH = Path('/home/cgu/Documents/FES_Board/FESboard/FESboard.kicad_pro')
with PATH.open() as f:
    j = json.load(f)

classes = j['net_settings']['classes']
def find(name):
    for c in classes:
        if c.get('name') == name:
            return c
    raise SystemExit(f"class {name} not found")

for name, new_w in (('PWR', 0.60), ('SWNODE', 0.60)):
    c = find(name)
    old = c.get('track_width')
    if old != new_w:
        c['track_width'] = new_w
        print(f"{name}: track_width {old} -> {new_w}")
    else:
        print(f"{name}: already {new_w}")

with PATH.open('w') as f:
    json.dump(j, f, indent=2)
print("saved.")
