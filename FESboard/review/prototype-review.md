# FESboard prototype electrical/layout review

Reviewed saved FESboard.kicad_pcb, .kicad_sch, .kicad_pro, custom rules, and BOM/FESboard.csv using KiCad CLI 10.0.6. Design files were not modified. Review includes netlist inspection, front/back copper plots, ERC, saved-fill DRC, and in-memory zone-refill DRC with schematic parity. It does not certify assembly, firmware, or measured operation.

## Verdict

No copper shorts or unrouted connections were reported. Nevertheless, fix the charger EN2 bias before relying on USB startup, resolve L1's unspecified current capability, and correct the radio antenna copper clearance if wireless operation is required. These matter more than achieving zero cosmetic DRC warnings.

## Findings requiring action

1. **R15 = 1 MΩ cannot guarantee charger EN2 HIGH.** R15 connects VBUS to U1 pin 5 (EN2); R14 pulls EN1 low. BQ24074 has approximately 285 kΩ internal pulldowns on EN1/EN2. At 5 V the nominal divider produces only 1.11 V, below the 1.4 V guaranteed HIGH threshold and above the 0.4 V LOW limit. Thus resistor-programmed input current is not reliably selected; a 100 mA mode or unpredictable mode selection could produce startup failure/reset under load, especially without a battery. Change R15 to 10 kΩ for a robust high. R16 = 1.13 kΩ selects approximately 1.37 A input limit when that mode is active; the USB source and F1 must support the intended draw. R17 = 3.6 kΩ programs approximately 247 mA charging, not 1.5 A. Source: [TI BQ24074 datasheet](https://www.ti.com/lit/ds/symlink/bq24074.pdf), pp. 8–9, 14, 34.

2. **L1 is only specified as 2.2 µH in a 0603 footprint.** No manufacturer part/current/DCR specification is present in the BOM. This is a substantial unverified power-component risk, not proof that every 0603 inductor fails. An unsuitable part can saturate or drop excessive voltage, collapsing the 3.3 V rail during MCU/radio activity. Select an actual power inductor with adequate RMS and peak/saturation current, including ripple and startup margin; use its actual footprint. The module alone has a manufacturer example measuring 270 mA peak, before the STM32 load. TI specifies calculating peak inductor current and adding margin. Widen the approximately 2.75 mm series route from the merged SW fanout to L1 from 0.25 mm where practical; short individual pad escapes are a lower priority. Sources: [TPS62130 datasheet](https://www.ti.com/lit/ds/symlink/tps62130.pdf), pp. 14–15; [WT02C40C specification](https://www.fanstel.com/s/WT02C40C-Product-Specifications-agy4.pdf), p. 17.

3. **U7 antenna region has no copper exclusion.** U7 is on B.Cu, within the board, with a board-wide GND pour on both copper layers and no rule-area keepout. The copper plots show ground continuing beneath the module antenna region. This can detune/shield the antennas and impair Wi-Fi/BLE connectivity; it does not imply MCU power-up failure. Apply the manufacturer's antenna-area keepout on both layers, preserving the required ground beneath the castellated/body region, or move the antenna portion beyond the board edge. Do not clear ground beneath the entire module. Source: [Fanstel specification](https://www.fanstel.com/s/WT02C40C-Product-Specifications-agy4.pdf), Host Board Design and Notes on Antenna and PCB Layout.

4. **Assembly specifications remain incomplete.** U4 is the generic fixed-output family value TPS7A20xxxDBV; explicitly select the 3.3 V variant, such as TPS7A2033PDBVR, before ordering. F1 has no current rating; Y1 has no exact load-capacitance/ESR part specification. The BOM gives no capacitor voltage/DC-bias specifications. In particular verify effective capacitance of C17 (22 µF/0402), C35 and C64, and STM32 VCAP capacitors. These are unresolved purchasing/assembly conditions rather than confirmed copper errors. [TPS7A20 datasheet](https://www.ti.com/lit/ds/symlink/tps7a20.pdf).

## DRC/ERC interpretation

- Refilled DRC: 34 violations (26 errors, 8 warnings), zero unconnected items. Schematic parity adds four warnings, exclusively PCB-only mounting holes H1–H4. No electrical net mismatch was reported.
- Errors: ten differential-gap reports (including repeated item pairs) at 0.15 mm versus the custom 0.20 mm requirement; two uncoupled analog-pair length reports (18.94 and 15.58 mm versus 5 mm); ten SW trace-width reports (0.25 versus 0.40 mm); four U3 thermal-pad holes (0.20 versus board minimum 0.30 mm).
- Analog-pair findings are primarily noise/CMRR concerns, not evidence of a nonfunctional prototype. Actual 0.15 mm copper spacing must be supported by the chosen fabrication process.
- U3 0.20 mm holes need explicit fabrication support or a footprint revision; do not simply ignore a manufacturing limitation.
- Warnings comprise six silkscreen issues and two +3.3 V dangling vias. No disconnected functional pads were found after refill.
- Saved-fill DRC reports 36 violations and zero unconnected items. Refill reduces the count to 34. Refill and regenerate fabrication files after any accepted changes; existing Gerbers were not validated against this review.
- ERC reports 7 errors and 8 warnings. Four errors are U2 IN3P/N and IN4P/N nets containing only their own ADC pins. Three errors report power nets without output-power pin declarations. The exported netlist nevertheless contains physical power paths through regulators/passives. Do not equate those ERC declarations with missing copper power connections.
- Ground-name warning reflects GND/GNDA being intentionally joined; this is not a short between independent voltage rails.

## Basic features and checks

- Current hardware is a two-channel sensing board with STM32, IMU, USB, radio and battery charger. J1/J2 connect ADS channels 1/2. Channels 3/4 have labels but no external signal paths; 5–8 are explicitly unused. There is no stimulation output stage in this revision. components.md describes an older design and is not authoritative.
- STM32 supply pins, PDR_ON, BOOT0 pulldown, NRST pullup/capacitor, SWD, and both 2.2 µF VCAP networks are present. ADC/IMU SPI paths and radio UART/SWD connections are present. This confirms connectivity, not firmware operation.
- Buck feedback nominal output is 0.8 × (1 + 316k/100k) = 3.328 V.
- USB-C CC1 and CC2 each have 5.1 kΩ pulldowns; D+/D− connect to STM32 PA12/PA11. Verify firmware clocking and VBUS-sense configuration during enumeration testing.
- The BAV99 physical pad wiring is appropriate: pad 1 GND, pad 2 +3.3VA, pad 3 analog input. Embedded symbol pin-function names are misleading; do not reverse the actual PCB connections based on those names. [Nexperia pin table](https://assets.nexperia.com/documents/data-sheet/BAV99.pdf), p. 2.
- ADS internal clock is selected. Firmware must enable/configure reference and RLD appropriately, observe power-on/reset timing, and disable/short unused channels internally. Start with the ADC internal test signal, then an external differential test signal biased within the permitted input common-mode range. [ADS1298 datasheet](https://www.ti.com/lit/ds/symlink/ads1298.pdf).
- Battery-only operation cannot maintain regulated 3.3 V over the entire discharge curve: U3 is buck-only and U4 is an LDO. Check operation as battery voltage falls, particularly against the module's 2.9 V minimum VDD. This is a supply-headroom limitation, not necessarily an immediate startup defect.

## Minimal bench acceptance sequence

1. Confirm assembled parts, polarity, and absence of rail-to-ground shorts. Start with a current-limited 5 V source, no electrodes, and no battery.
2. Measure charger OUT/VCC (nominal 4.4 V with valid USB input), EN2 (>1.4 V), digital rail (~3.33 V), analog rail (~3.3 V), and both STM32 VCAP pins. Check for heat or oscillation.
3. Connect STM32 SWD and run simple firmware using the internal oscillator first. Then test the external clock and USB enumeration.
4. Read ADS1298 and IMU IDs; test ADC internal signal, then channels 1/2 with a suitable bench source.
5. Program U7 and test UART, BLE and Wi-Fi while observing the 3.3 V rail for dips and watching reset causes. Repeat with USB-only and a charged battery.
6. Test low-battery behavior separately. Bench operation does not establish suitability for human-connected testing.
