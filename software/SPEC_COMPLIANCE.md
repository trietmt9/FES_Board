# Spec compliance - `Biosignal Monitor - Qt Spec.md`

Written 2026-10-05, after the viewer in `emg-viewer/` was rebuilt to the spec. This
lists where the build **departs from or cannot honour** the spec, and why. Anything
not listed was implemented as written (tokens, Inter/Phosphor, header, signal rail,
wave panel, metrics, Wi-Fi dialog, time/frequency, markers, 1200x720 minimum and the
narrow layout, 512-point Hann FFT, ...).

**There was no mockup.** The spec refers to an HTML mockup that is not in the repo.
Where the prose is silent, the result is my reading of it and should be compared
against the mockup if you have it.

## Not measured, so shown as `-`

The firmware does not produce these, and you asked for real data only:

| Spec element | Why it is `-` |
|---|---|
| PR / QRS / QTc intervals | needs fiducial-point delineation; only R peaks (heart rate) are detected |
| Battery level | no fuel gauge or ADC tap on the board |
| Electrode impedance | the ADS1298 lead-off feature is not configured |

## Not implemented

- **Wi-Fi transport.** The dialog has a USB / Wi-Fi switch and a Wi-Fi view (address
  entry, status, device card; see `WIFI_DESIGN.md`), but the only working transports are
  serial and replay. Connect on the Wi-Fi tab validates and remembers the address and
  then says the link is not built yet. Automatic discovery and Rescan are not shown for
  the same reason. The board's Wi-Fi path (nRF7002) has no firmware protocol yet.
- **Settings UI for CLI-only options** (`--window`, `--replay-speed`, ...) - they stay
  command-line flags.

## Deliberate deviations

- **Qt 6.4.2 lacks features the spec assumes.** No `QtQuick.Effects`, so blurs and
  shadows are layered translucent shapes (`Shadow.qml`, `Glow.qml`); no SVG plugin, so
  icons are the Phosphor font; no `font.features`, so tabular figures use a separate
  "Inter Tabular" family. The trace glow is a wider low-alpha stroke - the spec's own
  fallback.
- **GPU drawing, not QPainter.** QPainter took 77 ms per row; plots are a static
  painted layer plus a scene-graph layer (see `CLAUDE.md`). Appearance is the same.
- **Spectrum gain sign is inverted relative to the spec** on purpose: raising the
  gain raises the trace, matching what the time view does.
- **Spectrum decimation** to the nominal 250 / 256 / 1000 Hz rate before the FFT, so
  the Hz axis is right for each signal type.
- **Metrics are floored and gated** - median frequency needs >= 0.03 mV, contraction
  >= 0.12 mV, heart rate is shown only within 50-120 bpm - so noise does not produce
  a confident-looking number.
- **The dialog has two tabs** (USB / Wi-Fi) where the spec has one list, because the
  amplifier is reached by typed address, not by scanning networks.
- **No warning colour.** The spec's palette has none, so "connected but no data" and
  errors are shown in words and by icon (`wifi-medium` vs `wifi-high`), not in red/amber.
- **Diagnostics drawer** (`D`) was added; the spec has none. It shows link statistics.

## Worth knowing

- **Double filtering.** The firmware filters (EMG band-pass + 60 Hz notch) and the
  host applies the spec's display filter on top. Both are present in the signal.
- Mains is **60 Hz** in the host notch.
- **Unverified:** real key presses (1/2/3, F, M, Space, D) - the window manager here
  refuses focus to `xdotool` - and a live serial connection. Everything else was run
  through the real replay path.
- `core/Resample.h` and `core/Spectrum.h` are no longer used by the app (kept for
  their tests).
- The `heartrate` test suite had a failure before the rebuild and still does.
