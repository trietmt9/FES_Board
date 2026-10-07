# analysis — environment

Signal analysis and filter design for the FES_Board. Deliberately separate from
`firmware/` and `software/`: it runs on a workstation, produces plots and
numbers, and ships nothing to the board.

**Verified working on this machine, 2026-09-03.** Everything below was run, not
copied from documentation.

## Why a venv and not apt

Python 3.12 here is PEP 668 managed — `/usr/lib/python3.12/EXTERNALLY-MANAGED`
exists, so `pip install` into the system interpreter is refused. Two ways round
it:

| | apt | venv |
|---|---|---|
| needs sudo | yes | no |
| numpy / scipy / matplotlib | 1.26.4 / 1.11.4 / 3.6.3 | **2.5.2 / 1.18.1 / 3.11.1** |
| `bioread` for BIOPAC `.acq` | **not packaged** | yes |
| removable | no, system-wide | `rm -rf .venv` |

`bioread` decides it: it is not in apt, so an apt-only setup cannot read `.acq`
files at all. The venv is project-local and disposable.

## Setup

```bash
cd analysis
python3 -m venv .venv
.venv/bin/pip install numpy scipy matplotlib bioread
```

Installed and confirmed importable:

```
numpy        2.5.2
scipy        1.18.1
matplotlib   3.11.1
bioread      2025.05.02
```

Costs 334 MB in `analysis/.venv/`, which `.gitignore` excludes along with
`out/`.

## Running

Always through the venv's interpreter — there is no need to "activate" it:

```bash
.venv/bin/python analyse.py session.acq
```

Or activate if you prefer a shell session:

```bash
source .venv/bin/activate
python analyse.py session.acq
deactivate
```

## Plotting is headless

Scripts must set the Agg backend **before** importing pyplot:

```python
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
```

Plots are written to `out/`, never shown in a window. That keeps the tools
usable over SSH and makes every result a file you can point at later, rather
than something that vanished when the window closed.

## Smoke test

This ran and produced `out/smoketest.png` — a 4th-order 20–450 Hz band-pass at
4520 SPS, correct at the corners:

```bash
.venv/bin/python - <<'EOF'
import matplotlib; matplotlib.use("Agg")
import matplotlib.pyplot as plt, numpy as np
from scipy import signal
import bioread                      # noqa: F401 - import check only

fs = 4520.0
sos = signal.butter(4, [20, 450], btype="band", fs=fs, output="sos")
w, h = signal.sosfreqz(sos, worN=2048, fs=fs)

fig, ax = plt.subplots(figsize=(6, 3))
ax.semilogx(w[1:], 20 * np.log10(abs(h[1:])))
ax.set_xlim(1, fs / 2); ax.set_ylim(-60, 5)
ax.grid(True, which="both", alpha=0.3)
ax.set_xlabel("Hz"); ax.set_ylabel("dB")
fig.tight_layout(); fig.savefig("out/smoketest.png", dpi=90)
print("ok")
EOF
```

If that produces a plot, the environment is sound and any later failure is in
our code rather than the install.

## Layout

```
analysis/
  SETUP.md       this file
  README.md      what the tools do and why - read before writing any of them
  .venv/         gitignored
  out/           plots, gitignored
```

The tools themselves are not written yet. `README.md` is the design.

## One thing to be careful about

**scipy is for plotting and cross-checking, never for generating the
coefficients that go on the board.** Those must come from our own mirror of
`firmware/filters/src/bio_filter.c` — same RBJ formulas, same Butterworth Q
values, rounded to float32 as the firmware stores them. If scipy produced them,
comparing the model against the C would prove nothing.

The smoke test above uses `signal.butter` precisely because it is *only* a smoke
test.
