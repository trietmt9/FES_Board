# EMG Viewer — Software Architecture

Host-side acquisition and visualisation software for the FES_Board 8-channel
biopotential AFE. Qt 6 / QML desktop application plus the firmware-side wire
protocol it speaks.

Status: **implemented and verified against synthetic data, 2026-07-29.** Not
yet run against the board — see section 7 for what remains to be checked on
hardware.

---

## 1. Why this exists

The board acquires biopotential on an ADS1298 (schematic U2) driven by an
STM32F767 (U6), but nothing usable reaches a PC. The sole output path today is a
Zephyr debug print in `firmware/src/main.c:238`:

```c
LOG_INF("block %u  CH%u: %8d code  (%6d uV)%s",
        blocks++, EMG_CHANNEL, code, code_to_uV(code),
        emg_buf.overflow ? "  OVERFLOW" : "");
```

It emits `dsp_buf.processing[EMG_CHANNEL_IDX][0]` — the **first sample of each
64-sample block** — for **one** channel (CH2, hardcoded at `main.c:22`). At the
configured 500 SPS that is **≈7.8 samples/s on one channel**: 98.4 % of acquired
data is discarded before it leaves the MCU, and the remaining channels never
leave at all. It is a bring-up trace, not a data stream.

This document specifies the replacement: a framed binary link carrying **4
channels at 1000 SPS**, and a Qt application that plots, records, replays and
diagnoses it.

## 2. Design constraints (measured, not assumed)

### 2.1 The hardware wires four channels, not eight

The PCB routes exactly four differential pairs — nets `/IN1P`…`/IN4N` — out to
the four 3.5 mm TRS jacks J1–J4. ADS1298 channels 5–8 have **no electrode
inputs**. The system therefore streams 4 channels, and CH5–CH8 remain powered
down (`PDn=1`, MUX = input-shorted, register value `0x81`).

### 2.2 Sample rate is quantised, and 256 Hz is not on the menu

ADS1298 data rates are `fCLK / (4 · N)`. With the internal 2.048 MHz oscillator
the complete set in high-resolution mode is (`drivers/inc/ads129x.h:200-209`):

| `enum dataRate` | HR mode | LP mode |
|---|---|---|
| `fmod_div_16` | 32 kSPS | 16 kSPS |
| `fmod_div_32` | 16 kSPS | 8 kSPS |
| `fmod_div_64` | 8 kSPS | 4 kSPS |
| `fmod_div_128` | 4 kSPS | 2 kSPS |
| `fmod_div_256` | 2 kSPS | 1 kSPS |
| **`fmod_div_512`** | **1 kSPS** | 500 SPS |
| `fmod_div_1024` | 500 SPS | 250 SPS |

There is no 256 Hz setting; producing one would require driving the CLK pin from
an external source. Nor would 256 Hz be desirable: surface EMG occupies roughly
20–450 Hz with dominant power at 50–150 Hz, so a 128 Hz Nyquist would alias the
majority of the band back in-band and render median-frequency and fatigue
metrics meaningless. ISEK and SENIAM both specify ≥1000 Hz for sEMG.

**Design minimum: `fmod_div_512`, high-resolution mode → 1000 SPS**, Nyquist
500 Hz.

**Currently configured: `fmod_div_128` → 4000 SPS** (`src/main.c:75`), chosen
2026-07-29 as deliberate headroom above that minimum rather than a requirement.
The cost is real — see §2.3 — and `EMG_SAMPLE_RATE_HZ` must track the enum,
since it is what every `INFO` frame advertises and the host derives its whole
timebase from it. Drop back to `fmod_div_512` if the acquisition loop ever
starts reporting overflow.

### 2.3 Bandwidth forces a faster link

4 ch × 3 B × 1000 SPS = **12.0 kB/s** of payload. An 8N1 UART at 115200 baud
tops out at 11.52 kB/s, so the old console speed cannot carry even the minimum
rate, before framing overhead.

**Chosen: 921600 baud** (92.16 kB/s). A one-line `current-speed` change in the
DTS, and it works on both the Nucleo bench rig (ST-Link VCP) and the custom
board via a USB-UART.

| Rate | Frame rate | Link | Utilisation | Loop budget/sample |
|---|---|---|---|---|
| 1000 SPS | 31.25 /s | 12.6 kB/s | 13.7 % | 1000 µs |
| **4000 SPS** (current) | **125 /s** | **49.3 kB/s** | **54 %** | **250 µs** |

At 4 kSPS the 27-byte SPI read at 2 MHz (108 µs) consumes ~43 % of the
per-sample budget. Raising `spi-max-frequency` to 4 MHz halves that if headroom
is ever needed; the ADS1298 allows up to 20 MHz.

### 2.4 Scale factors must travel with the data

`code_to_uV()` in `firmware/src/main.c:78-83` implements
`V = code / 2²³ × VREF / gain` with VREF = 2.4 V and gain 6, giving
≈0.0477 µV/LSB and ±400 mV full scale. Meanwhile `firmware/docs/WORKFLOW.md`
still documents gain 12. That drift is exactly why the host must **never**
hardcode the conversion — it reads rate, VREF and gain out of an `INFO` frame.

---

## 3. Data path

```
 electrodes ──► J1..J4 (TRS)          4 differential pairs, /IN1P../IN4N
                  │
                  ▼
            ADS1298  (U2)             HR mode, gain 6, VREF 2.4 V, 1000 SPS
                  │ SPI1 @ 2 MHz, mode 1 (CPOL=0 CPHA=1), DRDY level-polled
                  ▼
            STM32F767 (U6)
              ├─ ads_emg_read_frame() 27-byte RDATAC frame -> int32[4], direct
              ├─ block[4][32]         accumulate one wire frame
              └─ emg_stream_send()    -> TX ring -> UART TX ISR
                  │
                  ▼
       UART @ 921600  (later: USB CDC ACM)
                  │
                  ▼
 ┌────────────────────────────────────────────────────────────────┐
 │ EMG Viewer (Qt 6 / QML)                                        │
 │   IO thread      QSerialPort -> FrameParser -> SPSC rings      │
 │   GUI thread     WaveformItem (QSG) @ 60 Hz, min/max decimate  │
 │   Writer thread  Recorder -> .emgraw / .csv                    │
 └────────────────────────────────────────────────────────────────┘
```

The ADS1298 always clocks out all 8 channels in RDATAC (a 27-byte frame:
3 status bytes then 8 × 3 data bytes), so the driver keeps parsing the full
packet; only CH1–CH4 are transmitted.

---

## 4. Wire protocol (normative)

Shared by `firmware/proto/emg_frame.{h,c}` and
`software/emg-viewer/src/core/FrameParser.{h,cpp}`. **Any change here must land
in both, with the test vectors updated in lockstep.**

### 4.1 Frame header — uniform across all types

6-byte header + payload + 2-byte CRC trailer, i.e. **8 bytes of overhead**.

| Offset | Size | Field | Value |
|---|---|---|---|
| 0 | 2 | `magic` | `0xAA 0x55` |
| 2 | 1 | `type` | `0x01` DATA, `0x02` INFO, `0x03` TEXT |
| 3 | 1 | `ver` | `0x01` |
| 4 | 2 | `len` | payload length, **uint16 little-endian** |
| 6 | *len* | `payload` | |
| 6+*len* | 2 | `crc16` | CRC-16/CCITT-FALSE over bytes `[2 .. 6+len-1]`, **uint16 little-endian** |

CRC parameters: polynomial `0x1021`, init `0xFFFF`, no input/output reflection,
final XOR `0x0000`. Check value for the ASCII string `123456789` is `0x29B1`.

The CRC deliberately covers the header from `type` onward as well as the
payload, so a corrupted `len` cannot silently be accepted.

### 4.2 Why `0xAA 0x55`, and why logs may share the wire

**Both magic bytes are outside the ASCII range.** Zephyr `LOG_*` output is ASCII,
so a log line can never contain the sync pattern. Binary frames and ordinary
console logging therefore coexist on one UART with no escaping, no second port
and no firmware log changes: the host parser emits framed data as samples and
passes everything else through as text to the console pane. This is what makes
`Chip ID = 0x92`, `OVERFLOW` and the 2 s watchdog warning visible in the app
for free.

The converse hazard — a chance `AA 55` *inside* a binary payload — is handled by
validating `type`, `ver` and `len` before trusting a candidate header, then
verifying the CRC. A false sync survives all four checks with negligible
probability, and costs only a resync when it happens.

### 4.3 DATA payload (`type = 0x01`)

| Offset | Size | Field |
|---|---|---|
| 0 | 4 | `seq` — uint32 LE frame counter; host detects loss from gaps |
| 4 | 4 | `t_ms` — uint32 LE, `k_uptime_get_32()` at block close |
| 8 | 1 | `n_ch` (2) |
| 9 | 1 | `n_samp` (32) |
| 10 | 1 | `flags` — bit0 ring overflow, bit1 lead-off |
| 11 | 1 | `ch_mask` — which ADS channels these are; bit0 = CH1 … bit7 = CH8 |
| 12 | `n_samp·n_ch·3` | samples |

**`ch_mask`** was reserved-zero through protocol version 1, and zero keeps that
meaning: the traces are `CH1..CH(n_ch)`, contiguous. A non-zero mask has exactly
`n_ch` bits set and the payload carries them in **ascending channel order**.

This exists because the useful channels are not always the first ones. On the
ADS1298ECG-FE only CH2 (LA−RA, Lead I) and CH3 (LL−RA, Lead II) have both
inputs on a real electrode terminal — CH1 and CH4–CH8 sit against the Wilson
Central Terminal — so the firmware streams mask `0x06`. Without the mask the
host counts up from CH1 and labels Lead I as "V6". Anything that names a
channel must go through `DataFrame::channelIndex()`.

For the custom FES board, where J1–J4 wire four real differential pairs, the
mask is `0x0F` and behaves identically to the old contiguous encoding.

Samples are **sample-major** (`s0c0 s0c1 s0c2 s0c3 s1c0 …`), each a **24-bit
big-endian two's-complement** word copied verbatim in ADS wire order. No
repacking on either side; the host sign-extends bit 23 exactly as
`drivers/src/ads129x.c:149-153` does:

```c
int32_t sample = (p[0] << 16) | (p[1] << 8) | p[2];
if (sample & 0x800000) sample |= 0xFF000000;
```

Sample-major ordering means a truncated frame still yields whole samples, and it
matches the order the ADS produces.

**Budget:** at the configured 2 channels the payload is 12 + 32·2·3 = 204 B and
the frame 6 + 204 + 2 = **212 B**. At 1000 SPS that is 31.25 frames/s →
6.6 kB/s, about 7 % of the 921600 link, block latency 32 ms. Four channels
(mask `0x0F`) doubles the payload to a 404 B frame → 12.6 kB/s (13.7 %).

### 4.4 INFO payload (`type = 0x02`)

Emitted once at boot and once per second thereafter, so an app that connects
mid-stream configures itself within a second.

| Offset | Size | Field |
|---|---|---|
| 0 | 2 | `sample_rate_hz` uint16 LE (1000) |
| 2 | 4 | `vref_uv` uint32 LE (2400000) |
| 6 | 1 | `gain` (6) |
| 7 | 1 | `chip_id` — as read by `ads_read_id()`; `0x92` = ADS1298, `0xD2` = ADS1298R |
| 8 | 1 | `n_ch_active` (4) |
| 9 | 1 | `hr_mode` (1) |
| 10 | 4 | `uptime_s` uint32 LE |
| 14 | 8 | `fw_version` — git short hash, NUL-padded |

Payload 22 B, frame 30 B.

> **Careful with `gain`:** this field carries the *numeric* gain (6), not the
> register enum. In `enum gain` (`drivers/inc/ads129x.h:411-420`) `gain_6 = 0` —
> the enum is not ordered by gain value. Convert explicitly on the firmware side.

### 4.5 TEXT payload (`type = 0x03`)

Optional and still unused. Payload is raw log bytes. v1 relies on the
unframed-ASCII passthrough of §4.2 instead.

Note that §4.2's argument was only ever about the *bytes* — the magic being
outside ASCII means text can sit between frames unescaped. It said nothing about
*writers*, and that gap was a real bug: Zephyr's stock UART backend busy-writes
the data register with `uart_poll_out()` while `emg_stream`'s TX interrupt writes
the same register, so a log line emitted mid-frame landed inside it and cost the
host a whole block to a CRC failure. Fixed in
`firmware/proto/src/emg_log_backend.c`, which emits byte-identical text through
`emg_stream`'s ring so there is exactly one producer and text can only land on a
frame boundary. No protocol change was needed, which is why TEXT stays unused.

---

## 5. Firmware side

`firmware/` is a **separate git repository** nested inside the KiCad repo.

### 5.1 Changes (applied 2026-07-29)

| File | Change |
|---|---|
| `src/main.c:54` | `.data_rate = fmod_div_512` → 1000 SPS |
| `src/main.c:22` | drop the single-`EMG_CHANNEL` model; configure CH1–CH4 live |
| `src/main.c:89` | `power_down_unused_channels()` parks CH5–CH8 only |
| `drivers/inc/ads129x.h:30` | `DSP_BLOCK_SIZE` 64 → **32** (now unused by the app) |
| `drivers/src/ads129x.c` | new `ads_emg_read_frame()`; init buffers made optional |
| `prj.conf` | revert `CONFIG_LOG_MODE_IMMEDIATE=y` to deferred |
| `boards/nucleo_f767zi.overlay` | console USART3 `current-speed = <921600>` |
| `boards/fes/fesboard/fesboard.dts:110` | usart2 `current-speed = <921600>` |
| `CMakeLists.txt` | add `proto/src/emg_frame.c` |
| `proto/{inc,src}/emg_frame.{h,c}` | new — CRC + frame builders (pure C99) |
| `proto/{inc,src}/emg_stream.{h,c}` | new — ring buffer + UART TX ISR |

### 5.2 Timing rules

**`DSP_BLOCK_SIZE` 64 → 32** halves end-to-end block latency from 64 ms to
32 ms. The 256-byte per-channel ring (`ADS129x_BUFFER_SIZE`) still holds 64
`int32` samples, so it keeps two blocks of slack.

**`CONFIG_LOG_MODE_IMMEDIATE` must be reverted to deferred.** It was introduced
as a bring-up aid — it guarantees the last printed line is the last executed
line, which is how the DRDY hang was found — but it busy-writes the UART
synchronously, roughly 6 ms for a 70-character line at 115200. Against a 1 ms
per-sample budget at 1000 SPS that alone starves the acquisition loop.

**Never `uart_poll_out()` a frame from the sample loop.** 404 B at 921600 is
4.4 ms of blocking inside a 32 ms block period. Use a `k_msgq` of frame buffers
plus a dedicated TX thread (or `uart_tx()` with DMA), double-buffered, so
acquisition never waits on the link. Emit `LOG_*` only from that same TX
context, or share its lock, so a log line can never interleave into the middle
of a frame.

**The acquisition path is direct — no ring buffer, no DSP double buffer.**
`ads_emg_read_frame()` sign-extends the 27-byte RDATAC frame straight into an
`int32_t[4]`, which `main()` accumulates into `block[4][32]`; once full, that
block *is* the wire payload. The driver's `ads_emg_read()` + `ads_emg_dsp()`
path still exists for callers that want the driver to own buffering, but the
streaming app does not: routing every sample through a ring buffer and then a
double buffer cost two extra copies and ~3.8 kB of RAM to arrive at identical
numbers, because **nothing in the firmware filters**. `filters/src/iir.c` is not
built and `iir_bandpass()` was never called from anywhere — band-passing happens
on the host (§6.4b), where it is tunable without a reflash.

Measured: RAM 17728 → **13952 bytes**. The wire frame is unchanged at 404 B, so
the host needed no modification.

Single-buffered rather than double: `emg_stream_send_data()` copies the frame
into the TX ring synchronously, so there is no window in which the link is still
reading `block` while acquisition refills it.

Back-pressure is now reported from `emg_stream_dropped()` rather than a ring
overflow flag. Missed *conversions* remain undetectable in firmware — the ADS
provides no sample counter — but they show up on the host as measured SPS below
nominal, which the stats panel already displays.

**Keep DRDY level-polling exactly as it is** (`src/main.c:203`):

```c
if (gpio_pin_get_dt(&ads_rdy) == 1) {
        ads_dev.data_ready = true;
}
```

On this wiring DRDY is held low continuously while a sample is pending and does
not produce the clean falling edges EXTI needs; the edge-interrupt path
deadlocks after the first sample, and reordering the IRQ arming relative to
START does not fix it. Polling the asserted level sidesteps it. Likewise the
START **pin** must be driven low (`GPIO_OUTPUT_INACTIVE`, `src/main.c:139`) or
the START *command* is ignored and no conversion ever runs.

**SPI headroom:** 27 B at 2 MHz (`spi-max-frequency = <2000000>` in both DTS
files) is 108 µs against a 1000 µs sample period — comfortable. The ADS1298
allows 20 MHz, so raising to 4 MHz is available if the loop tightens.

### 5.3 USB CDC ACM (later)

`boards/fes/fesboard/fesboard.dts:160` already enables
`zephyr_udc0: &usbotg_fs` on PA11/PA12, and USB-C J5 carries `/USB_DP` and
`/USB_DN`. The port is `CONFIG_USB_DEVICE_STACK=y` + `CONFIG_USB_CDC_ACM=y` and
pointing the TX thread at the CDC device. **The frame format does not change**
and the host still sees a `/dev/ttyACM*` character device, so the Qt application
needs no modification. The Nucleo overlay needs the OTG FS node added for bench
parity.

---

## 6. Qt application

> **Superseded 2026-10-05.** The application was rebuilt to `Biosignal Monitor - Qt Spec.md`; see `CLAUDE.md` (layout, drawing split) and `SPEC_COMPLIANCE.md`. The wire protocol sections of this file remain normative; this section describes the previous UI.


### 6.1 Toolchain

Verified present on this machine: Qt **6.4.2** at `/usr`, with Qt6Quick,
Qt6QuickControls2, Qt6Widgets, Qt6Charts; CMake 3.28.3; Ninja; g++ 13.3.

**One missing dependency:**

```bash
sudo apt install qt6-serialport-dev     # 6.4.2-4build2, matches the rest of Qt
```

Installed 2026-07-29; `pkg-config --modversion Qt6SerialPort` reports 6.4.2.

### 6.2 Source layout

```
software/emg-viewer/
  CMakeLists.txt                    qt_add_executable + qt_add_qml_module
  src/
    main.cpp
    core/
      SampleTypes.h                 SampleBlock, StreamInfo, ChannelId
      Crc16.{h,cpp}                 CRC-16/CCITT-FALSE (mirrors emg_frame.c)
      FrameParser.{h,cpp}           byte stream -> frames + passthrough text
      SpscRing.h                    lock-free single-producer/single-consumer
    io/
      ISampleSource.h               abstract source interface
      SerialSource.{h,cpp}          QSerialPort on its own QThread
      ReplaySource.{h,cpp}          .emgraw playback, paced from t_ms
      Recorder.{h,cpp}              .emgraw + .csv writer thread
    model/
      StreamController.{h,cpp}      QML-facing facade; owns source + recorder
      ChannelModel.{h,cpp}          per-channel visibility, colour, scale
      LinkStats.h                   Q_GADGET: throughput, errors, measured rate
      LogModel.{h,cpp}              bounded QAbstractListModel of console lines
    ui/
      WaveformItem.{h,cpp}          QQuickItem + QSGGeometryNode renderer
  qml/
    Main.qml  ScopeView.qml  ChannelStrip.qml
    ConnectionBar.qml  StatsPanel.qml  ConsolePane.qml
  tests/
    tst_frameparser.cpp
```

`software/` belongs to the **top-level KiCad repository**; add
`software/*/build/` to the root `.gitignore`.

### 6.3 Threading model

Three threads, no locks on the hot path.

**IO thread.** `SerialSource` owns a `QSerialPort` moved into its own
`QThread`. On `readyRead` it feeds bytes to `FrameParser`, decodes each DATA
frame's 24-bit big-endian words with sign extension, converts to µV using the
scale from the latest `INFO` frame, and writes into a per-channel **lock-free
SPSC ring** (~10 s deep, 10 000 floats × 4). It also hands raw frame bytes to
the `Recorder` queue. It never touches QML or any GUI object.

**GUI thread.** `WaveformItem` runs a 60 Hz timer, reads the newest window out
of the rings, decimates, rebuilds its vertex arrays and calls `update()`.
Because the ring is SPSC and the reader only ever trails the writer, no mutex is
needed and a slow repaint degrades to dropped *frames of video*, never dropped
samples.

**Writer thread.** `Recorder` drains its own queue to disk, so a slow
filesystem or a large flush cannot stall acquisition.

`FrameParser` is deliberately Qt-light — `QByteArray` in, callbacks out, no
event loop, no signals — so it unit-tests standalone and `ReplaySource` reuses
it byte-for-byte. That reuse is what makes replay a genuine regression test of
the live path rather than a parallel implementation.

### 6.4 Rendering

`WaveformItem` is a **`QQuickItem` producing a `QSGGeometryNode`**, not a
`QQuickPaintedItem`. A painted item rasterises on the CPU into an FBO every
frame; the scene graph uploads a vertex array once and lets the GPU draw it.
With 4 channels streaming continuously at 1000 SPS that is the difference
between a smooth trace and a stuttering one.

Decimation is **min/max per pixel column**, not stride-sampling. A 5 s window is
5000 samples across ~1000 px; stride-sampling would drop precisely the
motor-unit spikes that EMG is about, while two vertices per column (the column
minimum and maximum) preserves the true envelope at the same vertex cost.

**Zoomed in**, the opposite problem appears: fewer samples than pixel columns.
Sample-and-hold there draws a visible staircase, and straight chords between
samples undershoot every peak. The renderer instead performs **band-limited
(Whittaker–Shannon) reconstruction** — `src/core/Resample.h` — which draws the
unique continuous curve the samples imply. The sampling theorem guarantees that
curve exists and is unique, and the ADS1298's sinc³ decimation filter
band-limits the signal before it ever reaches us, so the precondition genuinely
holds.

This is *not* a literal FFT. Frequency-domain resampling (FFT → zero-pad →
inverse FFT) is the same reconstruction mathematically, but it assumes the
window is periodic; on a scrolling scope the left and right edges are unrelated,
so that assumption wraps one edge into the other and rings at both ends of every
repaint. A Lanczos-windowed sinc applied in the time domain has no edge
coupling, needs no transform, and is what instrument and audio software actually
use. It is toggleable from the UI (*Smooth*) and inactive when zoomed out, where
min/max decimation is correct and interpolating would alias.

### 6.4a Display freeze

A **Pause / Play** button (and the space bar, the scope convention) freezes the
display. Acquisition, recording and the link statistics all carry on — only the
rendering stops. In replay mode it also pauses the source, since letting it run
past a frozen view just discards capture the user wanted to look at.

Freezing takes a **snapshot** of the rings rather than remembering a read
position: the producer keeps writing while frozen, and within one ring capacity
(10 s) it would overwrite the very samples being examined.
`SampleRing::snapshotInto()` copies the buffer and write index; the newest sample
or two can tear, which is invisible in a frozen trace and not worth a lock on the
acquisition path.

The render items **keep repainting while frozen, at 1/12 rate**, rather than
stopping. Stopping leaves the item permanently clean, and the next thing that
invalidates the scene graph — a resize, a re-expose, `grabWindow()` — renders it
blank with nothing to restore it. That was observed, not theorised.

The frozen state is signposted on the plot (*"PAUSED — display only, still
receiving"*), because a frozen trace and a dead board look identical otherwise
and confusing the two costs real debugging time.

### 6.4b ECG mode

The viewer handles ECG as well as EMG, selected by **signal mode** in the Signal
panel. This is not cosmetic — the band is the whole difference:

| Mode | Band | Why |
|---|---|---|
| **EMG** | 20–450 Hz | SENIAM surface EMG |
| **ECG monitor** | 0.5–40 Hz | bedside monitoring; rejects drift hard |
| **ECG diagnostic** | 0.05–150 Hz | IEC 60601-2-25; preserves the ST segment |

**Using the EMG preset on an ECG destroys it.** A 20 Hz high-pass sits directly
on the QRS complex and removes the P wave, the ST segment and most of the T wave.
Conversely the 0.05 Hz diagnostic corner passes every bit of electrode drift,
which is why monitoring mode exists.

**Heart rate** comes from `src/core/HeartRate.cpp`, a reduced Pan-Tompkins:
band-pass 5–15 Hz → derivative → square → 150 ms moving integration → adaptive
threshold → 200 ms refractory, with the rate taken as the *median* of the last
8 RR intervals so one missed beat cannot lurch the readout.

Three details that took getting wrong to find:

- **Level estimates update from an excursion's peak when it ends, never per
  sample.** Updating continuously makes the threshold follow the signal down, so
  it can never fall below it — the detector latches after one beat and reports
  nothing further.
- **A settling period precedes the learning phase.** A DC-coupled channel starts
  with millivolts of electrode offset; that step through the 5 Hz high-pass rings
  far longer than any QRS, and learning through it sets the threshold absurdly
  high.
- **The signal-level decay has a floor, and the rate is gated on signal-to-noise
  separation.** Without them, the self-heal that recovers from an artifact keeps
  going until the threshold reaches the noise floor and the detector cheerfully
  reports ~270 BPM from pure noise.

The detector is fed the **raw** signal and runs its own band-pass, so it is
unaffected by the display filter — including an EMG preset that would gut an ECG.

In ECG mode the channel strips show **rate (BPM)** and **peak-to-peak in mV** —
the two numbers you check against a simulator, whose settings are known in
advance. That makes an ECG simulator the best end-to-end validation available:
unlike EMG, the correct answer is known before you start.

### 6.4c Frequency domain

A **Time / Frequency** toggle switches the plot between `WaveformItem` and
`SpectrumItem`. They are separate items rather than one item with a mode: the
axes (Hz vs seconds, dB vs microvolts), the reduction (averaged periodograms vs
min/max decimation) and the update cost all differ, and folding both into one
`updatePaintNode` would be a bigger tangle than the lane-layout code they share.

The spectrum is computed by **Welch's method** — `src/core/Spectrum.cpp`, over
`src/core/Fft.cpp` — not a single FFT. EMG is a stochastic signal, so one
periodogram has roughly 100 % variance per bin and plots as a hedge of noise no
matter how long the record; splitting into Hann-windowed segments at 50 %
overlap and averaging the power is what makes it readable. Scaling is
coherent-gain normalised, so a sine of amplitude *A* µV reads back at *A*
(20·log₁₀ A dB re 1 µV) regardless of window or transform length.

The mean is removed before transforming — a gain-6 channel sits on millivolts of
electrode offset that would otherwise dominate the DC bin and leak across the
low end, swamping the very band the median frequency is taken over.

Three markers earn their place on the axis:

- **50 and 60 Hz** (red, dashed) — mains. A spike here is the single most
  common real fault on a biopotential front end: it means the electrodes or the
  ground reference are picking up the room, not the muscle.
- **20 and 450 Hz** (green, dashed) — the SENIAM surface-EMG band, over which
  the median frequency is computed. Restricting to that band matters: below
  20 Hz is motion artifact and electrode drift, above 450 Hz there is no EMG
  left, and including either would move the median for reasons unrelated to the
  muscle.

In frequency mode the channel strips swap their last/RMS readouts for **median
and peak frequency**. Median frequency is the classic surface-EMG fatigue index
— it falls as a muscle tires — and is the main reason to open this view.

The FFT is a small self-contained radix-2 Cooley-Tukey, not a library: the
transforms here are 256–4096 points on four channels at 10 Hz refresh, a few
hundred microseconds of work, so FFTW or KissFFT would cost more in build and
packaging than they save. That trade flips if the analysis ever grows to
spectrograms, long averages, or non-power-of-2 sizes.

**Note that this is unrelated to the interpolation above.** A Fourier transform
shows what frequencies a signal contains; it cannot make sampled data
continuous. Reconstruction between samples is `core/Resample.h`; frequency
analysis is `core/Spectrum.h`.

### 6.4d Conditioning — why the band-pass is mandatory, not optional

Originally deferred as "v1 polish". That was wrong, and hardware proved it: with
no filter the muscle signal is invisible and the envelope is meaningless.

The ADS1298ECG-FE input network is **DC-coupled as populated** — the AC-coupling
capacitors (C87, C25, …) are Not Installed and JP6–JP14 default to pins 1-2
shorted — so there is no high-pass anywhere in hardware. Straight off the ADC a
channel therefore carries millivolts of electrode half-cell offset, sub-hertz
drift from electrode settling and movement, and whatever mains the leads pick
up — all one to three orders of magnitude larger than the 50–500 µV of muscle
underneath. Consequences, both observed:

- autoscale keys off the window peak, so drift sets the range and the burst
  compresses to a thickening of the line;
- a window-wide RMS is a measurement of the drift, not of the muscle.

`src/core/EmgFilter.cpp` runs, per channel: 4th-order Butterworth high-pass at
20 Hz → 4th-order Butterworth low-pass at 450 Hz (the SENIAM band) → optional
mains notch at 50 or 60 Hz plus its second harmonic → sliding RMS envelope,
default **100 ms**.

**The chain always runs; `FilterConfig::enabled` selects only what the display
shows.** The envelope is taken from the filtered signal either way, because an
RMS that includes electrode drift is a measurement of the artifact rather than
the muscle. So viewing the raw ADC output and having a meaningful activity
reading are not in conflict — and the display defaults to **raw**, because
during bring-up you want to see what the converter actually produced before any
host processing. (An earlier version bypassed the chain entirely when disabled,
which silently sent the envelope back to measuring drift; the comment claimed
otherwise and the code did not match it.)

Envelope window is selectable (50 / 100 / 250 / 500 ms). 250 ms is the textbook
sEMG constant but feels sluggish live; 100 ms still averages ~10 cycles of the
dominant 100 Hz content. Note the envelope divides its running sum by the number
of *filled* slots, so it is valid from the first sample — which means the window
length governs the response to change, not the startup ramp. Tests measure it as
a step response with the window already full for exactly that reason.

Surface EMG legitimately looks like band-limited random noise: it is a
stochastic interference pattern from asynchronously firing motor units, with no
repeating morphology. The information is in the envelope, not the waveform.

Fourth order rather than second on the high-pass because drift, not noise, is
the enemy: at 0.5 Hz a 4th-order 20 Hz high-pass is ~128 dB down versus ~64 dB
for a 2nd-order.

**Filtering runs in the IO thread, on the stream** — not per-repaint over a
display window, which would restart the IIR transient every frame and make the
envelope meaningless. The controller keeps **two rings**: raw and filtered. The
waveform reads the filtered one; the spectrum always reads **raw**, because a
spectrum with the notch already applied hides the mains spike you opened it to
find.

The 250 ms envelope constant is the other half of the fix. The previous readout
averaged over the whole display window, so a one-second contraction inside a
five-second window barely moved it.

Also worth knowing: **the ADC's own sinc³ decimation filter is −3 dB at
0.262·f_DR**, so below about 1720 SPS it rolls off inside the sEMG band and no
host filtering can recover what it removed. The UI warns when that is the case.

### 6.5 Features

**Connection bar.** Ports enumerated with `QSerialPortInfo`; baud defaults to
921600. Channel count, sample rate, gain, VREF and chip ID are displayed
read-only, sourced from the `INFO` frame — never from a constant in the app.

**Recording.** Two formats, independently selectable:

- `.emgraw` — the verbatim frame stream. Lossless, replayable, and the format
  the app's own replay path consumes.
- `.csv` — `t_s,ch1_uv,ch2_uv,ch3_uv,ch4_uv` for Python/MATLAB analysis.

**Replay.** `ReplaySource` reads `.emgraw` through the identical `FrameParser`,
paced from the embedded `t_ms`, with play / pause / seek / speed. The entire UI
and DSP chain can be developed and demonstrated with no board attached.

**Console and link statistics.** `LogModel` holds the unframed ASCII the parser
passes through — banner, `Chip ID = 0x..`, `OVERFLOW`, the 2 s watchdog warning.
`LinkStats` reports bytes/s, frames/s, CRC failures, resyncs, dropped frames
(from `seq` gaps) and **measured versus nominal sample rate**. Given the
DRDY/START problems already encountered on this hardware, that panel is the
first place to look when a capture goes wrong.

---

## 7. Verification

**Spectrum tests** (`tst_spectrum`) — the FFT against the O(n²) DFT definition,
an impulse transforming flat, Parseval's theorem, and forward/inverse round-trip;
then the spectrum against closed-form answers: a sine peaks at its own frequency
and its own amplitude, a 5 mV DC offset changes nothing, averaging measurably
reduces variance versus fewer segments, and the median frequency tracks where
the band power actually sits. This caught a real defect — a broken
`floorPowerOfTwo` that silently pinned every transform to 256 points.

**Reconstruction tests** (`tst_resample`) — that the interpolator is *correct*,
not merely smooth: unit DC gain, exact pass-through at original sample instants,
a reconstructed sine within 5 % of truth at 0.31 of Nyquist, and at least a 2×
error reduction versus linear interpolation on band-limited input.

**Parser unit tests** (`ctest`) — the failure modes that actually occur:

- a well-formed DATA frame decodes to the expected µV values;
- a single flipped bit is rejected by CRC;
- a frame split across two `readyRead` chunks reassembles;
- ASCII log text interleaved between frames passes through intact and in order;
- a planted `AA 55` inside a payload does not cause a false sync;
- a `seq` gap is counted as a drop;
- leading garbage is discarded and the parser resyncs on the next valid frame.

**Throughput.** Replay a synthetic 60 s / 1000 SPS / 4 ch capture and assert
zero dropped samples and zero ring overflows with the UI rendering at 60 fps.

**Scale correctness.** Feed the codes `0x7FFFFF`, `0x000000` and `0x800000` and
check the app's µV against the firmware formula `V = code/2²³ × VREF/gain`
(VREF 2.4 V, gain 6, `src/main.c:78-83`): full scale must land at ±400 mV.

**Hardware bring-up** on the Nucleo rig:

```bash
source ~/zephyrproject/.venv/bin/activate
export ZEPHYR_BASE=~/zephyrproject/zephyr
export ZEPHYR_SDK_INSTALL_DIR=~/zephyr-sdk-0.17.4
cd firmware && west build -b nucleo_f767zi . && west flash

stty -F /dev/ttyACM0 921600 raw -echo
xxd /dev/ttyACM0 | head          # expect aa55 01 01 ... frames
```

Then, with the app attached: short an input and confirm a flat trace at the
noise floor; touch the electrodes and confirm a burst on the correct channel;
confirm the measured rate reads 1000 SPS and that CRC errors and `seq` gaps both
stay at zero over a 5-minute soak; verify the J1→CH1 … J4→CH4 mapping one jack
at a time.

---

## 8. Deferred

- ~~Host DSP~~ — **done** (§6.4b). It turned out to be a prerequisite for seeing
  EMG at all, not polish. `firmware/filters/src/iir.c` remains compiled and
  uncalled; keeping DSP on the host leaves the firmware a dumb, fast pump and
  makes the filter tunable without a reflash.
- **USB CDC ACM transport** (§5.3) — no host-side change.
- **Lead-off detection** — `flags` bit 1 is allocated; the ADS1298's LOFF
  registers are not yet configured, and the RDATAC status word (which carries
  `LOFF_STATP`/`LOFF_STATN`) is currently parsed away and discarded in
  `ads_emg_read()`.
- **IMU (U5, ICM-42688-P)** — the SPI2 node is `status = "disabled"` pending
  firmware; a second stream type would slot in as `type = 0x04`.

## 9. Open risks

- **Channel-to-jack mapping is unverified on the FES_Board.** `/IN1P`…`/IN4N`
  exist on the PCB, but nothing has confirmed which ADS input each TRS jack
  lands on. Verify per jack on hardware before trusting the channel labels.

  The UI therefore carries an explicit **board selector** rather than assuming.
  On the **ADS1298ECG-FE** the mapping is documented and quite unlike the FES
  board's — SBAU171 Table 2 distributes ten electrodes across eight channels:

  | Channel | Lead | Channel | Lead |
  |---|---|---|---|
  | 1 | V6 − WCT | 5 | V3 − WCT |
  | **2** | **LEAD I = LA − RA** | 6 | V4 − WCT |
  | 3 | LEAD II = LL − RA | 7 | V5 − WCT |
  | 4 | V2 − WCT | 8 | V1 − WCT |

  So a two-electrode EMG pair in the RA and LA jacks appears on **channel 2**,
  and channels 1 and 4–8 sit against the Wilson Central Terminal with nothing
  attached. Labelling those "J1…J4" was actively misleading, which is why the
  scheme is now selectable.
- **Log/frame interleaving.** The log backend writes to the same UART through
  its own path, so a `LOG_*` emitted mid-frame can still tear one. The parser
  treats that as routine — CRC reject, resync, counter — and the console text
  survives intact, but a chatty log will cost frames. If that becomes a problem,
  route logging into `TEXT` frames (§4.5) or move the console to RTT.
- **On the custom board the console UART is shared with the WT02C40C module**
  (`fesboard.dts:27`, usart2 on PA2/PA3). Streaming there needs either an
  external USB-UART on those pins or the USB CDC port. The Nucleo bench rig has
  no such conflict; do bring-up there.
