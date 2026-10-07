# Discrete EMG/ECG analog front end — design for a FES_Board respin

Alternative to the ADS1298 (U2): instrumentation amplifier + analog filter chain
into the STM32F767's own 12-bit ADC.

**Status: proposal, not built.** Every number below is calculated, none measured.

## Why consider this at all

Not because the ADS1298 failed — it did not. Its internal ±1 mV self-test
produced a clean square wave on 2026-08-04, which proves SPI, RDATAC frame
alignment, sign extension, the channel transpose, framing, CRC, UART and the
host decode are all correct together.

The real argument is **observability**. Answering "is the ADC working?" on the
ADS1298 required firmware, host software and a purpose-built self-test mode. On
a discrete chain the same question is a scope probe on a pin, and the front end
produces a viewable EMG trace **with no firmware running at all**. For a board
that has to be developed, demonstrated and defended, that is worth real money.

Read the trade honestly before committing — see "What this costs" at the end.
It is not cheaper, not smaller, and not more accurate.

## Architecture

```
electrode ─ Rs ─┬─ BAV99 ─┬─ INA333 ─ HPF ─ G=51 ─ LPF ─ STM32 ADC
   (J1..J4)     │  clamp  │  G=10    (sw)         500Hz   16 kSPS
              C_cm      C_diff                            │
                                                    software ÷16
                                                          │
                                                       1 kSPS
```

One channel shown; four required (J1–J4). RLD and the mid-rail reference are
shared across all four.

### Stage 0 — input protection and RF rejection

| Part | Value | Note |
|---|---|---|
| Rs (series, each leg) | 10 kΩ | also the clamp's current limit |
| D1–D8 | BAV99 | **already on the board, reuse** |
| C_diff (across the pair) | 1 nF | differential corner 7.96 kHz |
| C_cm (each leg to GNDA) | 100 pF | common-mode corner 159 kHz |

`C_diff` must be **at least 10x** `C_cm`. Mismatch between the two `C_cm` caps
converts common-mode into differential, which is exactly the mains pickup the
in-amp then cannot reject. Use 1 % C0G and keep `C_diff` dominant.

Thermal noise of 2 x 10 kΩ: `sqrt(4kT x 20k)` = 18.2 nV/sqrt(Hz), which over a
450 Hz band is **0.39 µV RMS**. Negligible against the electrode.

### Stage 1 — instrumentation amplifier, G = 10

**INA333** (rail-to-rail, 1.8–5.5 V, single-resistor gain).

```
G = 1 + 100 kΩ / R_G      R_G = 11.0 kΩ (E96)  ->  G = 10.09
```

**Why only 10.** The electrode DC offset is amplified along with the signal. A
matched gelled Ag/AgCl pair gives roughly ±100 mV differential, so at G=10 the
output sits at 1.65 V ±1.0 V — inside a 3.3 V rail with margin. At G=100 it
would slam the rail on offset alone and never recover.

**This is the whole reason the analog high-pass is mandatory**, and the reason
the ADS1298 does not need one: 24 bits of range digitises the offset and the
signal together.

Noise: INA333 is ~50 nV/sqrt(Hz), so 1.06 µV RMS over 450 Hz. With stage 0 that
is **1.13 µV RMS ≈ 6.8 µV p-p** input-referred — comparable to the ADS1298's
4 µV p-p spec once scaled from its 150 Hz test bandwidth to 450 Hz.

### Stage 2 — high-pass, software-switchable

Passive RC. The resistor returns to `VREF_MID`, so it sets the DC bias for the
next stage as well as the corner.

| Mode | R | C | Corner |
|---|---|---|---|
| EMG | 8.2 kΩ | 1 µF | **19.4 Hz** |
| ECG | 330 kΩ | 1 µF | **0.48 Hz** |

Selected by a **TS5A3157** SPDT analog switch on a GPIO. This matters: a fixed
20 Hz high-pass is what destroys an ECG — it sits on the QRS and removes the
P wave, ST segment and most of the T wave. Making it switchable preserves the
dual-mode capability that the ADS1298 gets for free in software.

### Stage 3 — second gain stage, G = 51

Non-inverting, `G = 1 + 49.9 kΩ / 1.0 kΩ = 50.9`.

```
total gain = 10.09 x 50.9 = 514
full scale = 3.0 V / 514 = 5.8 mV differential
```

Covers surface EMG (50 µV – 5 mV) and ECG (~1 mV) with headroom.

### Stage 4 — anti-alias low-pass, 2nd-order Sallen-Key

Unity gain, `R1 = R2 = 22 kΩ`, `C1 = 22 nF`, `C2 = 10 nF`.

```
f_c = 1 / (2*pi*R*sqrt(C1*C2)) = 488 Hz
Q   = 0.5 * sqrt(C1/C2) = 0.742      (Butterworth is 0.707)
```

### Sampling — 16 kSPS then decimate, not 1 kSPS directly

**A 2nd-order filter at 488 Hz is nowhere near enough for a 1 kSPS Nyquist ADC.**
Content at 550 Hz would fold straight onto 450 Hz essentially unattenuated.

Sample each channel at **16 kSPS** and average 16 samples in software:

- Nyquist moves to 8 kHz, where the filter gives `2 x 20*log10(8000/488)` = **49 dB**
- averaging 16 gains `log2(sqrt(16))` = **2 bits**, so 12-bit becomes ~14-bit effective
- output rate is exactly **1 kSPS**, matching the existing wire protocol

Resolution after decimation:

```
3.3 V / 2^14        = 201 µV/LSB at the ADC pin
201 µV / 514        = 0.39 µV/LSB referred to the electrodes
```

That sits **below the 1.13 µV RMS analog noise floor**, so the ADC is no longer
the limiting element — which is the correct place to stop. (The ADS1298 at
gain 6 is 0.0477 µV/LSB, about 8x finer.)

The STM32F767 ADC does 2.4 MSPS, so 4 ch x 16 kSPS = 64 kSPS is trivial. Use
DMA in circular mode.

### Shared — right-leg drive and reference

**RLD**: sum the four inputs through 1 MΩ each into an inverting amp,
`Rf = 390 kΩ` with `Cf = 1 nF` (pole at 408 Hz) for stability, then a **1 MΩ
series resistor** into the RL electrode. That resistor is a safety limit: it caps
fault current at 3.3 µA.

**VREF_MID**: 2 x 10 kΩ from +3.3VA, 10 µF decoupling, buffered by an OPA333.
It biases every stage, so it must be low-impedance.

## Bill of materials

| Part | Qty | Package | ~Unit | ~Total |
|---|---|---|---|---|
| INA333 | 4 | MSOP-8 | $5.50 | $22.00 |
| OPA4333 (2nd stage + LPF) | 2 | TSSOP-14 | $6.00 | $12.00 |
| OPA2333 (RLD + VREF buffer) | 1 | MSOP-8 | $3.00 | $3.00 |
| TS5A3157 (mode switch) | 4 | SC-70-6 | $0.60 | $2.40 |
| passives | ~50 | 0402/0603 | — | ~$5.00 |
| | | | | **~$44** |

## What this costs, honestly

| | ADS1298 | This design |
|---|---|---|
| Cost, 4 ch | ~$35–45 | **~$44** |
| Components | ~20 | **~70** |
| Board area | 1x TQFP-64 | **~4x** |
| Resolution (referred to input) | 0.0477 µV/LSB | 0.39 µV/LSB (**8x worse**) |
| Noise, input-referred | ~4 µV p-p | ~6.8 µV p-p |
| Channels available | 8 | 4 (scales linearly in parts) |
| Simultaneous sampling | yes, 8 ΔΣ | no — muxed SAR, phase skew |
| Lead-off detection | on-chip | not included |
| Gain / rate change | register write | resistor swap |
| Mode (EMG/ECG) | software | GPIO + analog switch |
| **Verify with a scope** | **impossible** | **yes, every stage** |
| **Works with no firmware** | **no** | **yes** |

**It is not cheaper, not smaller, and not more accurate.** It buys exactly one
thing: you can see the signal at every point with an oscilloscope, and the front
end produces a real EMG trace before a single line of firmware exists.

## Firmware impact

Small, and the host software does not change at all.

- **delete** `drivers/src/ads129x.c`, `bus/src/spi_bus.c` and the register map
- **add** an STM32 ADC + DMA source at 16 kSPS x 4 ch with software ÷16
- **keep** `proto/` unchanged — same 0xAA55 framing, same CRC, same DATA layout
- **keep** the entire Qt app unchanged; report the new scale in the INFO frame:
  `vref_uv` and `gain` become the analog chain's numbers (`gain = 514`), and the
  host's `uvPerCode` arithmetic then works out on its own

Add test points at the output of every stage. That is the entire point of the
exercise, and they cost nothing.

## Open questions before committing

1. **Do the two-minute check first** — ADS1298 with `EMG_SELFTEST` off and the
   host set to the ECG preset (0.5–40 Hz). If a real ECG appears, the existing
   hardware works and this respin buys observability only.
2. **±100 mV offset is an assumption.** It holds for a matched gelled Ag/AgCl
   pair. Dry or dissimilar electrodes reach ±300 mV, which saturates stage 1 at
   G=10 — that case needs a DC servo (integrator from the INA output back into
   its REF pin) instead of a passive high-pass.
3. **Consider a hybrid.** Populate one discrete channel for scope-level
   verification and keep the ADS1298 for the other channels. Costs ~$12 and one
   op-amp, and you get both the observability and the resolution.
