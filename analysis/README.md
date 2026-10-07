# filterlab — record, analyse, then design

Filter design driven by **recorded data**, not by a specification written in
advance. Record a capture on the board, import it into Python, look at what is
actually in it, and choose the band from that.

Status: **plan only.** Nothing here is written yet.

## The workflow

```
  BIOPAC  ──►  .acq  ─┐
                      ├─►  load  ──►  LOOK  ──►  design  ──►  apply  ──►  verify
  board   ──►  .csv  ─┘             time +        pick        before/     C matches
                                   spectrum      corners       after       the model
```

The **LOOK** step is the one that has been missing. Every filter decision on this
project so far has been made from arithmetic and assumption:

- a 40 Hz low-pass was specified to remove impulse artifacts, which a linear
  filter cannot do
- the 5-point median was sized from an assumed motor-unit duration
- corners were designed against a nominal sample rate that is 13 % wrong

All three are answerable from a recording. **Nobody has yet plotted the recorded
waveform** — `decode_capture.py` prints statistics, and statistics hid a
railed-looking peak-to-peak that turned out to be two glitch samples.

## Recording

From BIOPAC: record as usual and save the `.acq`.

From this board: in the viewer, **Record…**, capture 30–60 s, stop. It writes a
`.csv` beside the raw capture; the CSV is what filterlab reads.

Record what you want to reason about:

| Goal | Record |
|---|---|
| Where is my noise? | electrodes on, source **off** |
| Is my signal in band? | source on, at a known rate |
| Do artifacts survive the filter? | a session with real movement |
| Is the timebase right? | anything — `t_ms` carries the MCU clock |

One caution: the firmware currently band-passes **before** transmitting, so a
recording is already conditioned and you cannot design a wider band from it than
the board allowed through. **To design filters, record with the on-board filter
removed** — comment out the `bio_filter_process()` call in `service_adc()` — or
you are designing against your own previous filter.

## Sources

Two formats. Both carry samples already scaled to microvolts, so nothing here
needs the board's framing, its CRC, or its INFO frame.

```python
rec = filterlab.load("session.acq")      # or .csv
rec.uv        # (n_channels, n_samples) float64, microvolts
rec.rate_hz   # sample rate
rec.names     # channel labels
rec.source    # which reader, and how the rate was determined
```

### `.acq` — BIOPAC AcqKnowledge, via `bioread`

The primary source. BIOPAC is the reference instrument in the lab and it is
trusted: when this board and BIOPAC disagreed about the same ECG simulator,
BIOPAC was right. Designing filters against its recordings means designing
against signal whose amplitude, timebase and bandwidth are not in question.

`bioread` reads all known AcqKnowledge versions — compressed and uncompressed,
Mac and Windows. Channel names, units and per-channel sample rates come out of
the file, so nothing is assumed.

**Watch the per-channel rate.** AcqKnowledge permits different rates per channel
in one file; the loader must carry each channel's own rate rather than assume a
single one, or every frequency axis is silently wrong for some of them.

### `.csv` — this board, via the viewer

The viewer writes a CSV beside every recording:

```
t_s,ch1_uv
0.000000,3322.887
0.001000,3322.887
```

Time in seconds, values already in microvolts, one column per channel. That is
why `.emgraw` is not needed here: **the CSV carries the same samples in physical
units**, and filterlab has no use for the wire framing.

**One caveat that matters for design.** `t_s` is generated from the *advertised*
sample rate, so it inherits any error in it — and this board's clock runs about
13 % fast. A CSV recorded before the firmware measured its rate has a stretched
time axis, and therefore a stretched frequency axis. `.acq` has no such problem.
The loader should report the rate it derived from `t_s` so a wrong one is
visible rather than silent.

## Analysis — the part that decides the filter

`analyse.py session.acq`

| Plot | Answers |
|---|---|
| time series, full and zoomed | what does it actually look like? |
| amplitude histogram | offset, clipping, is it railing? |
| power spectral density (Welch) | where is the signal, where is the noise |
| spectrogram | is the noise stationary, or bursts? |
| artifact census | how big are the spikes, how often, **how many samples wide** |

The artifact census sizes the median directly: a window has to exceed twice the
widest artifact to remove it, and stay well under the narrowest feature worth
keeping. Both numbers come out of the recording rather than out of a textbook.

The PSD sets the corners. If mains sits at 50 Hz and the muscle energy runs
20–200 Hz, that is an argument about a notch, not about the band edges.

## Design, apply, verify

`design.py --fs <measured> --hp 20 --lp 450`
: Coefficients plus magnitude, phase, group delay, step and impulse response.
Must **mirror `bio_filter.c` exactly** — RBJ cookbook biquads, Butterworth
pole-pair Qs `1/(2cos(pi/8))` and `1/(2cos(3pi/8))`, high-pass cascade then
low-pass, computed in double and **rounded to float32** as the firmware stores
them. scipy is for plotting and cross-checking only; if scipy generated the
coefficients, comparing against the C would prove nothing.

`apply.py session.acq --hp 20 --lp 450`
: Runs the **full chain including the median** over the recording and plots raw
against filtered, with both spectra. The only way to judge the median, which is
nonlinear and has no frequency response.

`verify.py --fs <measured> --hp 20 --lp 450`
: Compiles `firmware/filters/src/{bio_filter,iir}.c` into a host harness, feeds
it and the Python model identical vectors — impulse, step, chirp, and a slice of
real capture — and reports the maximum divergence. This is what makes the plots
describe the board rather than an idealisation of it.

## Layout

```
filterlab/
  README.md      this file
  filterlab.py   library: load, analyse, design, simulate
  analyse.py     CLI - what is in this recording?
  design.py      CLI - response of a candidate band
  apply.py       CLI - before/after over a recording
  verify.py      CLI - does the C match the model?
  out/           plots, gitignored
```

## Setup

Nothing is installed, and Python 3.12 here is PEP 668 managed, so `pip install`
into the system interpreter is refused.

```bash
sudo apt install python3-numpy python3-scipy python3-matplotlib
#   numpy 1.26.4, scipy 1.11.4, matplotlib 3.6.3

# or, isolated:
python3 -m venv ~/.venvs/filterlab
~/.venvs/filterlab/bin/pip install numpy scipy matplotlib
```

`bioread` is required for `.acq` and is not packaged in apt:

```bash
pip install bioread            # or ~/.venvs/filterlab/bin/pip
```

`verify.py` also needs `gcc`, already present.

## Build order

1. **`filterlab.py` load + `analyse.py`** — nothing else is worth anything
   without a way to look at a recording
2. **`design.py`** — once the analysis says what band to aim at
3. **`apply.py`** — before/after on the same data
4. **`verify.py`** — last, and only if the C is going to be trusted

## Deliberately excluded

- **Chebyshev, elliptic** — sharper skirts for passband ripple or phase
  distortion, and biopotential work values phase linearity more
- **FIR** — linear-phase at 20 Hz and 4.5 kSPS needs hundreds of taps
- **A GUI** — plots to `out/`, arguments on the command line
