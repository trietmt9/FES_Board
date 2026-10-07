# WORKFLOW — how to work on FES_Board

Operating rules for an AI agent (or any new contributor) in this repository.

**Every rule here exists because something actually went wrong.** The "why" is
kept short but never omitted, because a rule whose reason is forgotten gets
optimised away by the next person who thinks they know better.

---

## 0. Phase gate — check this first, every session

The project is currently in the **PLANNING PHASE** (re-opened 2026-09-01). See
the Status section at the top of `CLAUDE.md`.

While the gate is closed:

| Allowed | Not allowed |
|---|---|
| Reading, measuring, analysing | Schematic edits |
| Writing and revising plans | PCB edits |
| Answering questions, doing calculations | Firmware changes |
| Drafting design options with trade-offs | Host software changes |

If a request would cross that line, **say so and ask** rather than doing it
quietly. Reopening the gate is the user's call, not yours.

---

## 1. Read before you touch anything

In this order:

1. **`CLAUDE.md`** — hardware ground truth, phase gate, open items
2. **This file**
3. The subtree guide for whatever you are touching — `software/CLAUDE.md`,
   `firmware/`
4. Memory notes (`ads1298-board-is-not-ti-evm`, `fesboard-pcb-actual-state`, …)

Then **verify the current state yourself.** Do not open with an action based on
what a document claims.

> `CLAUDE.md` once recorded a baseline of "7 violations / 17 unconnected" and
> said copper pours existed on all four layers. Re-measurement found **78
> violations, 144 unconnected, and zero copper zones on the board.**
> `components.md` still lists parts that are not on the board at all.

---

## 2. Verify, don't assume — the prime directive

**2.1 Never infer hardware identity from a document lying in the repo.**

> `datasheet/sbau171d.pdf` (the TI ADS1298ECG-FE guide) sits in this repo, so it
> was assumed to describe the ADS1298 board in use — and that board's electrode
> map was written into firmware register presets. The real board has **JP36** and
> a populated **D11**; the TI board has neither (it has JP1–JP33 and D1–D10, all
> "Not installed"). **A datasheet in the repo is not evidence of what is on the
> bench.** Confirm against a silkscreen, a photo, or the user.

**2.2 The PCB is ground truth for what a part is.** Parse
`FESboard.kicad_pcb` refdes/value fields. Not the markdown, not the schematic
position.

**2.3 Judge support components by net connectivity, not schematic position.**
Whether a decoupling cap is correct depends on which nets its pads land on.

**2.4 Measure; never estimate and present it as measurement.** If a number was
not produced by a command in this session, say where it came from.

---

## 3. KiCad work

**3.1 Is KiCad open?** `pgrep kicad`, and look for `~*.lck` files (stale locks do
linger, so a `.lck` alone is not proof). KiCad holds files in memory and
overwrites on save — an edit made while it runs is silently lost.

**3.2 Back up before any scripted edit.** Established pattern:
`FESboard.kicad_pcb.bak_before_<thing>`.

**3.3 Always DRC in place.**

```bash
kicad-cli pcb drc --format json --units mm -o out.json FESboard/FESboard.kicad_pcb
```

> `kicad-cli` resolves netclasses and the custom `.kicad_dru` **relative to the
> board file**. Running DRC on a copy elsewhere silently falls back to default
> rules: 111 violations versus 78 for the same board. This invalidated a whole
> set of reported numbers once already.

**3.4 A clean `kicad-cli` run is not proof the rules file is valid.** It does
**not** reject a malformed `.kicad_dru` — it drops rules and carries on. Confirm
a rule is live by finding it cited by name in DRC output, and confirm a new rule
works by measuring that the target violations actually disappear.

**3.5 Refill zones after anything that affects fill**, or DRC reports stale
geometry:

```python
f = pcbnew.ZONE_FILLER(b); f.Fill(b.Zones()); pcbnew.SaveBoard(path, b)
```

**3.6 Three pcbnew traps in this KiCad 10 build.** `LoadBoard()` returns `None`
for any filename not ending `.kicad_pcb`; `GetNetcodeFromNetname()` and
`FindNet()` return a bare `SwigPyObject` (take the netcode off a pad instead);
and `b.BuildConnectivity()` or calls after `b.Remove(zone)` **segfault** — build
zones from a clean file rather than mutating one.

**3.7 Netclasses and DRC rules match by *name string*.** A refdes renumber or a
netclass rename silently redirects them. This already sent the channel-1
electrode jack into the least-protected netclass.

**3.8 Verify schematic edits by netlist, never by eye.** Export and check the
pins landed on the intended net. The `kicadsexpr` netlist is multi-line, so
single-line regexes silently match nothing.

---

## 4. Firmware (`firmware/`)

**`firmware/` is its own git repository, nested inside this one.** Commit there
separately.

```bash
source ~/zephyrproject/.venv/bin/activate
export ZEPHYR_BASE=~/zephyrproject/zephyr ZEPHYR_SDK_INSTALL_DIR=~/zephyr-sdk-0.17.4
cd firmware && west build -b nucleo_f767zi . && west flash
```

**4.1 The wire protocol is one spec in two halves.**
`firmware/proto/emg_frame.{h,c}` and `software/emg-viewer/src/core/` are the two
sides. `tst_frameparser` compiles the firmware's C encoder and decodes it with
the host parser. **Change both sides in the same commit** or that test fails to
build — which is the point.

**4.2 Guard silent encodings with `BUILD_ASSERT`.** A wrong bit in a packed
register byte is accepted by the part, leaves the link healthy, and shows up only
as a subtly wrong trace. Pin the value so a bad edit fails the build.

**4.3 A constant that must track another has to say so and be checked.**
`EMG_SAMPLE_RATE_HZ` must match the `data_rate` enum — it is what every INFO
frame advertises and the host derives its whole timebase from it. Nothing
enforces the pairing automatically.

---

## 4b. Changes to analog behaviour ship OFF

**If a change alters what the ADC actually sees, and it cannot be measured here,
it defaults to disabled.** Digital fixes may default on; analog ones are opt-in
until proven on the bench.

> Two changes were shipped always-on in one session: closing the RLD feedback
> loop, and lead-off detection that injects 6 nA into the inputs being measured.
> Each is defensible alone. Together, enabled by default and unvalidated, they
> made a working setup wrong and could not be bisected. See B-017.

Corollary: **change one analog thing at a time**, and give each its own build
flag so the user can bisect without editing code.

## 4c. A comment explaining unobvious code is evidence

If code takes a strange-looking route and a comment says why, **that comment is a
measurement someone already made.** Do not replace the code without first
reproducing the condition it describes.

> The acquisition loop polled DRDY by level, with a comment recording that the
> pin never produces clean edges on this wiring. That was replaced with edge
> detection to fix a real duplicate-sample bug — and the stream stopped dead
> after one sample, exactly as the comment predicted. See B-022.

When both routes have failure modes, **default to the one that produces data**
(a degraded signal is debuggable, silence is not), put the other behind a flag,
and add a counter that says which one this hardware actually needs.

## 4d. ADS1298 start-up sequence

Order matters and several steps are silent if skipped. Working implementation:
`firmware/src/main.c`. Per-step detail: `BUG_LOG.md`.

| # | Step | Skip it and you get |
|---|---|---|
| 1 | SPI + GPIO ready; CS idle high, DRDY input, **START pin LOW** | START floating high ⇒ the START *command* is ignored and the device never converts |
| 2 | Assert RESET ≥ 2 tCLK, release, wait ≥ 18 tCLK (~20 µs) | first command lands before the device is ready |
| 3 | **SDATAC** | every register write is ignored — the device is in continuous mode at power-up |
| 4 | `CONFIG1` (rate/mode), `CONFIG2`, `CONFIG3` | see the reset-value table in `CLAUDE.md`; the worst is `CONFIG3 = 0x40`, reference **powered down** |
| 5 | **wait ~150 ms** for the internal reference to settle | early samples are garbage while VCAP1 charges |
| 6 | `CHnSET`, then `RLD_SENSP/N`, `WCT1/2`, `LOFF` as needed | RLD derived from a **detached** electrode contaminates every channel (B-012) |
| 7 | **read every register back and compare** | writes fail silently — this is B-025, the single most expensive bug in this log |
| 8 | read ID, check `id & 0x1F == 0x12` | wrong part or dead SPI |
| 9 | START (pin already low) | — |
| 10 | **stay out of RDATAC**; read per-sample with RDATA | RDATAC does not latch ⇒ spliced samples (B-032) |

**Steps 3, 5, 7 and 10 are the ones that get skipped**, and all four fail
quietly rather than loudly.

## 4e. Reading phases, and DMA

### The read itself

```
wait for DRDY asserted   (LEVEL on this wiring - see 4c/B-022)
send RDATA               (LATCHES the sample - B-032)
clock out 1 + 27 bytes   in ONE transaction
check status nibble      raw[0] & 0xF0 == 0xC0, else discard
sign-extend bit 23       per channel
```

**One transaction, always.** Splitting the RDATA opcode from the data lets a
conversion land between them, which is the race RDATA exists to avoid. The same
rule sank register writes for weeks (B-025).

**Never block longer than a sample period without servicing the ADC.** At
~1130 SPS that is 885 µs, and a 116-byte `uart_poll_out()` is 1.26 ms — which
silently dropped 4 % of conversions (B-030). Interleave: a byte at 921600 is
10.9 µs, so polling DRDY between bytes costs nothing.

### DMA — not yet configured, and worth doing

Today the transfer is blocking: 28 bytes at 8 MHz ≈ **28 µs of CPU per sample**,
about 3 % at 1130 SPS. That is affordable for one channel and is not for eight
at 4 kSPS, where it becomes ~90 %.

Zephyr supports it: `CONFIG_SPI_STM32_DMA=y`, plus `dmas` and `dma-names` on the
SPI node. `spi_transceive_dt()` then uses DMA transparently — no API change.

```dts
&spi1 {
        dmas = <&dma2 3 3 STM32_DMA_PERIPH_TX>,
               <&dma2 2 3 STM32_DMA_PERIPH_RX>;
        dma-names = "tx", "rx";
};
```

*(Stream and request numbers above are illustrative — take the real ones from
the STM32F767 reference manual DMA request mapping table before using them.)*

Four things to get right, in the order they will bite:

1. **D-cache coherency.** The STM32F767 has a data cache, and Zephyr's Kconfig
   says so explicitly: `select CACHE_MANAGEMENT if CPU_HAS_DCACHE`. DMA buffers
   must be cache-line aligned and cleaned/invalidated around transfers, or you
   read stale data — intermittently, which looks exactly like the corruption
   B-032 was.
2. **DTCM is not DMA-accessible on STM32F7.** The DMA controllers cannot reach
   DTCM RAM; buffers must live in SRAM1/SRAM2. Verify against the reference
   manual's bus matrix for the specific part — a buffer that silently never
   fills is the failure mode.
3. **Arm RX before TX.** With RX started second the first bytes are lost to an
   overrun.
4. **RDATA still matters.** DMA reduces CPU cost; it does not change when the
   device latches. Keep RDATA.

**Keep the gap counter when switching.** `service_adc()` reports
`gaps: N overruns, ~M samples lost, worst U us` once a second — that is how a
DMA change gets verified rather than assumed. Overruns should stay near zero.

## 5. Host software (`software/emg-viewer/`)

```bash
cd software/emg-viewer/build && QT_QPA_PLATFORM=offscreen ctest --output-on-failure
```

Baseline: **7 suites, 102 cases, all passing.** Any failure is new breakage.

Read `software/CLAUDE.md` before editing — it carries the Qt/QML traps
(the singleton-in-subdirectory 100 % CPU hang, Qt 6.4 API gaps, the palette
issue) that are not guessable from the code.

**Tests can pass for the wrong reason.** When `FilterConfig::enabled` defaulted
to false, `passbandIsFlat` still passed — raw passthrough trivially satisfies
"the signal passes."

---

## 6. Debugging discipline

**6.1 Partition before investigating.** Find the one measurement that cuts the
system in half, and take it first.

> The ADS1298's internal ±1 mV self-test proves SPI, frame alignment, sign
> extension, the transpose, framing, CRC, UART and host decode **in a single
> measurement**, and is board-independent. It was built after days of
> investigation and answered the question immediately.

**6.1b Search for the known problem before deriving it.** If the symptom is
"random, occasional corruption" in a widely used part, someone has hit it and
written it up. Check the vendor's forum and any mainline Linux driver for the
chip **before** building instrumentation.

> Three rounds went into reproducing, instrumenting and reasoning about random
> spikes from first principles. A single search found the cause stated plainly
> by the author of the mainline `ti-ads1298` driver: RDATAC does not latch. See
> B-032.

**6.2 Test the cheapest hypothesis first — configuration before hardware.**

> Days were spent chasing "sine-like random noise" that turned out to be the
> host's **EMG band-pass preset (20–450 Hz) applied to an ECG**. A square wave
> rendered as a decaying sinusoid is the step response of a band-pass, not a
> broken ADC.

**6.3 Know what the display does to the data before drawing conclusions from
it.** `WaveformItem` subtracts the window mean from every sample (raw path
included) and autoscale stretches a few µV to fill the plot. Read the numeric
value, not the shape. Recordings are always raw, so decoding a `.emgraw`
directly bypasses every UI setting.

**6.4 Know the signal.** Raw surface EMG is a stochastic interference pattern —
it legitimately looks like noise, and the information is in the envelope. Do not
diagnose that as a fault.

---

## 7. Reporting

- **Say plainly what was measured and what was assumed.**
- **If tests fail, say so and show the output.** If a step was skipped, say that.
- **Correct wrong numbers explicitly**, once, without ceremony — then continue.
  Several DRC figures reported here were wrong because of the copy-vs-in-place
  trap, and saying so plainly was the only thing that made the later numbers
  trustworthy.
- **Do not claim done until verified.** "Builds" is not "works."
- Nothing in `software/` has ever run against real hardware. Treat every "works"
  in that tree as "works in replay" until proven otherwise.

---

## 8. Commits

Uppercase verb, colon, short subject: `ADD:`, `DESIGN:`, `FIX:`.
Example: `ADD: Power`, `DESIGN: ADS1298`.

Commit or push only when asked. `firmware/` commits go to its own repo.
