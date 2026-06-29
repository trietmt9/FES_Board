# FESboard — Main Components

## ICs & Active Components

| Ref | Part Number | Function | Package | Supply Voltage | Key Specs | Datasheet |
|-----|-------------|----------|---------|---------------|-----------|-----------|
| U1 | STM32F767ZITx | Microcontroller (MCU) | LQFP-144 | 1.7–3.6V | ARM Cortex-M7 @ 216MHz, 2MB Flash, 512KB SRAM, FPU, SPI/I2C/UART/USB | [ST](https://www.st.com/resource/en/datasheet/stm32f767zi.pdf) |
| U2 | ADS1298xPAG | Biopotential AFE / 24-bit ADC | TQFP-64 | AVDD 2.7–3.6V, DVDD 1.8–3.6V | 8-ch 24-bit delta-sigma ADC, PGA (1–12×), SPI interface, designed for ECG/EEG/EMG | [TI](http://www.ti.com/lit/ds/symlink/ads1298.pdf) |
| U3 | TPS62130A | Synchronous Buck Regulator | VQFN-16 (3×3mm) | 3–17V input, adj. output | 3A output, up to 2.25MHz switching, Power Good output | [TI](http://www.ti.com/lit/ds/symlink/tps62130.pdf) |
| U4 | MIC4604YM | Half-Bridge MOSFET Driver | SOIC-8 | 4.5–85V | 85V half-bridge driver, up to 16V programmable gate drive, 3A peak output | [Microchip](http://ww1.microchip.com/downloads/en/DeviceDoc/20005852A.pdf) |
| U5 | ICM-45686 | 6-Axis IMU | LGA (custom) | 1.71–3.6V | 3-axis accelerometer + 3-axis gyroscope, SPI/I2C/I3C, 0.86mm height | [TDK InvenSense](https://invensense.tdk.com) |

## ESD Protection Diodes

| Ref | Part Number | Function | Package | Key Specs |
|-----|-------------|----------|---------|-----------|
| D1–D8 | BAV99 | Dual series switching / ESD clamp diode | SOT-23 | 70V, 200mA, dual diode (anode-common), protects analog electrode inputs |

## Connectors

| Ref | Part Number | Function | Footprint |
|-----|-------------|----------|-----------|
| J1 | TC2030-NL | ARM SWD debug (TagConnect) | Tag-Connect TC2030-IDC-NL 2×3 P1.27mm |
| J2–J5 | PJ320D | 3.5mm audio jack — electrode input (4-pole) | Horizontal, CUI PJ320D |

## Notes

- **U2 (ADS1298)** has separate AVDD (+3.3VA) and DVDD (+3.3V) supply domains. Analog and digital grounds (GNDA / GND) must be joined at a single star point.
- **U4 (MIC4604YM)** drives the FES stimulation output stage. Its 85V rating accommodates the high-voltage pulses required for functional electrical stimulation.
- **U5 (ICM-45686)** monitors limb movement/position feedback during stimulation.
- **D1–D8 (BAV99)** are placed at each electrode input to clamp transient voltages to the +3.3VA / GNDA rails before they reach the ADS1298 inputs.
- **J2–J5** use 3.5mm audio jacks as electrode connectors (tip = signal, ring = reference, sleeve = GND).
