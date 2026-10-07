# Biosignal Monitor — UI Specification for Qt

Source mockup: `Biosignal Monitor.dc.html`
Target: desktop monitor, clinicians in a hospital. Dark theme. Live ECG / EEG / EMG display with a time ⇄ frequency domain switch.

Recommended stack: **Qt 6 + QML (Qt Quick)** for the UI, **Qt Quick Shapes / QSGGeometry or QQuickPaintedItem** for traces, C++ backend for acquisition + FFT. A Qt Widgets version is also possible (see §10).

---

## 1. Design tokens

### 1.1 Colors

| Token | Hex | Use |
|---|---|---|
| bg | `#161826` | Window background |
| surface | `#232532` | Panels, cards, dialog |
| text | `#E9E9ED` | Primary text |
| divider | `#E9E9ED` @ 16% alpha (`#29E9E9ED` ARGB) | Rules, seg borders |
| accent | `#9184D9` | Traces glow, active outlines, kicker text, icons |
| accent-200 | `#E7E5FE` | Trace head dot, marker label text |
| accent-300 | `#D2CEFD` | Accent text at small sizes, peak label |
| accent-400 | `#B5ABFC` | Trace stroke |
| accent-600 | `#796CBF` | — |
| accent-700 | `#5D5294` | Selected-card outline |
| accent-800 | `#423A6A` | Accent tag fill, marker label fill |
| accent-900 | `#2B2741` | Selected-card fill, background glow |
| neutral-200 | `#E4E7F5` | Kbd text |
| neutral-300 | `#CFD3E5` | Clock, secondary values |
| neutral-400 | `#B2B6CA` | Header meta text, inactive icons |
| neutral-500 | `#9397AB` | Muted labels, axis text |
| neutral-600 | `#75798C` | Freq axis labels, inactive band bars |
| neutral-700 | `#595D6C` | Time-grid center line, kbd outline |
| neutral-800 | `#3F424D` | Major grid lines, neutral tag fill, card hairline |
| neutral-900 | `#292B31` | Minor grid lines, icon tiles, bar tracks, hover fill |

Rules: no pure black/white; accent is used as line + glow, never as a large fill. Paragraph-size accent text uses accent-300.

### 1.2 Typography
- Font: **Inter** (bundle Inter-Regular/Medium .ttf via `QFontDatabase::addApplicationFont`). Weights 400 and 500 only — never bold.
- Base: 13 px. Small labels 11 px. Kicker 10 px, uppercase, letter-spacing 0.1 em, accent color.
- Panel title 17 px / 500. Dialog title 18 px / 500.
- Hero metric 56 px / 500, letter-spacing −0.02 em, tabular figures. Secondary metric 20 px / 500.
- All numeric readouts use tabular figures (`font.features: { "tnum": 1 }` in Qt 6.6+, or a monospaced-digit fallback).

### 1.3 Spacing / radius / elevation
- Spacing scale (px): 2.8, 5.6, 8.4, 11.2, 16.8, 22.4 → use 3, 6, 8, 11, 17, 22.
- Radius: sm 4, md 8, lg 14.
- Elevation: sm = 1 px border neutral-800; md = 1 px neutral-700 + shadow 0 6 18 rgba(0,0,0,.55); lg (dialog) = 1 px neutral-500 + shadow 0 16 40 rgba(0,0,0,.65).
- Horizontal rules fade out at both ends over 48 px (linear gradient transparent → divider → divider → transparent).

### 1.4 Interaction states
- Hover: 7% text-color tint (secondary/ghost) or 12% accent tint (primary).
- Pressed: 14% text tint / 22% accent tint.
- Keyboard focus: 2 px accent outline, 2 px offset.
- Disabled: 45% opacity.

### 1.5 Icons
Phosphor Icons (regular weight). Bundle SVGs or the Phosphor icon font. Names used: `pulse, user, wifi-high, wifi-medium, wifi-low, wifi-slash, caret-down, battery-high, record, heartbeat, brain, hand-fist, wave-sine, chart-line, flag, pause, play, x, arrow-clockwise`.

---

## 2. Window layout

Minimum window: 1200 × 720 (designed for 1440 × 900+).

```
┌──────────────────────────────── Header (52 px) ────────────────────────────────┐
│ [logo] BioView Monitor   👤 Patient 0042-118 / Session 03     [Wi-Fi ▾] 🔋82% 12:04:31 [● Record] │
├──────────────┬─────────────────────────────────────────────┬───────────────────┤
│ Signal rail  │ Wave panel (fills)                          │ Metrics panel     │
│ 232 px       │                                             │ 272 px            │
└──────────────┴─────────────────────────────────────────────┴───────────────────┘
```
- Body padding 17 px, column gap 17 px.
- Background: bg plus a soft radial glow of accent-900 from the top-left corner (1200×600 ellipse, fading to transparent at 60%).
- Header bottom edge: faded rule (§1.3).
- Below 1200 px width: columns become `200 px | fill`, metrics panel moves below as a wrapping row of cards (min 240 px each), window scrolls vertically, wave panel min-height 620 px.

---

## 3. Header

Left → right, vertically centered, gap 17 px:
1. **Logo tile** 26×26, radius 4, 1 px accent inner outline, `pulse` icon 16 px accent. Text "BioView" (14 px/500) + "Monitor" (neutral-600).
2. **Patient context** (neutral-400): `user` icon, "Patient 0042-118", "/" (neutral-700), "Session 03".
3. Spacer.
4. **Wi-Fi button** (secondary style, 32 px tall): status icon (accent when connected, neutral-500 otherwise) + label (single line, max 140 px, elide right) + `caret-down` 11 px. Labels: connected → device name; connecting → "Connecting…"; off → "Connect device". Opens the Wi-Fi dialog (§8).
5. Battery: `battery-high` + "82%".
6. Clock HH:MM:SS (neutral-300, tabular), 1 s update.
7. **Record button** (primary outline, 32 px tall, min-width 118): idle → `record` icon + "Record"; recording → pulsing 7 px dot (accent-300, opacity 1→0.25→1 over 1.2 s) + "Stop · mm:ss".

---

## 4. Signal rail (left)

- Kicker "SIGNAL TYPE" (neutral-500).
- Three selectable cards (one signal shown at a time, exclusive selection):

| Id | Title | Meta line | Icon | Live value (right) |
|---|---|---|---|---|
| ecg | ECG | 3 leads · 250 Hz | heartbeat | `72 bpm` |
| eeg | EEG | 4 channels · 256 Hz | brain | `10.2 Hz` |
| emg | EMG | 2 channels · 1 kHz | hand-fist | `0.42 mV` |

- Card: grid `32 px icon tile | text | value`, padding 10×8, radius 8. Icon tile 32×32 neutral-900, radius 4.
- Selected: fill accent-900, 1 px accent-700 inner outline + soft accent glow (0 0 24 −8); icon accent. Unselected: transparent, icon neutral-400, hover neutral-900.
- Bottom of rail: **Shortcuts** box (hairline sm elevation): `1–3` Signal type · `M` Add marker · `F` Time / frequency · `Space` Freeze. Kbd chips: 10 px/500, 1 px neutral-700 outline, radius 4.

---

## 5. Wave panel (center)

Container: surface → 55% surface/bg vertical gradient, radius 14, sm elevation.

### 5.1 Toolbar row 1
- Left: kicker "TIME DOMAIN · ECG · 250 Hz sampling" (or "FREQUENCY DOMAIN · …"), title "Electrocardiogram" / "Electroencephalogram" / "Electromyogram".
- Right: **Domain switch** — segmented control, 34 px tall, two options with icons: `wave-sine` "Time domain", `chart-line` "Frequency domain". Selected option: accent text + 1 px accent inner outline. Shortcut F toggles.

### 5.2 Toolbar row 2
- Filter tags (neutral tag: neutral-800 fill, neutral-100 text, 11 px, radius 6):
  - ECG: HP 0.5 Hz · LP 40 Hz · Notch 50 Hz
  - EEG: HP 0.5 Hz · LP 45 Hz · Notch 50 Hz
  - EMG: HP 20 Hz · LP 450 Hz · Notch 50 Hz
- Spacer.
- Time mode: **Window** segmented `2.5 s | 5 s | 10 s` (default 5 s).
- Frequency mode: **Averaging** segmented `Low | Med | High` (default Med).
- **Gain** segmented `×0.5 | ×1 | ×2` (default ×1).
- **Marker** button (secondary, `flag` icon + "Marker"). Shortcut M.
- **Freeze** icon button 30×30 (`pause` / `play`). Shortcut Space.

Segmented controls: 1 px divider border, radius 8, options 12 px text, never wrap or clip labels.

### 5.3 Channel rows
Rows split the remaining height equally. Each row: `96 px label column | plot`, faded rule at the bottom.

Label column: channel name (14 px/500), location (11 px neutral-500), scale (10 px accent-300, tabular): time mode "±1.6 mV" / "±70 µV" divided by gain; freq mode "dB · rel".

| Signal | Rows (name — location) |
|---|---|
| ECG | I — Limb lead · II — Limb lead · V1 — Precordial |
| EEG | Fp1 — Frontal · C3 — Central · O1 — Occipital · O2 — Occipital |
| EMG | Biceps — Right arm · Triceps — Right arm |

### 5.4 Footer
`96 px | plot` grid, 11 px neutral-500:
- Time mode: left "Scroll" / "Sweep" / "Frozen"; right evenly spaced time ticks (2.5 s → 0.5 s steps, 5 s → 1 s, 10 s → 2 s), e.g. "0 s 1 s 2 s 3 s 4 s 5 s".
- Freq mode: left "Hz"; right "FFT 512 pt · Hann · Δf 0.49 Hz" (ECG), "Δf 0.50 Hz" (EEG), "Δf 1.95 Hz" (EMG).

---

## 6. Plot rendering

### 6.1 Time domain
- Grid: vertical minor lines every 0.1 s (2.5 s window) / 0.2 s (5 s) / 0.4 s (10 s), every 5th is major. Minor neutral-900, major neutral-800, 1 px. Horizontal: 8 divisions, center line neutral-700, even lines neutral-800, odd neutral-900.
- Vertical scale: half-height = RANGE / gain, RANGE = ECG 1.6 mV, EEG 70 µV, EMG 1.6 mV.
- Trace: accent-400, 1.4 px, round joins; optional glow = accent, 6 px blur (toggle).
- Display mode (setting):
  - **Scroll** (current preference): newest sample at the right edge, history scrolls left.
  - **Sweep**: x = (t mod W)/W; erase gap of 3.5% of the window ahead of the head with a fading accent band (18% → 0% alpha).
- Head: 2.6 px dot, accent-200, 10 px accent glow.
- Decimation: when samples > 2 × pixel width, use min/max per bucket (alternate) so EMG spikes are kept.
- Markers: dashed vertical line (3/3), accent-300 at 70%; label chip "M1" on the top row only (accent-800 fill, accent-200 10 px text).

### 6.2 Frequency domain (FFT spectrum line)
- FFT: N = 512, Hann window, mean removed, power = |X|²/N; recompute about every 5 frames (~12 Hz).
- Display range: ECG 0–40 Hz, EEG 0–45 Hz, EMG 0–450 Hz.
- Averaging: exponential, new-frame weight Low 0.6 / Med 0.3 / High 0.12.
- Y axis in dB: top = smoothed max (0.92/0.08 EMA) + 4 dB + 20·log10(gain), 48 dB span. Plot area: 8 px top, 18 px bottom for labels.
- Grid: vertical every 5 Hz (EMG 50 Hz), major every 2nd step, labels on major steps at the bottom ("0 Hz", "10", "20"…), neutral-600 10 px. Horizontal: 4 divisions, neutral-900.
- Spectrum: area under the curve filled with accent at 32% → 0% vertical gradient, line accent-400, 1.4 px, optional glow. (Alternative setting: bars.)
- EEG only: shaded band columns δ 0.5–4, θ 4–8, α 8–13, β 13–30, γ 30–45 Hz (accent at 7% / 3.5% alternating), with Greek labels top-left in neutral-500.
- Peak marker: largest bin above 0.8 Hz (EMG: above 20 Hz). 3 px accent-200 dot with glow + label "10.2 Hz" in accent-300 on a neutral-900 85% chip.

---

## 7. Metrics panel (right)

Update every 400 ms. Cards: surface fill, radius 8, padding 17.

**ECG**
- Hero card: kicker "HEART RATE", value "72" + "bpm", tag "Sinus rhythm" (accent tag), "Limits 50–120".
- 2×2 grid: RR interval (live, 60000/HR ms), PR interval, QRS duration, QTc (ms).

**EEG**
- Hero: "DOMINANT FREQUENCY · O1", value in Hz, tag "<Band> band".
- Relative band power: 5 rows (Greek symbol in accent-300 | name + range | %). 4 px bar on neutral-900 track; the dominant band is accent, the others neutral-600; width animates over 0.4 s.

**EMG**
- Hero: "RMS AMPLITUDE · BICEPS", value in mV (250 ms window), tag "Contraction" (accent) when RMS > 0.12 mV, otherwise "Rest" (neutral).
- Activation card: "% MVC", 6 px accent bar with glow.
- Grid: Median frequency (Hz, from 20 Hz upward), Triceps RMS (mV).

**All signals** — Markers card:
- Header "Markers" + "N this view".
- Empty: "Press M or Marker to annotate the trace."
- List (latest 5 first): outline tag "M3" | label | HH:MM:SS.

Footer: Status (Live / Recording / Display frozen / Device offline), Electrode impedance "< 5 kΩ".

---

## 8. Wi-Fi device dialog

Modal, centered, width 420, surface, radius 14, lg elevation, padding 22. Backdrop neutral-900 at 50% with 2 px blur. Clicking the backdrop or ✕ closes it.

- Header: kicker "DEVICE CONNECTION", title "Connect amplifier over Wi-Fi", ghost ✕ button.
- Status row: "3 devices nearby" / "Scanning for devices…", plus a ghost "Rescan" button (`arrow-clockwise`).
- Network list (single select). Each row: signal icon (wifi-high/medium/low) | name + meta | status ("Connected" / "Connecting…" / "Locked"). Selected row uses the same style as the selected signal card.
  - BioAmp-8CH-3F2A — Amplifier · WPA2 · 5 GHz
  - BioAmp-8CH-71C0 — Amplifier · WPA2 · 2.4 GHz
  - Ward4-Telemetry — Hospital network · WPA2-Enterprise
- Passkey field (password echo), shown when the selected network isn't the connected one. Placeholder "Printed on the amplifier label".
- Actions (right-aligned): "Disconnect" (secondary, only when connected) and the primary button "Connect" / "Connecting…" / "Connected" (disabled while connecting or when already connected).
- States: `off → connecting → connected`. On success the dialog closes after 0.6 s. While not connected, acquisition and metrics pause and the status shows "Device offline".

---

## 9. Behavior summary

| Action | Input | Result |
|---|---|---|
| Select signal | Click card / keys 1–3 | Swap channel rows, filters, titles, metrics |
| Toggle domain | Segmented switch / F | Time ⇄ frequency rendering, toolbar swaps Window ⇄ Averaging |
| Window | 2.5 / 5 / 10 s | Time span + grid density + tick labels |
| Gain | ×0.5 / ×1 / ×2 | Vertical scale (time); dB offset (freq) |
| Freeze | Button / Space | Stop acquisition display; icon → play; footer "Frozen" |
| Marker | Button / M | Store {id, signal, sample index, timestamp, label}; draw on plot; list in panel |
| Record | Header button | Toggle recording, show mm:ss timer |
| Wi-Fi | Header button | Open device dialog |

---

## 10. Qt implementation notes

**Architecture**
- `AcquisitionWorker` (QThread): reads device samples (Wi-Fi via `QTcpSocket`/`QUdpSocket`), applies filters (biquad HP/LP/notch per signal), writes to lock-free ring buffers (12 s per channel).
- `SignalModel` (QObject, exposed to QML): current signal, domain, window, gain, averaging, paused, markers, metrics (Q_PROPERTY + NOTIFY).
- `SpectrumEngine`: FFT via **KissFFT** or **FFTW** (N = 512, Hann), EMA averaging, band power, dominant/median frequency.
- `WaveItem` (C++ `QQuickItem` with `QSGGeometryNode` line strip, or `QQuickPaintedItem` + `QPainter` for a first version): one per channel row; repaint driven by a 60 Hz `QTimer` / `QQuickWindow::frameSwapped`. Draw the grid as a cached texture.
- `DeviceManager`: scan, connect, disconnect; emits `stateChanged(Off|Connecting|Connected)`.

**QML component map**
- `Header.qml`, `SignalRail.qml` (`ListView` + `ButtonGroup`), `WavePanel.qml` (toolbar + `Repeater` of `ChannelRow.qml`), `MetricsPanel.qml` (`StackLayout` by signal), `WifiDialog.qml` (`Popup`, modal, `dim: true`).
- `Theme.qml` singleton holding every token in §1.
- Segmented control: `RowLayout` of `AbstractButton`s in a `ButtonGroup`, outlined `Rectangle` background.
- Faded rule: `Rectangle { height: 1; gradient: Gradient { orientation: Gradient.Horizontal; … } }`.
- Glow: `MultiEffect` (Qt 6.5+) with `shadowEnabled`/`blurEnabled`, or draw a wider low-alpha stroke under the main stroke (cheaper).

**Qt Widgets alternative**
- `QMainWindow` + `QGridLayout`; QSS for tokens; plots with **QCustomPlot** or Qt Charts (`QLineSeries` with `setUseOpenGL(true)`); segmented control = `QButtonGroup` of checkable `QToolButton`s.

Example QSS:
```css
QWidget { background:#161826; color:#E9E9ED; font-family:Inter; font-size:13px; }
QFrame#panel { background:#232532; border:1px solid #3F424D; border-radius:14px; }
QPushButton.primary { background:transparent; color:#9184D9; border:1px solid #9184D9; border-radius:8px; padding:0 12px; min-height:32px; }
QPushButton.primary:hover { background:rgba(145,132,217,0.12); }
QPushButton.primary:pressed { background:rgba(145,132,217,0.22); }
QPushButton.secondary { background:transparent; border:1px solid rgba(233,233,237,0.16); border-radius:8px; }
QPushButton.secondary:hover { background:rgba(233,233,237,0.07); }
QToolButton.seg { border:1px solid rgba(233,233,237,0.16); padding:4px 10px; font-size:12px; }
QToolButton.seg:checked { color:#9184D9; border:1px solid #9184D9; }
QLabel.kicker { color:#9184D9; font-size:10px; letter-spacing:1px; text-transform:uppercase; }
*:focus { outline:none; border:2px solid #9184D9; }
```

**Performance targets**
- 60 fps trace rendering with up to 4 channels at 1 kHz.
- FFT refresh at about 12 Hz; metrics at 2.5 Hz.
- Display latency below 100 ms from sample arrival.

**Clinical notes**
- Show units and scale on every channel; never hide the filter state.
- Freeze must not stop recording. Keep the two independent.
- Device-offline state must be visually obvious (status text + header icon); consider an alarm banner in the production build.
- Values marked static in the mockup (PR, QRS, QTc, impedance, battery) must come from real measurement in production.
