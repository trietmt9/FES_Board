# EMG Viewer

Real-time acquisition and visualisation for the FES_Board: 4 channels of surface
EMG at 1000 SPS, over a framed binary link from the STM32F767.

Design rationale and the normative wire protocol are in
[`../ARCHITECTURE.md`](../ARCHITECTURE.md).

## Build

Needs Qt 6.4+, CMake 3.21+, a C++17 compiler.

```bash
sudo apt install qt6-base-dev qt6-declarative-dev qt6-serialport-dev cmake ninja-build

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Binaries land in `build/bin/`.

## Run

```bash
# Live, from the board
./build/bin/emg-viewer --port ttyACM0 --baud 921600

# Or just launch and pick the port in the UI
./build/bin/emg-viewer

# Replay a recorded capture — no hardware needed
./build/bin/emg-viewer --replay session.emgraw
```

## Test

```bash
cd build && ctest --output-on-failure
```

Seven suites:

- **`frameparser`** — compiles `firmware/proto/src/emg_frame.c` *directly* and
  decodes its output with the host `FrameParser`. A CRC or endianness
  disagreement between firmware and host is otherwise invisible until you are
  staring at corrupt data from real hardware. Also covers torn frames,
  interleaved log text, false sync patterns planted inside payloads, and resync
  after corruption.
- **`emgfilter`** — that the conditioning chain does what it is for: flat across
  20–450 Hz, >60 dB down on 0.5 Hz drift, DC fully removed, mains notched, and an
  envelope that rises within 400 ms of a contraction. Plus the composite case:
  100 µV of EMG recovered to within 10 % from 5 mV offset + 200 µV drift + 40 µV
  mains.
- **`heartrate`** — QRS detection against synthetic ECG at 30–180 BPM, under
  5 mV offset plus drift, mains and noise; that it counts the right number of
  beats, does not double-trigger on the S wave, follows a rate change, and
  reports *nothing* rather than inventing a rate from noise.
- **`spectrum`** — the FFT against the naive DFT, Parseval, and inverse
  round-trip; then the Welch spectrum against signals with closed-form answers
  (a sine peaks at its own frequency and amplitude, DC offset is rejected,
  averaging reduces variance, median frequency tracks band power).
- **`resample`** — that band-limited reconstruction is *correct*, not just
  smooth: unit DC gain, exact pass-through at sample instants, within 5 % of a
  true sine at 0.31 of Nyquist, and ≥2× better than linear interpolation.
- **`replay`** — 60 s of 1000 SPS × 4 ch through the same decode path the live
  link uses, asserting zero loss; plus injected drops and corruption, and the
  recorder's `.emgraw`/`.csv` round trip.

The tests need `firmware/` checked out beside `software/`. Without it they are
skipped and the app still builds.

## Synthetic data

`emg_gen` writes a capture using the firmware's own encoder, so the bytes are
identical to what the board emits:

```bash
./build/bin/emg_gen session.emgraw 30              # 30 s, 4 ch, 1000 SPS
./build/bin/emg_gen bad.emgraw 30 --drop 50        # lose a frame every 50
./build/bin/emg_gen bad.emgraw 30 --corrupt 100    # flip a bit every 100
./build/bin/emg_gen hum.emgraw 30 --hum 40         # add 40 uV of 50 Hz mains
./build/bin/emg_gen hidden.emgraw 25 --drift 900   # bury the EMG under drift
./build/bin/emg_gen fast.emgraw 20 --rate 4000     # match the firmware's rate
```

`--hum` is for checking the frequency view: the spike must land exactly on the
50 Hz marker, and every channel's peak readout must say 50 Hz.

The fault-injection flags are for exercising the viewer's error reporting —
dropped-frame and CRC counters should track exactly what was injected.

## Freezing the display

**Pause / Play** in the scope toolbar, or the **space bar**, freezes the trace so
you can read it. The board stays connected, samples keep arriving, and any
recording in progress carries on — only the drawing stops. A badge on the plot
says so, because a frozen trace and a dead link look the same otherwise.

In replay mode it also pauses playback.

## ECG

Set **Signal → ECG monitor** (0.5–40 Hz) or **ECG diagnostic** (0.05–150 Hz).
The EMG preset's 20 Hz high-pass sits on the QRS and will destroy an ECG, so the
mode matters. In ECG mode the channel strips show heart rate and peak-to-peak
amplitude instead of the envelope.

Test it without hardware:

```bash
./build/bin/emg_gen ecg.emgraw 25 --rate 4000 --ecg --bpm 75 --mv 1.0
./build/bin/emg-viewer --replay ecg.emgraw
```

With an ECG simulator this is the best end-to-end check you have — the rate and
amplitude are known in advance, so the readouts either match or they don't.

## Recording formats

| Extension | Contents |
|---|---|
| `.emgraw` | The verbatim byte stream, log text included. Lossless, and the format `--replay` consumes. |
| `.csv` | `t_s,ch1_uv,ch2_uv,ch3_uv,ch4_uv` for Python/MATLAB. |

Microvolt scaling is never hardcoded: it comes from the `INFO` frame the
firmware emits every second, so a capture stays correct even if the gain or
reference changes.

## Layout

```
src/core/   protocol decode (Crc16, FrameParser), lock-free SampleRing
src/io/     SerialSource, ReplaySource, Recorder — all off the GUI thread
src/model/  QML-facing controller and list models
src/ui/     WaveformItem — QQuickItem + QSGGeometryNode renderer
qml/        the interface
tools/      emg_gen, the synthetic capture generator
```

`core` and `io` build into `emg_core`, which has no QML or QtQuick dependency —
that is what lets the tests run headlessly.

## Notes

- **Channel-to-jack mapping is unverified.** The UI labels channels J1–J4 after
  the schematic's intent; nobody has confirmed on hardware which ADS1298 input
  each 3.5 mm jack actually lands on. Check per jack before trusting the labels.
- **The display defaults to raw**, but the envelope is always computed from the
  filtered chain (20–450 Hz Butterworth + 50/60 Hz notch), so you see the real
  ADC output *and* get a meaningful activity reading. *Show filtered* in the
  Signal panel switches the trace. Recorded `.emgraw` is always raw.
- **Surface EMG looks like random noise — that is correct.** It is a stochastic
  interference pattern with no repeating morphology; a contraction is a burst of
  noise whose amplitude tracks effort. Read the envelope, not the waveform.
- Envelope window is selectable (50–500 ms). 100 ms is the default: responsive
  without being jumpy.
- The spectrum view is **always unfiltered**, deliberately — that is where you
  diagnose mains pickup and drift.
- `--grab <file>` renders one frame to PNG and exits, for documentation shots
  and CI. `--window <seconds>` sets the initial time span.
- The **Time / Frequency** toggle switches between the scrolling waveform and a
  Welch-averaged amplitude spectrum. In frequency mode the channel strips show
  median and peak frequency instead of last and RMS; median frequency is the
  standard surface-EMG fatigue index.
