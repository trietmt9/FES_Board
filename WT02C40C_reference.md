# WT02C40C Application / Reference Design

Fanstel **WT02C40C** = Nordic **nRF5340** (BLE 5.4 / Thread, dual Cortex-M33) + **nRF7002**
(Wi-Fi 6, 2.4/5 GHz) combo module, chip antennas. Powered at **3.3 V**. DC-DC inductors and
power-up sequencing are embedded — feed 3.3 V only, no external coils.

Datasheet: https://www.fanstel.com/wt02e40e  (pin functions pp.13-16)
KiCad part: library nickname `Fanstel_WT`, symbol/footprint `WT02C40C`.

## Power rails (all 3.3 V)
| Pin | Net          | Connect      | Notes |
|-----|--------------|--------------|-------|
| 9   | VDD          | +3V3         | Main nRF5340 supply (1.7-3.6 V) |
| 17  | Vbat-3.3V    | +3V3         | Powers nRF7002 Wi-Fi (BUCKVBAT). Required for Wi-Fi |
| F4  | VDDH         | +3V3         | Tie to +3V3 for normal mode |
| F6  | VBUS         | +5V or NC    | Only if using USB (internal 3.3 V reg). Leave NC otherwise |
| 10, A0/B0/C0/D0, H0-H3 | GND | ground | H0-H3 = big thermal/ground pads, via to GND plane |

## Decoupling (per Fanstel datasheet Ver 0.96, Oct 2024)
Datasheet "Suggestion for Battery Power Application": VDD DC-DC and VDDH DC-DC
inductors+caps are EMBEDDED. "No external component is required." Do NOT add external
caps on VDDH (F4) - internal.
Design Note (2): supply must be free of AC ripple. For NOISY supplies, add a ferrite in
series + a bypass cap to ground of >= 47 uF directly at the module.
Practical for this board (mixed-signal EMG/FES):
- >= 47 uF bulk cap (47-100 uF, 6.3 V+ X5R/tant) at VDD (pin 9) + VBAT (pin 17) feed.
- 100 nF close to pin 9 (cheap HF insurance, not mandated).
- Ferrite bead in series from main 3.3 V ONLY if the rail is shared/noisy.
- Nothing on VDDH (embedded).

## SWD debug / programming
  SWDIO -> pin 16
  SWCLK -> pin 15
  RESET -> pin 14   (active-low, internal pull-up)
  VREF  -> +3V3
  GND   -> GND
Recommended on RESET: 10k pull-up to +3V3 + 100nF to GND (+ optional button to GND).

## DO NOT CONNECT - reserved for internal nRF5340<->nRF7002 link/coexistence
(datasheet "NC for WT02C40C, used internally")
  pin 13 (P0.30 COEX_Status0)
  B5 (P0.12 BUCKEN), C1 (P0.31 IOVDD ctrl), C3 (P0.24 COEX_Grant),
  C4 (P0.23 Host IRQ), D3 (P0.28 COEX_REQ), D4 (P0.29 COEX_Status1)
  QSPI bus P0.13-P0.18 (grid pins B2/B3/B4/B6/F3 + clk) = internal QSPI to nRF7002
Leave all of the above unconnected.

## UART link to STM32F767 (assigned 2026-06-30)
NOTE: WT02C40C has NO dedicated UART pin. Per Fanstel datasheet Ver 0.96 (Oct 2024),
the nRF5340 routes UARTE to ANY free GPIO via firmware pinctrl. Pins 11 (P0.09) and
12 (P0.11) are both plain GPIO (verified in the datasheet pin table) so they are a
valid free pair; TX/RX direction is your firmware's choice, not fixed by hardware.
Datasheet source: https://fcc.report/FCC-ID/X8WWT02C40C/7885630.pdf
STM32 USART2 chosen (both pins were free in the design). Net labels added to schematic:
  STM32 PA2 (pin 36, USART2_TX) = net STM32_NRF_UART_TX  -> WT02 RX (pin 12, P0.11)
  STM32 PA3 (pin 37, USART2_RX) = net STM32_NRF_UART_RX  -> WT02 TX (pin 11, P0.09)
  Common GND. Both 3.3 V, direct connect, no level shifter.
Wire the WT02 when placed: connect WT02 pin 11 -> STM32_NRF_UART_RX, WT02 pin 12 -> STM32_NRF_UART_TX.
nRF firmware: map UARTE TX=P0.09, RX=P0.11 in devicetree pinctrl.
Other free USARTs if needed: USART6 (PC6/PC7), UART8 (PE0/PE1). Avoid USART3 PD8/PD9 (IMU INT) and PA9.

## Free I/O (safe to use)
  I2C:    pin 1 (P1.02/SDA), pin 2 (P1.03/SCL)
  Analog: pin 5 (P0.04/AIN0), pin 6 (P0.05/AIN1)
  USB:    E4 (D+), E5 (D-), F6 (VBUS)
  GPIO:   pin 11 (P0.09), pin 12 (P0.11), Z0-Z6, E0-E3
  NFC:    pin 7 (P0.02), pin 8 (P0.03)
