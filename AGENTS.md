# FES_Board

KiCad hardware project for a wearable biopotential sensing board: 8-channel
biopotential AFE (EMG/ECG) + STM32F7 + Wi-Fi/BLE + IMU, USB-C powered with
LiPo charging. Electrodes attach via four 3.5 mm TRS jacks.

This file covers the **hardware**. Firmware and host software each have their
own guide:

| Tree | What | Guide |
|---|---|---|
| `firmware/` | Zephyr app (`emg_reader`) — **its own git repository**, nested | `firmware/` + the memory notes |
| `software/` | Qt 6 / QML host viewer | `software/AGENTS.md` |

`FES_Board.ioc` is a CubeMX pin-planning file only — no generated sources,
nothing to compile. (An earlier revision of this file said there was no firmware
at all; `firmware/` has since become a working Zephyr application that streams
4 channels at 4 kSPS over a framed binary link.)

**Process rules — how to work in this repo, the KiCad and pcbnew traps, the
verify-don't-assume rules, and the debugging discipline — are in
[`WORKFLOW.md`](WORKFLOW.md). Read it before your first edit.**
**Bugs found and fixed are logged in [`BUG_LOG.md`](BUG_LOG.md) — check it
before re-diagnosing anything.**

## Status — PLANNING PHASE (re-opened 2026-09-01)

**Do not implement anything until planning is complete and approved.** No
schematic edits, no PCB edits, no firmware changes. Planning was restarted from
scratch on 2026-09-01 and supersedes the in-progress work described below.

**Direction: the ADS1298 (U2) is to be removed.** It is being replaced by a
discrete analog front end — instrumentation amplifier plus analog filter chain —
feeding the STM32F767's own 12-bit ADC.

State the reason accurately so it is not misremembered later: the ADS1298 was
**verified functional** on 2026-08-04. Its internal ±1 mV self-test produced a
clean square wave through the raw path, which proves SPI, RDATAC frame
alignment, sign extension, the channel transpose, framing, CRC, UART and the
host decode are all correct together. The earlier "random sinewave noise" was
the host's EMG band-pass preset applied to an ECG, not a hardware fault. **This
removal is therefore not a fault report** — it is that the EMG result did not
meet expectations and the part cannot be observed with an oscilloscope, so every
debug cycle needs firmware plus host software before it can say anything.

`EMG_AFE_discrete_design.md` is **background input to planning, not the plan of
record.** It was drafted before this reset; treat its component values as a
sketch to be re-derived, not as decisions already taken.

One fact worth carrying into planning: the ADS1298 occupies **PA4–PA7** (CS,
SCK, MISO, MOSI), and on the STM32F767 those same pins are **ADC1_IN4–IN7** —
four contiguous channels on one ADC. Removing U2 frees them.

Everything below this section describes the **board as it exists today** and
stays accurate until a respin lands: the U2 refdes, the `AIN`/`AREF` netclasses,
`ADS1298_STM32_pinout.md`, and the `firmware/drivers/` ADS driver are all still
live. Expect every one of them to be deleted or rewritten by the respin.

## Layout

| Path | What |
|---|---|
| `FESboard/FESboard.kicad_sch` | Schematic (single sheet, ~229 symbols) |
| `FESboard/FESboard.kicad_pcb` | PCB — **2-layer** (2026-08-03), 109 footprints, routing incomplete |
| `FESboard/FESboard.kicad_pro` | Project + netclass definitions |
| `FESboard/FESboard.kicad_dru` | Custom DRC rules (analog-input protection) |
| `FESboard/components.md` | Component notes — **stale, see below** |
| `WT02C40C_reference.md` | Fanstel module pinout/decoupling notes |
| `datasheet/` | ADS1298, TPS62130A, TPS7A20, BQ24074, ICM-45686, SBAU171 (EVM) PDFs |
| `software/` | Host viewer — see `software/AGENTS.md` |
| `firmware/` | Zephyr firmware — separate git repo |
| `FESboard/FESboard-backups/` | KiCad auto-backup zips — ignore these |

## Refdes map (re-verified against the PCB, 2026-07-28)

**The refdes were renumbered at some point between 2026-07-18 and 2026-07-28.**
The table below is read directly from `FESboard.kicad_pcb` footprint
Reference/Value fields. Every `U`/`J` assignment in the old table was wrong
except U2/U3/U4/U7 — see the warning after the table, it caused a real bug.

| Ref | Part | Role |
|---|---|---|
| U1 | BQ24074RGT | LiPo charger, VQFN-16 |
| U2 | ADS1298 | 8-ch 24-bit biopotential AFE, TQFP-64 |
| U3 | TPS62130A | Buck, VBUS/VBAT → +3.3V (digital), VQFN-16 |
| U4 | TPS7A20 | Low-noise LDO, VCC → +3.3VA (analog), SOT-23-5 |
| U5 | ICM-42688-P | 6-axis IMU, 14-pin LGA 2.5×3 mm |
| U6 | STM32F767ZITx | MCU, LQFP-144 |
| U7 | WT02C40C | Fanstel nRF5340 + nRF7002 (BLE + Wi-Fi 6) |
| D1–D8 | BAV99 | Clamps on the electrode inputs, SOT-23 |
| D9, D10 | LED | 0603 indicators |
| F1 | Fuse | 0603, on VBUS |
| L1 | 2.2 µH | 0603, TPS62130 inductor |
| **J1–J4** | PJ320D | 3.5 mm electrode jacks |
| **J5** | USB-C 2.0 16P | Power + USB (GCT USB4105) |
| **J6** | JST PH 2-pin | LiPo battery |
| **J7, J8** | TC2030-NL | SWD (J7 → nRF module, J8 → STM32) |
| Y1 | 8 MHz, 4-pin 3225 | HSE crystal → U6 PH0/PH1 |

**This staleness has already caused one real bug.** The netclass patterns in
`FESboard.kicad_pro` were written against the *old* numbering, so on
2026-07-28 the channel-1 electrode jack (J1) was resolving to the `DIG`
netclass — the least-protected class — while the USB-C connector matched an
`AIN` pattern. Fixed by repointing the patterns (J1–J4 → AIN, J6 → PWR,
J7 → DIG). **Re-verify refdes against the PCB before touching netclasses or
DRC rules**; they are matched by *name string*, so a renumber silently
redirects them.

**`components.md` disagrees with this table and the PCB is correct.** It lists
U4 as a MIC4604 85 V half-bridge driver and U5 as the IMU; neither is on the
board. Do not use `components.md` for refdes or part identity without checking
the PCB first. Treat its per-part *notes* as possibly-stale too.

Consequence worth stating plainly: **there is no stimulation output stage on
this board.** Despite the project name, nothing here generates a stimulation
pulse and no rail exceeds 5 V (VBUS). Do not assume high-voltage or
patient-isolation circuitry exists.

## ADS1298 driver traps — all of these cost real debugging time

Hard-won on 2026-08-31/09-02. Full detail per item is in
[`BUG_LOG.md`](BUG_LOG.md); this is the short form.

### Use RDATA, not RDATAC (B-032)

**RDATAC does not latch.** The device reloads its output shift register the
instant DRDY falls, so a conversion completing during your 27-byte read
overwrites it mid-transfer and you get a sample **spliced from two
conversions** — a convincing spike at a random moment.

**RDATA latches on the command**, so the read is coherent however long it takes.
Costs one byte per sample. TI's forum and the mainline Linux `ti-ads1298` driver
both say this outright; the driver author:

> "This chip doesn't have a buffer, but it does 'latch' the sample data when it
> receives a RDATA command (hence I use that in favor of RDATAC, which does not
> latch and might return corrupted data)."

Use `ads_emg_read_rdata_masked()` + `ads_emg_start_rdata()`.

### A register write must be ONE SPI transaction (B-025)

`spi_write_register()` splits opcode and data into two `spi_write_dt()` calls,
and **the writes silently did not take** — while reads, which use a single
`spi_transceive_dt()`, worked perfectly throughout. Use `spi_write_bytes()`,
which sends `{WREG|reg, 0x00, value}` in one transfer.

**"SPI works" is not one fact.** Reads working says nothing about writes.

### Always read the registers back (B-020, B-025)

Writing a register is not evidence it took, and the failure is silent and
catastrophic:

| Reg | Reset | What you get if the write is lost |
|---|---|---|
| `CONFIG1` | `0x06` | Low Power, fMOD/1024 = **250 SPS**, not 1000 |
| `CONFIG3` | `0x40` | **`PD_REFBUF = 0` — internal reference POWERED DOWN** |
| `CONFIG3` | `0x40` | `PD_RLD = 0` — no mid-supply bias either |

With no reference the ADC's output bears **no relation to its input** — the
trace looks identical whether the signal source is on or off. `ads_emg_verify()`
reads every configuration register back and logs each mismatch.

### DRDY on this wiring: poll by LEVEL, not edge (B-022) — **under review**

> **The DRDY wire was found loose on 2026-09-07 (B-034).** A pin stuck asserted
> produces exactly the symptom B-022 diagnosed, so the rule below may be a
> workaround for a broken connection rather than a property of the wiring.
> `main.c` now checks DRDY liveness at bring-up and prints `DRDY alive: N edges
> in 50 ms`. **Confirm that line looks healthy before trusting any timing
> conclusion, and re-test edge detection before treating the rule below as
> settled.**

The pin stays asserted while a result is pending rather than producing clean
edges. Edge detection takes exactly one sample and then stalls forever.
Level polling re-reads the same conversion instead, which is a degradation
rather than silence — prefer it, and never block for longer than a sample
period without servicing the ADC (B-030).

**Level polling therefore REQUIRES a minimum read interval** — otherwise the
loop re-reads one conversion many times, and since the rate is measured by
counting reads, the advertised rate inflates and the host time-scales every
trace by that factor. This is `MIN_READ_INTERVAL_US` in `main.c` (70 % of the
nominal period). It has caused the same wrong-BPM symptom twice: B-020, then
B-033 when a ring buffer removed the guard that had been throttling reads as a
side effect. **Do not remove a condition gating the DRDY read without replacing
the throttle.** And note the related invariant: `conversions++` must count
samples *delivered*, never reads *attempted*.

### The clock is NOT 2.048 MHz on this board (B-026)

Measured conversion rate is **~1130 SPS** where the nominal calculation gives
1000, implying fCLK ≈ 2.32 MHz — 13 % out, well beyond the ±5 % the datasheet
allows for the internal oscillator. Consistent with this not being the TI EVM.

**Never advertise a calculated rate.** The host derives its entire timebase from
`INFO.sample_rate_hz`, so a 13 % error there becomes 13 % on every heart rate
and every frequency. The firmware measures the rate over a settling window and
advertises the measured value.

## Power domains

```
USB-C VBUS ─F1─┬─ BQ24074 (U1) ─┬─ VCC ─┬─ TPS62130A (U3) ─ +3.3V  (digital)
   (J5)        │                │       └─ TPS7A20   (U4) ─ +3.3VA (analog)
      J6 LiPo ─┘                └─ battery path
```

Grounds are split: `GND` (digital) and `GNDA` (analog), intended to meet at a
single star point. Keep them distinct when editing the schematic.

## Stackup — **2-layer**, 1.6 mm (since 2026-08-03)

Reverted from 4-layer to 2-layer on 2026-08-03 for a cheap connectivity-test
run; the 4-layer order comes later. **The conversion was lossless**: the inner
layers held no traces and no zones, so DRC was byte-identical before and after
(78 violations / 144 unconnected both ways) and all 109 footprints, 595 pads and
777 tracks were preserved. Backup: `FESboard.kicad_pcb.bak_before_2layer`.

| Layer | Type | Thickness (mm) | Material | εr | Loss tan |
|---|---|---|---|---|---|
| F.Mask | mask | 0.01524 | JLCPCB Soldermask | 3.8 | 0 |
| F.Cu | copper 1 oz | 0.035 | | | |
| dielectric 1 | core | 1.49952 | FR4 | 4.6 | 0.02 |
| B.Cu | copper 1 oz | 0.035 | | | |
| B.Mask | mask | 0.01524 | JLCPCB Soldermask | 3.8 | 0 |

**εr 4.6 here is the commonly-quoted JLCPCB 2-layer FR4 figure and is NOT
verified against their stackup API** — unlike the 4-layer numbers below, which
were. It is fine for a connectivity-test board; pull the real value before
trusting any impedance calculation.

Consequence worth knowing: with a 1.5 mm core you **cannot** hit 90 Ω
differential for USB with sane trace widths. USB here is full-speed (12 Mbit/s)
via `usbotg_fs`, which tolerates the mismatch, but the `USB` netclass's
controlled-impedance intent is not achievable on 2 layers.

### The 4-layer stackup, for when you go back

JLCPCB **JLC04161H-3313**, chosen 2026-07-18. Values below are extracted from JLCPCB's own stackup API
(via `gsuberland/jlcpcb_autogenerated_stackups`), not from prose docs — several
secondary sources quote the core as εr 4.6, which is **wrong**; it is 4.43.

| Layer | Type | Thickness (mm) | Material | εr | Loss tan |
|---|---|---|---|---|---|
| F.Mask | mask | 0.01524 | JLCPCB Soldermask | 3.8 | 0 |
| F.Cu | copper 1 oz | 0.035 | | | |
| dielectric 1 | prepreg | 0.0994 | Nan Ya NP-155F 3313 | 4.1 | 0.02 |
| In1.Cu | copper 0.5 oz | 0.0152 | | | |
| dielectric 2 | core | 1.265 | Nan Ya NP-155F Core | 4.43 | 0.02 |
| In2.Cu | copper 0.5 oz | 0.0152 | | | |
| dielectric 3 | prepreg | 0.0994 | Nan Ya NP-155F 3313 | 4.1 | 0.02 |
| B.Cu | copper 1 oz | 0.035 | | | |
| B.Mask | mask | 0.01524 | JLCPCB Soldermask | 3.8 | 0 |

Intended layer use: **F.Cu** signal + components, **In1.Cu** solid GND plane,
**In2.Cu** power (`+3.3V` / `+3.3VA`), **B.Cu** signal. **Note that this was only
ever intent — the planes were never actually poured** (see Open items).

The thin 0.0994 mm F.Cu→In1.Cu coupling is the reason for this variant over
the default -7628: return currents stay tightly under their traces.

Applied 2026-07-18, backup `FESboard.kicad_pcb.bak_before_4layer`. Superseded
2026-08-03 by the 2-layer stackup above.

**How to change the layer count again.** This file is format version
`20260206`, whose copper-layer IDs differ from every published example
(`B.Cu` is 2 here, 31 in pre-2025 files; `In1.Cu`=4, `In2.Cu`=6). Do not
hand-write those IDs. Let pcbnew do it, then edit the stackup block as text —
that block is keyed by layer *name*, so it is safe:

```bash
flatpak run --filesystem=host --command=python3 org.kicad.KiCad -c "
import pcbnew
b=pcbnew.LoadBoard(PATH); b.SetCopperLayerCount(N)
en=b.GetEnabledLayers()
for L in [pcbnew.In1_Cu, pcbnew.In2_Cu]: en.addLayer(L)
b.SetEnabledLayers(en); b.SetVisibleLayers(en); pcbnew.SaveBoard(PATH,b)"
```

`BOARD_STACKUP` is **not** exposed to Python in KiCad 10, so the dielectric
rows cannot be set via the API — they have to be written into the file.

## PCB netclasses

Assigned by pattern in `FESboard.kicad_pro` → `net_settings.netclass_patterns`.
`FESboard.kicad_dru` references these names as **strings**, so renaming a
netclass silently disables its rules.

| Class | Nets | Track / clearance |
|---|---|---|
| `AIN` | `/IN1P`–`/IN4N`, `/RLDOUT`, `Net-(J1..J4-Pad*)`, `Net-(U2-RLDINV)` | 0.25 / 0.40 mm |
| `AREF` | `Net-(U2-VREFP)`, `Net-(U2-VCAP1..4)` | 0.30 / 0.20 mm |
| `APWR` | `+3.3VA`, `GNDA` | 0.60 / 0.30 mm |
| `PWR` | `VCC`, `VBUS`, `+3.3V`, `GND`, `Net-(F1-Pad1)`, `Net-(J6-Pin_2)` | 0.80 / 0.30 mm |
| `SWNODE` | `Net-(L1-Pad1)` — TPS62130 switch node | 0.80 / 0.40 mm |
| `USB` | `/USB_DP`, `/USB_DN`, `Net-(J5-CC*)` | 0.35 / 0.25 mm |
| `DIG` | `/STM32_*`, SWD, `Net-(J7-*)`, `Net-(J8-*)`, `Net-(U2-CLKSEL)` | 0.20 / 0.20 mm |

Patterns were repointed 2026-07-28 after the refdes renumber (see the refdes
warning above). J1–J4 are the electrode jacks, J5 is USB-C, J6 the battery,
J7/J8 the SWD headers.

The custom rules enforce separation from the electrode path — the largest is
`AIN` ↔ `SWNODE` at 2.0 mm, because the buck switches at up to 2.25 MHz and the
inputs run at PGA gains up to 12×.

Three `disallow` rules depend on a Rule Area named exactly `ANALOG_ZONE`, which
**does not exist yet** — those rules match nothing until it is drawn over U2's
analog half and the J1–J4 input network. Apply it to F.Cu/B.Cu only, never to
In1.Cu, so the ground plane stays whole.

The rules file is confirmed live: DRC output cites rules by name.

## Working on this repo

- **Check whether KiCad is open before editing any project file.** KiCad holds
  files in memory and overwrites on save, so an edit made while it is running
  will be silently lost. `pgrep kicad` and look for `~*.lck` files (stale locks
  do linger, so a `.lck` alone is not proof).
- **Back up before scripted edits.** Pattern already in use:
  `FESboard.kicad_pro.bak_before_netclass`, `*.kicad_sch.bak_decoup`.
- **Judge support components by net connectivity, not schematic position.**
  Whether a decoupling cap is correct depends on which nets its pads land on;
  proximity on the schematic canvas means nothing.
- **The refdes/value fields in the PCB are the ground truth** for what a part
  is. Prefer parsing `FESboard.kicad_pcb` over reading the markdown docs.
- **KiCad 10.0.5 is installed natively** (verified 2026-07-28). `kicad-cli` is
  on `PATH` at `/usr/bin/kicad-cli` and `import pcbnew` works in the system
  `python3` — the flatpak wrapper described in earlier revisions of this file
  is no longer needed, and there is no `--filesystem=host` sandbox limitation:

  ```bash
  kicad-cli pcb drc --format json --units mm -o out.json FESboard/FESboard.kicad_pcb
  python3 -c "import pcbnew; b=pcbnew.LoadBoard('FESboard/FESboard.kicad_pcb'); ..."
  ```

  `import pcbnew` prints harmless `PROPERTY_ENUM()` assert noise on stderr —
  redirect with `2>/dev/null`.

  **`kicad-cli pcb drc` resolves netclasses and the custom `.kicad_dru`
  relative to the board file.** Running DRC on a copy elsewhere silently falls
  back to default rules and reports completely different counts — 111/144 versus
  78/144 for the same board. **Always DRC in place**, and be suspicious of any
  comparison whose runs used different paths.

  **Three pcbnew Python traps in this KiCad 10 build**, all hit on 2026-08-03:
  `LoadBoard()` returns `None` for any filename not ending `.kicad_pcb` (so copy
  `.bak` files first); `GetNetcodeFromNetname()` and `FindNet()` both return a
  bare `SwigPyObject` — get the net code off a pad instead; and
  `BoardConnectedItem`-heavy calls after `b.Remove(zone)`, or any call to
  `b.BuildConnectivity()`, **segfault**. Build zones from a clean file rather
  than mutating one.

  **Zone fills are not recomputed by `kicad-cli drc`.** After changing anything
  that affects fill (netclasses, clearances, zone settings) you must refill or
  DRC reports stale geometry:

  ```python
  f = pcbnew.ZONE_FILLER(b); f.Fill(b.Zones()); pcbnew.SaveBoard(path, b)
  ```

  This bit once already: a netclass change looked like it *added* 12 clearance
  errors until the zones were refilled, after which it removed 2.

  The `kicad-mcp` MCP tools still cannot run headlessly (they fail with
  "Context is not available outside of a request") — use `kicad-cli`.
- **`kicad-cli` does not reject a malformed `.kicad_dru`.** Tested by injecting
  a syntax error: DRC ran anyway and silently dropped rules rather than
  erroring. A clean CLI run is therefore *not* evidence the rules file is
  valid. Confirm custom rules are live by checking that DRC output cites them
  by name (`rule 'ain_to_switch_node'`), or use *Check rule syntax* in the GUI.
- Custom symbol/footprint libs are referenced by **`${KIPRJMOD}`-relative**
  paths in `fp-lib-table` / `sym-lib-table` (e.g.
  `${KIPRJMOD}/libraries/Fanstel_WT.pretty`), so the project is portable.
  Earlier revisions of this file described absolute `/home/stephen/...` paths;
  that is no longer true as of 2026-07-28.

## HSE crystal (Y1) — why CL must stay low

**STM32F767 datasheet (DS11532) Table 43 specifies `Gm_crit_max = 1 mA/V`.**
Per AN2867 the crystal's critical transconductance must stay under it:

```
gm_crit = 4 · ESR · (2πf)² · (C0 + CL)²
```

With the ABM8's 8 MHz numbers (ESR ≤ 400 Ω, C0 ≤ 3 pF):

| Crystal CL | gm_crit | vs 1 mA/V limit | C1=C2 |
|---|---|---|---|
| 18 pF (ABM8 *standard*) | 1.78 mA/V | **fails, 1.78× over** | — |
| 12 pF | 0.91 mA/V | ok, only 1.10× | 16 pF |
| **8 pF (chosen)** | **0.49 mA/V** | **ok, 2.04× margin** | **8 pF** |

So Y1 is specified as **CL = 8 pF**, which is a non-default ABM8 order option
(the datasheet allows CL down to 6 pF). **Do not substitute a stock 18 pF
crystal** — it exceeds the STM32F7's drive capability and may not start.

Load caps from `CL = C1·C2/(C1+C2) + Cs` with Cs ≈ 4 pF (pin + trace):
`C1 = C2 = 2·(8 − 4) = 8 pF` → C61/C62.

## Editing the schematic programmatically

`FESboard.kicad_sch` is a single flat sheet (UUID
`/bc6baefb-a034-4913-a751-2c83a57572e0`) with no hierarchy, so local labels
connect globally. Nets are prefixed `/` at the root.

Coordinate transform, verified rather than assumed — schematic Y is flipped
relative to symbol-library Y:

```
sheet_x = instance_x + pin_local_x
sheet_y = instance_y - pin_local_y      # note the minus
```

A pin's `(at x y angle)` is its **connection point**; the pin body extends from
there by `length` in direction `angle`, toward the symbol body.

Two things that will bite:

- **Stay on the 1.27 mm grid.** Anything off-grid produces
  `endpoint_off_grid` ERC errors and may silently fail to connect. Verify with
  `coord / 1.27 == int`. A 0.02 mm slip cost five ERC errors when Y1 was added.
- **Verify by netlist, never by eye.** Export and check the pins actually
  landed on the intended net:

  ```bash
  flatpak run --filesystem=host --command=kicad-cli org.kicad.KiCad \
      sch export netlist --format kicadsexpr -o out.net FESboard/FESboard.kicad_sch
  ```

  The `kicadsexpr` netlist is multi-line — `(ref "X")` and `(pin "N")` sit on
  separate lines, so single-line regexes silently match nothing.

To render a region for visual check, export **PDF** and crop with
`pdftoppm -r 300 -x .. -y .. -W .. -H ..` (px = mm/25.4×300). Do not use
ImageMagick `convert` on the SVG export — its SVG renderer drops most of the
content and produces a misleadingly blank image.

## Commit style

Prefix with an uppercase verb and a colon: `ADD:`, `DESIGN:`, `FIX:`.
Example: `ADD: Power`, `DESIGN: ADS1298`.

## Open items

Baseline as of **2026-07-28** — compare against this rather than treating any
violation count as new breakage:

**The 2026-07-28 baseline recorded here was badly wrong and has been corrected.**
It claimed 7 violations / 17 unconnected and that copper pours existed on all
four layers. Re-measured 2026-08-03 with `kicad-cli pcb drc`:

- **78 violations, 144 unconnected, 0 schematic-parity errors.**
- **There are no copper zones on this board at all** — `grep -c '(zone' ` finds
  only `(zone_connect)` properties on pads. The planes described in the stackup
  section were never poured.
- **57 of the 78 violations are `via_dangling`** — vias placed expecting inner
  planes that do not exist. The rest: 10 `track_width`, 8 `diff_pair_gap`,
  2 `track_dangling`, 1 `diff_pair_uncoupled_length`.
- **The board is not orderable in either stackup.** 233 of 595 pads (39 %) are
  power/ground and currently have no copper connecting them.

### Copper pours (2026-08-03)

Poured by `FESboard/pour_zones.py`, which rebuilds every zone from
`FESboard.kicad_pcb.bak_before_routing` — **run it, do not hand-edit zones**, and
`--with-3v3` toggles the power island.

| Zone | Layer | Priority | Area |
|---|---|---|---|
| `GND` | F.Cu, B.Cu | 0 | 4975 / 3419 mm² |
| `GNDA` | F.Cu, B.Cu | 1 | 1550 / 1806 mm² |
| `+3.3V` | B.Cu | 2 | 1956 mm² |

`GND` covers the whole board at low priority and the others carve out of it —
letting priority resolve the overlap avoids computing the complement by hand and
gets inter-zone clearance for free.

The `GNDA` outline is shaped from where the GNDA pads actually are, not a
vertical split. A split gets two things wrong: **J10 (SWD) sits at y 58–62**,
above the analog cloud (y 80–122), and **U4, the analog LDO, sits just *right* of
the analog block** — hence the lobe out to x 148.5 below y 112.

The `+3.3V` island is a **2-layer compromise**: it is a 1956 mm² hole in the
bottom ground plane under the digital section, sized to cover the 20 dangling
+3.3V vias and no more. **Delete it on the 4-layer build**, where the power plane
returns on In2.Cu.

Result, all measured in place (see the DRC warning below):

| Stage | Violations | Unconnected |
|---|---|---|
| 2-layer, no pours | 78 | 144 |
| + ground pours | 63 | 76 |
| + `+3.3V` island | **43** | **56** |

`via_dangling` went 57 → 9; all 136 ground pads connect. **None of the 43
remaining violations is attributable to the pours** — they are 11 `track_width`,
10 `clearance` (USB traces at 0.15 mm, and note they resolve to netclass
`Default`, so the `USB` pattern is not matching), 9 `via_dangling`, 8
`diff_pair_gap`, 4 `drill_out_of_range` (U3 pad 17, 0.2 mm vs 0.3 mm minimum),
1 `diff_pair_uncoupled`.

**Still not orderable.** All 56 remaining unconnected are power: `+3.3V` 8,
`VCC` 8, `+3.3VA` 6, `VBUS` 6, `Net-(J6-Pin_2)` 3. These need real
point-to-point routing — freerouting via a Specctra DSN export, or by hand.
- **ERC: 148 violations** as of 2026-07-18 (not re-measured since). Zero
  `endpoint_off_grid` — if that type reappears, something was added off the
  1.27 mm grid.

How the errors got from 112 to 0 (2026-07-28), because the reasoning matters
more than the number:

- **Most were never routing problems.** 20 `clearance` + 4 `hole_clearance`
  were pad-to-pad *inside a single package*, where the manufacturer's own pad
  pitch is tighter than the netclass. Rerouting cannot fix those; they needed
  scoped DRU exceptions (see below). Diagnose this first — check whether both
  DRC items belong to the same footprint before touching any track.
- 12 `starved_thermal` were a genuine design defect, not noise (thermal spokes
  wider than the pads they attach to) — see the zone section below.
- 58 `diff_pair_gap` were a mis-specified rule (a controlled-impedance number
  applied to a 500 Hz signal) plus some genuinely divergent routing.
- 125 `text_height` came from a prior silk-shrink to 0.5 mm/0.08 mm, which is
  below both JLCPCB's and PCBWay's 0.8 mm/0.15 mm silkscreen minimum. Fixed by
  resizing references back to **0.8 mm / 0.15 mm**, not by lowering the rule.
  Cost: a handful of `silk_over_copper` / `silk_overlap` on a dense board.
- The remainder (`track_width`, dangling tracks, 2 unconnected, the last
  diff-pair spots) were fixed by hand in the GUI.

Design items still open:

- **U5's footprint library is missing.** U5 is stored as
  `ICM-42688-P:PQFN50P300X250X97-14N`, but that library nickname is in neither
  the project nor the global `fp-lib-table`, and no `.pretty` containing it
  exists anywhere on disk (the global table has `ICM_45686` — a *different*
  part, matching the stale datasheet in `datasheet/`). The geometry is
  embedded in the `.kicad_pcb`, so **fabrication is unaffected**, but nothing
  is validating that 14-pin 0.5 mm-pitch LGA against the real datasheet, and
  *Update Footprint from Library* will fail. Worth re-downloading the
  footprint into `FESboard/libraries/ICM-42688-P.pretty` and registering it
  (same `${KIPRJMOD}` style as `Fanstel_WT`) before paying for assembly.
- `ANALOG_ZONE` rule area not yet drawn. Apply to F.Cu/B.Cu only, never In1.Cu.
- Y1 has no series damping resistor on OSC_OUT. Acceptable at this drive level,
  but re-check against AN2867 if the crystal is substituted.
- **U5** (ICM-42688-P) pin 7 `RESV` must tie to GND per the datasheet.
- U4 (TPS7A20) pin 4 `NR/SS` is unconnected — a noise-reduction cap there would
  lower output noise on the ADS1298's analog rail.
- Keep the In1.Cu GND plane unbroken under the **J1–J4** → U2 input path. An
  unsplit plane there is worth more than any DRC-rule tuning.

### Package-geometry DRU exceptions (added 2026-07-28)

Three rules in `FESboard.kicad_dru` exist **only** because a part's own pad
pitch is tighter than the netclass clearance. These are not routing problems
and no amount of rerouting clears them — the pads are where the manufacturer
put them:

| Rule | Scope | Relaxes to | Why |
|---|---|---|---|
| `u5_imu_pad_pitch` | pads of U5 | clearance 0.14 mm | ICM-42688-P 14-pin LGA, adjacent pads 0.15 mm apart vs 0.20 mm netclass |
| `u3_buck_pad_pitch` | pads of U3 | clearance 0.20 mm | TPS62130A VQFN-16, SW pins 1–3 vs exposed GND pad at 0.21 mm vs 0.40 mm `SWNODE` |
| `j5_usbc_hole_pitch` | pads of J5 | hole clearance 0.19 mm | GCT USB4105 USB-C, PTH GND pins A1/B1/A12/B12 0.1944 mm from its own NPTH mounting pegs vs 0.25 mm |

All three are deliberately narrow: **both objects must be pads AND both must
belong to that one footprint** (`A.memberOfFootprint('U5') &&
B.memberOfFootprint('U5') && A.Type == 'Pad' && B.Type == 'Pad'`). Tracks and
vias are *not* relaxed, so routed copper still owes full netclass clearance
even inside those courtyards, every other hole on the board still owes 0.25 mm,
and the electrode-isolation rules are untouched.

`memberOfFootprint()` is preferred over the `intersectsCourtyard()` used by the
older `ads_u2_fanout` rule — it cannot accidentally catch a neighbouring part
whose pad strays into the courtyard. **Both tokens fail silently if
misspelled** (as `A.Net` did — it must be `A.NetName`), so always confirm a new
rule by measuring that the target violations actually disappear, never by a
clean CLI run alone.

All of these are well within fab capability — JLCPCB/PCBWay copper spacing
floor is ~0.09 mm — so relaxing removes false positives without accepting
anything unmanufacturable.

### Zone pad-connection settings (changed 2026-07-28)

The F.Cu/B.Cu `GND` pour uses `connect_pads thru_hole_only` — **solid**
connections for SMD pads, thermal relief only for through-hole. This was
required, not cosmetic: U3's QFN pads are 0.25 mm wide and U2/U6's TQFP pads
0.30 mm, both *narrower than the 0.5 mm thermal spoke*, so two spokes could
never form and every one of them was a `starved_thermal` error. Thermal relief
on an IC ground pin also adds needless impedance to the return path.

U3's four thermal vias (pad 17, PTH) carry a per-pad override to **solid**
(`ZONE_CONNECTION_FULL`) — they are through-hole, so `thru_hole_only` would
otherwise have given them thermal relief, which defeats the point of a thermal
via. Trade-off accepted: solid SMD connections slightly raise tombstoning risk
on small passives during reflow.
