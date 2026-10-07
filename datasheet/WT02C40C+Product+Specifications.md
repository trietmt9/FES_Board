WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
WT02C40C Series are combo modules of a Nordic WiFi 6 nRF7002 SoC and a Nordic
nRF5340 BLE SoC. With a dual core ARM CortexTM M33 MCU, up to 128MHz, quad radio-
protocol transceivers, and various antenna options. They will be certified for BLE, 802.15.4,
2.4 GHz WiFi 6 and 5 GHz WiFi 6. It allows faster time to market with reduced development
cost, especially for Matter applications.
No external component needed to minimize host PCB area: 40MHz, 32 MHz and 32.768 KHz, -40oC to +105oC, 20 PPM
crystals are integrated. DCDC inductors for VDD and VDDH are integrated. Powering up sequence control circuitry is
embedded.
WiFi 6 Specifications • TX power: programmable +3dBm to -20dBm
•Nordic nRF7002 SoC. • BLE data rate: 2Mbps, 1Mbps, 500kbps, 125kbps.
•Low-power and secure Wi-Fi for the IoT • IEEE 802.15.4 data rate: 250 Kbps
•Supports IEEE 802.11 a/b/g/n/ac/ax • DCDC inductors for VDD, VDDH on board.
•Supports Target Wake Time (TWT), Orthogonal • Serial Wire Debug (SWD)
Frequency Division Multiple Access (OFDMA, Downlink • Over-the-Air (OTA) firmware update
and Uplink), BSS coloring • 48 General purpose I/O pins
•WiFi CERTIFIED 6TM, WiFi CERTIFIEDTM a/b/g/n/ac, WiFi • USB 2.0 full speed (12 Mbps) controller
Enhanced OpenTM. • QSPI interface
•Supports WPA3TM, WPA2TM, WPATM - Personal and • Type 2 NFC-A tag with wake-on field, Touch-to-pair
Enterprise, Protected Management Frames. support
•Supports WMM®, WMM-Power Save, WiFi Agile • Programmable peripheral interconnect (PPI)
MultibandTM, WiFi Direct®. • 12 bit/200 Kbps ADC, 8 configurable channels.
•2.4 GHz and 5 GHz dual-band or 2.4 GHz only • Up to 3x pulse width modulator (PWM)
•Adjustable TX power from +5 to +19 dBm. • Audio peripherals: I2S, digital microphone interface
•Wi-Fi 6 Station (STA) (PDM)
•1 Spatial Stream (SS) • 5 x 32 bit timers with counter mode
•20 MHz channel bandwidth • Up to 3x SPI masters/3x SPI slaves
•64 QAM (MCS7), 86 Mbps PHY throughput • Up to 2x I2C compatible 2-wire masters/slaves
•Co-existence interfaces • 2x UART (CTS/RTS)
•Carrier frequency: trimmed to +/- 2 ppm in the 5 GHz • Quadrature Demodulator (QDEC)
bands. • 2x real time counters (RTC)
BLE/802.15.4 Specifications: Miscellaneous
• nRF5340 CLAA, dual core ARM® Cortex M33 • Hybrid pins: 16 castellated and 45 LGA.
• Application Core • Modules can be “T” shape
• 128/64 MHz Cortex M33 with FPU and DSP • Operation voltage: 3.3V
instructions • Pending Certifications: (BLE, 802.15.4, 2.4 GHz WiFi,
• 1MB flash, 512KB RAM 5GHz WiFi).
• 8KB 2-way set associate cache • FCC ID: X8WWT02C40C
• ARM® TrustZone® Cryptocell-312 co-processor • ISED ID: 4100A-WT02C40C
•Network core: • CE, RCM: Compliant
• 64 MHz Cortex M33 with 2KB instruction cache • TELEC ID: 201-240778/00
• 256KB flash, 64KB RAM • Taiwan NCC: See ID numbers below
• Receiver Sensitivity: -98 dBm at 1Mbps. • QDID: 119517, 182626
• All modules support approtect features.
Model Summaries
module WT02E40E WT02C40C WT02V40V WT02E40C WT02P40P
Size W(antenna)xHxT, mm 14.0(15.5)x23.0x2.0 14.0(25.0)x24.8x2.0 14.0(16.4)x30.5x2.0 14.0(15.5)x24.5x2.0 14(15)x18.5x2.0
SoCs nRF5340+nRF7002 nRF5340+nRF7002 nRF5340+nRF7002 nRF5340+nRF7002 nRF5340+nRF7002
Embedded crystals 40M+32M+32K 40M+32M+32K 40M+32M+32K 40M+32M+32K 40M+32M+32K
BT/WiFi Antenna u.FL/u.FL Chip/Chip Chip/chip u.FL/Chip Pads/pads
2.4GHz WiFi range, iPhone 14
5GHz WiFI range, iPhone 14
Operating temp. -40oC to +85oC -40oC to +85oC -40oC to +85oC -40oC to +85oC -40oC to +85oC
NCC ID CCAL24Y11060T2 CCAL24Y11061T1 CCAL24Y11062T3 CCAL24Y11063T5 CCAL24Y11064T7
Evaluation board EV-WT02E40E EV-WT02C40C EV-WT02V40V EV-WT02E40C EV-WT02P40P
Availability Production Production 2025 Production Production
1

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Table Of Contents
1.Introduction ........................................................................................................................................................3
WT02C40C Block Diagram ...............................................................................................................................3
Module Descriptions ..........................................................................................................................................3
2.Product Descriptions .........................................................................................................................................5
Block Diagram of nRF5340 ...............................................................................................................................5
Mechanical Drawings ........................................................................................................................................7
Pin Assignments of WT02C40C ......................................................................................................................10
Pin Functions ...................................................................................................................................................12
nRF5340 and NRF7002 Internal Connection ..................................................................................................14
Using Internal Load Capacitors for the High Frequency Oscillator ..................................................................15
Powering Up Sequence ...................................................................................................................................17
Power Consumption Measurements ...............................................................................................................17
Carrier Frequency Trimming ............................................................................................................................18
Host Board Design ..........................................................................................................................................19
Mounting WT02C40C Module on the Host PCB .............................................................................................19
Evaluation Board Schematics ..........................................................................................................................20
Suggestion for Battery Power Application .......................................................................................................23
3.Firmware Development ...................................................................................................................................24
Nordic Development Tools ...............................................................................................................................24
Setting Up Development Environment ............................................................................................................25
Codes for Evaluation Boards. ..........................................................................................................................29
MAC addresses ...............................................................................................................................................29
Setting Regional Transmission Parameters ....................................................................................................30
Spectral Emission Mask and Antenna Gain .................................................................................................30
USA, Canada, and Taiwan ...........................................................................................................................30
Europe, UK, and Australia ............................................................................................................................33
Japan TELEC ...............................................................................................................................................35
4.Miscellaneous ..................................................................................................................................................38
Soldering Temperature-Time Profile for Re-Flow Soldering ............................................................................38
Cautions, Design Notes, and Installation Notes ..............................................................................................38
Packaging and Lot Number .............................................................................................................................42
Federal Communications Commission (FCC) Statement ................................................................................42
Revision History ..................................................................................................................................................44
Contact Us ..........................................................................................................................................................45
2

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
1. Introduction
WT02C40C Series are combo modules of a Nordic WiFi 6 nRF7002 SoC and a Nordic nRF5340 BLE SoC.
With a dual core ARM CortexTM M33 MCU, up to 128MHz, quad radio-protocol transceivers, and various
antenna options. They are certified for BLE, 802.15.4, 2.4 GHz WiFi 6 and 5 GHz WiFi 6. It allows faster time to
market with reduced development cost, especially for Matter applications.
WT02C40C Block Diagram
The following is a block diagram of WT02C40C. All required crystals are embedded. Connection to an external
NFC (Near Field Communication) antenna is provided. Both WiFi 6 and Bluetooth/Thread have 3 antenna
options, chip, u.FL or connection pads for an external antenna.
Module Descriptions
WT02C40C
• An nRF5340 SoC with dual core Cortex M33 MCU, up to 128 MHz
• An integrated high performance chip antenna for BLE/802.15.4.
• An nRF7002 WiFi 6 SoC supports IEEE 802.11 a/b/g/n/ac/ax for both 2.4 GHz and 5 GHz bands.
• An integrated high performance chip antenna for WiFi 6.
• Size: 14.0 (25.0 antenna area) x 24.8 x2.0 mm.
• Uses WiFi 6 in the 5GHz bands only for Matter applications.
WT02V40V
• An nRF5340 SoC with dual core Cortex M33 MCU, up to 128 MHz
• An integrated chip antenna for BLE/802.15.4.
• An nRF7002 WiFi 6 SoC supports IEEE 802.11 a/b/g/n/ac/ax for both 2.4 GHz and 5 GHz bands.
• An integrated chip antenna for WiFi 6.
• Size: 14.0 (16.4 antenna area) x 30.5 x 2.0 mm.
• Uses WiFi 6 in the 5GHz bands only for Matter applications.
WT02E40C
• An nRF5340 SoC with dual core Cortex M33 MCU, up to 128 MHz
• An integrated high performance chip trace antenna for BLE /802.15.4.
• An nRF7002 WiFi 6 SoC supports IEEE 802.11 a/b/g/n/ac/ax for both 2.4 GHz and 5 GHz bands.
• An u.FL connector for an external WiFi antenna.
• Size: 14.0 (15.5 antenna area) x 24.5 x 2.0 mm.
• Recommended for Matter applications.
3

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
WT02E40E
• An nRF5340 SoC with dual core Cortex M33 MCU, up to 128 MHz
• An u.FL connector for an external BLE/802.15.4 antenna.
• An nRF7002 WiFi 6 SoC supports IEEE 802.11 a/b/g/n/ac/ax for both 2.4 GHz and 5 GHz bands.
• An u.FL connector for an external WiFi 6 antenna.
• Size: 14.0 (15.5 antenna area) x 23.0 x 2.0 mm
• Recommended for Matter applications.
WT02P40P
• An nRF5340 SoC with dual core Cortex M33 MCU, up to 128 MHz
• Pads for BLE/802.15.4 antenna connection on the host board.
• An nRF7002 WiFi 6 SoC supports IEEE 802.11 a/b/g/n/ac/ax for both 2.4 GHz and 5 GHz bands.
• Pads for WiFi 6 antenna connection on the host board.
• Size: 14.0 (15.0 antenna area) x 18.5 x 2.0 mm
• Recommended for Matter applications.
Dual Band Antenna ANT036
•Peak gain in the 2.4 GHz band, 1.52 dBi.
•Peak gain in the 5 GHz band, 3.13 dBi.
•Antenna width x length, 17.7mm x 158.8mm.
•Includes an 100 mm, SMA to u.FL adapter cable. SMA
connector with panel mounting hardware.
4

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
2. Product Descriptions
Block Diagram of nRF5340
The following is a block diagram of Nordic nRF5340 Bluetooth Low Energy (BLE) SoC. Please visit Nordic
website for full description and product specifications.
nRF5340 is a wireless ultra-low power multiple core System on Chip (SoC) integrating two fully programmable
Arm Cortex M33 processors, advanced security features, a range of peripherals, and a multiprotocol 2.4 GHz
5

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
transceiver. The transceiver supports Bluetooth low energy, ANTTM, and 802.15.4 and allows the
implementation of proprietary 2.4 GHz protocols.
The two Arm Cortex M33 processors share the power, clock, and peripheral architecture with Nordic
Semiconductor nRF51, nRF52, and nRF91 Series of PAN/LAN SoCs, ensuring minimal porting efforts. The
application core is a full-featured Arm Cortex M33 processor including DSP instructions and FPU and running
at up to 128 MHz with 1MB of flash and 512 KB of RAM. The option to run the application processor at 64 MHz
allows the CPU to increase energy efficiency. The network core is an Arm Cortex M33 processor with a
reduced feature set, designed for ultra-low power operation. It runs at a fixed 64 MHz frequency and contains
256 KB of flash and 64 KB of RAM.
The peripheral set offers a variety of analog and digital functionality enabling single chip implementation of a
wide range of applications. Arm trustZone technology, Arm cryptoCell-312, and supporting blocks for ysytem
protection and key management are embedded for the advanced security needed for IoT applications.
6

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Mechanical Drawings
Soldering pads for WT02C40C is the same as that of the nRF5340 modules, BT40F and BT40N. Due to
different antenna designs, the sizes of modules are different.
Two types of pins are available to meet different application requirements.
• 16 castellated pins for application needing limited number of IOs. SMT equipment is not required for
soldering castellated pins.
• 45 LGA (Land Grid Array) pins to access all 48 GPIOs of nRF5340 when needed.
The following is WT02E40E mechanical drawings, top view.
7

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
WT02C40C mechanical drawings, top view.
WT02E40C mechanical drawings, top view.
8

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
WT02V40V mechanical drawings, top view.
WT02P40P mechanical drawings, top view.
9

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Pin Assignments of WT02C40C
The followings are WT02C40C pin assignment. Pin functions are in a table in next section. Please refer to
Nordic nRF5340 Product Specifications for detailed descriptions and features supported.
BT40F, BT40N, and WT02C40C have compatible footprints. Pin assignments are below. Firmware
configuration is required to upgrade module on the PCB.
BT40F pin assignments
BT40N pin assignments.
10

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
WT02C40C pin assignments. Pin 17 is 3.3V DC power input.
WT02P40P pin assignments.
11

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Pin Functions
The nRF5340 GPIO pins used to manage nRF7002 inside WT02C40C are in blue color in the Descriptions column.
These pins are NC (No Connect) on the module.
BT40N/NE has the same soldering pads as BT40F. It uses 9 GPIO pins internally to control the power amplifier,
nRF21540. These pins are described in red color in the Descriptions column. The SPI pins may be shared with other SPI
devices. Avoid using these pins as GPIO pins if you need Bluetooth range upgrade.
•WT02C40C represents WT02C40C, WT02E40C, WT02E40E, WT02P40P, and WT02V40V.
•WT02P40P only pins are marked with WT02P40P.
| BT40/BT40N | WT02C40C | nRF5340 |            |                                         |
| ---------- | -------- | ------- | ---------- | --------------------------------------- |
| pin#       | pin#     | pin#    | pin name   | Descriptions                            |
|            | 1        | 1 AE1   | P1.02      | GPIO, configured as I2C SDA on EV board |
|            | 2        | 2 AF2   | P1.03      | GPIO, configured as I2C SCL on EV board |
|            | 3        | 3 N1    | NC         | NC, 32.768 KHz crystal embedded.        |
|            | 4        | 4 R1    | NC         | NC, 32.768 KHz crystal embedded.        |
|            | 5        | 5 V2    | P0.04/AIN0 | GPIO, Analog input                      |
|            | 6        | 6 Y2    | P0.05/AIN1 | GPIO, Analog input                      |
|            | 7        | 7 W1    | P0.02/NFC1 | GPIO, NFC antenna connection            |
|            | 8        | 8 AA1   | P0.03/NFC2 | GPIO, NFC antenna connection            |
DC supply 1.7V to 3.6V. 3.3V nominal for WT02C40C Series,
|     | 9   | 9 A19 | VDD |     |
| --- | --- | ----- | --- | --- |
2.9V minimum, 3.6V maximum.
| 10  | 10  | A25 | VSS        | Ground                            |
| --- | --- | --- | ---------- | --------------------------------- |
| 11  | 11  | AJ1 | P0.09/MOSI | GPIO; BT40F, high speed SPI_MOSI. |
| 12  | 12  | AK4 | P0.11      | GPIO                              |
GPIO; NC for WT02C40C, used as COEX_Status 0 internally for
| 13  | NC  | B24 | P0.30 |     |
| --- | --- | --- | ----- | --- |
nRF7002.
14 14 AC31 /RESET GPIO, reset with internal pull up, active low.
| 15  | 15  | W31  | SWDCLK | Serial Wire Debug clock input |
| --- | --- | ---- | ------ | ----------------------------- |
| 16  | 16  | AA31 | SWDIO  | Serial Wire Debug I/O         |
To nRF7002 pin 31 BUCKVBATS and pin 25 BUCKVBAT. 3.3V
|     | 17  |     | Vbat-3.3V |     |
| --- | --- | --- | --------- | --- |
nominal, 3.6V maximum.
|     | 18  |     | GND | WT02P40P only. Ground pad for BLE RF connection. |
| --- | --- | --- | --- | ------------------------------------------------ |
19 BLE feed WT02P40P only. BLE RF connection to a host board antenna.
|     | 20  |     | GND | WT02P40P only. BLE antenna ground.  |
| --- | --- | --- | --- | ----------------------------------- |
|     | 21  |     | GND | WT02P40P only. WiFi antenna ground. |
22 WiFi feed WT02P40P only. WiFi RF connection to a host board antenna.
|     | 23  |      | GND   | WT02P40P only. WiFi antenna ground. |
| --- | --- | ---- | ----- | ----------------------------------- |
| Z0  | Z0  | AK26 | P1.09 | GPIO                                |
| Z1  | Z1  | R31  | P1.10 | GPIO                                |
| Z2  | Z2  | B20  | P1.11 | GPIO;                               |
| Z3  | Z3  | B18  | P1.12 | GPIO;                               |
| Z4  | Z4  | A17  | P1.13 | GPIO;                               |
| Z5  | Z5  | B16  | P1.14 | GPIO                                |
Z6 Z6 B14 P1.15 GPIO, Connected to nRF21540 SCK pin internally for BT40N.
| A0  | A0  | A25 | VSS        | Ground                            |
| --- | --- | --- | ---------- | --------------------------------- |
| A1  | A1  | AB2 | P0.06/AIN2 | GPIO, Analog input                |
| A2  | A2  | AD2 | P0.07/AIN3 | GPIO, Analog input                |
| A3  | A3  | AH2 | P0.08/SCK  | GPIO; BT40F, high speed SPI clock |
| A4  | A4  | AL9 | P0.16/IO3  | GPIO; BT40F, high speed SPI IO3   |
12

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
| A5  | A5  | M2  | P1.00 | GPIO |
| --- | --- | --- | ----- | ---- |
A6 A6 AL23 P1.08 GPIO, Connected to nRF21540 MOSI pin internally for BT40N.
| B0  | B0  |      | VSS       | Ground                   |
| --- | --- | ---- | --------- | ------------------------ |
| B1  | B1  | AL13 | P0.19     | GPIO                     |
| B2  | B2  | AK12 | P0.17/SCK | GPIO; BT40F, QSPI clock. |
| B3  | B3  | AK8  | P0.14/IO1 | GPIO; BT40F, QSPI IO1    |
| B4  | B4  | AK10 | P0.15/IO2 | GPIO; BT40F, QSPI IO2    |
GPIO; BT40F, high speed SPI DCX; NC for WT02C40C, used
| B5  | NC  | AK6 | P0.12/DCX |     |
| --- | --- | --- | --------- | --- |
internally as BUCKEN for nRF7002.
| B6  | B6  | AL5 | P0.13/IO0 | GPIO, BT40F, QSPI IO0 |
| --- | --- | --- | --------- | --------------------- |
| C0  | C0  |     | VSS       | Ground                |
GPIO; NC for WT02C40C, used internally as IOVDD Control for
| C1  | NC  | B22 | P0.31 |     |
| --- | --- | --- | ----- | --- |
nRF7002.
C2 C2 AL15 P0.21 GPIO; Connected to nRF21540 TX_EN pin internally for BT40N.
GPIO; NC for WT02C40C, used internally as COES_Grant for
| C3  | NC  | AL27 | P0.24 |     |
| --- | --- | ---- | ----- | --- |
RF7002.
GPIO; NC for WT02C40C, used internally as Host IRQ for
| C4  | NC  | AK20 | P0.23 |     |
| --- | --- | ---- | ----- | --- |
nRF7002.
| C5  | C5  | AK16 | P0.20 | GPIO |
| --- | --- | ---- | ----- | ---- |
GPIO, BT40F, analog input; Connected to nRF21540 MISO pin
| C6  | C6  | AK28 | P0.25/AIN4 |     |
| --- | --- | ---- | ---------- | --- |
internally for BT40N.
| D0  | D0  |     | VSS | Ground |
| --- | --- | --- | --- | ------ |
GPIO, BT40F, analog input; Connected to nRF21540 PDN pin
| D1  | D1  | AL29 | P0.26/AIN5 |     |
| --- | --- | ---- | ---------- | --- |
internally for BT40N.
GPIO; BT40F, analog input; Connected to nRF21540 RX_EN pin
| D2  | D2  | AK30 | P0.27/AIN6 |     |
| --- | --- | ---- | ---------- | --- |
internally for BT40N.
GPIO, BT40F, analog input; NC for WT02C40C, used internally
| D3  | NC  | AE31 | P0.28/AIN7 |     |
| --- | --- | ---- | ---------- | --- |
as COEX-REQ for nRF7002.
GPIO; NC for WT02C40C, used internally as COEX_Status 1 for
| D4  | NC  | U31 | P0.29 |     |
| --- | --- | --- | ----- | --- |
nRF7002.
| D5  | D5  | AK18 | P0.22 | GPIO |
| --- | --- | ---- | ----- | ---- |
| D6  | D6  | P2   | P1.01 | GPIO |
E0 E0 AL19 P1.04 GPIO; Connected to nRF21540 MODE internally for BT40N.
E1 E1 AK22 P1.05 GPIO; Connected to nRF21540 ANT-SEL internally for BT40N.
E2 E2 AL21 P1.06 GPIO; Connected to nRF21540 CSN pin internally for BT40N.
| E3  | E3  | AK24 | P1.07      | GPIO                             |
| --- | --- | ---- | ---------- | -------------------------------- |
| E4  | E4  | B2   | D+         | USB D+                           |
| E5  | E5  | B4   | D-         | USB D-                           |
| E6  | E6  | AK2  | P0.10/MISO | GPIO, BT40F, high speed SPI MISO |
| H0  | H0  |      |            | Ground pad                       |
| H1  | H1  |      |            | Ground pad                       |
| H2  | H2  |      |            | Ground pad                       |
| H3  | H3  |      |            | Ground pad                       |
| F3  | F3  | AK14 | P0.18/CSN  | GPIO, BT40F, QSPI chip select    |
High Voltage Power Supply, 2.5V to 5.5V. 3.3V for WT02C40C,
| F4  | F4  | E1  | VDDH |     |
| --- | --- | --- | ---- | --- |
engineering version.
| F5  | F5  | J1  | NC   | BT40F, No connect, L,C circuit embedded. |
| --- | --- | --- | ---- | ---------------------------------------- |
| F6  | F6  | A5  | VBUS | 5V DC power for USB 3.3V regulator       |
13

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
nRF5340 and NRF7002 Internal Connection
The WLCSP version of nRF5340 and the WLCSP version of nRF7002 are used in the WT02C40C Series
combo modules. The internal connections are shown below.
WT02C40C Internal connection
| nRF5340 CLAA |           | nRF7002 CEAA |                  |
| ------------ | --------- | ------------ | ---------------- |
| Pin          | Port name | Pin          | Pin name         |
| J6           | P0.12     | J5           | BUCK-EN          |
| K10          | P0.13     | B2           | QSPI-DATA0       |
| K9           | P0.14     | B4           | QSPI-DATA1       |
| L9           | P0.15     | A3           | QSPI-DATA2       |
| K8           | P0.16     | B6           | QSPI-DATA3       |
| L7           | P0.17     | C1           | QSPI-CLK         |
| K7           | P0.18     | C3           | QSPI-SS          |
| H3           | P0.23     | B10          | IRQ              |
| K2           | P0.24     | A11          | BT-COEX-GRANT    |
| E3           | P0.28     | B8           | BT-COEX-REQ      |
| E2           | P0.29     | A9           | SW-CTRL1/STATUS1 |
| C4           | P0.30     | A5           | BT-COEX-STATUS0  |
| C5           | P0.31     | B12          | IOVDD            |
14

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Using Internal Load Capacitors for the High Frequency Oscillator
The external load capacitors for the 32MHz crystal are not mounted. An Epson or equivalent +/-10 PPM crystal is
connected to XC1 and XC2 pins.The crystal’s CL is 8 pF. During certificate testing, we enabled
CONFIG_SOC_HFXO_CAP. When this option is used, the SoC initialization routine does not modify the XOSC32MCAPS
register value, meaning the default settings for the SoC remain in effect.
For the 32KHz crystal, external load capacitors are mounted.
The CL value of this crystal is 12.5 pF. The LFXO CAP shall be set to external when using the WT02C40C.
To setup the load capacitance, settings in the file cpu_app prj.conf:
For example:
#HFCLK
CONFIG_SOC_HFXO_CAP_DEFAULT=y
#LFCLK
CONFIG_CLOCK_CONTROL_NRF_K32SRC_XTAL=y
CONFIG_SOC_LFXO_CAP_EXTRNAL=y
15

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Alternatively, the load capacitance can be read using nrfjprog command.
16

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Powering Up Sequence
The following circuitry is embedded in WT20C40C series combo modules to control powering up sequence. Please use
P0.31 to control 3.3V power supply to nRF7002.
Power Consumption Measurements
The WT02C40C on EV-WT02C40C is set to transmit maximum power to pass FCC certification testing. The peak and the
average transmission powers are listed in the Setting Regional Transmission Parameters Section.
The worst case power consumption is when WiFi connected with Power Saving OFF. The peak current consumption
measurement is below, 270 mA. Fanstel uses a 3.3V, 300 mA voltage regulator in the EV boards.
17

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Carrier Frequency Trimming
The IEEE 802.11 specification defines the worst-case frequency offset that can be tolerated, measured in ppm.
The limit is ± 20 ppm in the 5 GHz band, and ± 25 ppm in the 2.4 GHz band. This limit ensures that demodulation is
successful for all modulation schemes even when both sender and receiver exhibit worst-case and opposite offsets. This
limit needs to hold across the full operating temperature range.
The offset is a combination of the selected crystal and the on-chip crystal oscillator circuit. The nRF7002 device includes
a programmable capacitor bank that can be configured to correct this frequency offset. The configuration of this
capacitor bank needs to be determined using calibrated test equipment, with the resulting parameter (trim value)
programmed into the One Time Programmable (OTP) memory on the nRF7002 device.
An Epson +/- 20ppm crystal is used in the WT02C40C series modules. The carrier frequency is trimmed to +/- 2 ppm in
the 5 GHz band at room temperature. We measure the carrier frequency deviation from -40oC to +85oC, temperature
reading from nRF7002, and use Epson’s crystal aging simulation. The carrier frequency of WT02C40C Series is within
+/-20ppm in the 5GHz bands for 10 years of usage.
18

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Host Board Design
EV board schematics and Gerber files can be used as reference circuit and ground plane designs.
• A 4 or more layers PCB is recommended.
• WT02C40C series soldering pad footprint is compatible with that of BT40F and BT40N (nRF5340+nRF21540). A
host PCB design can accommodate an nRF5340 module, with a power amplifier, or supporting WiFi 6.
WT02C40C has pin 17, 3.3V DC input to power WiFI 6 SoC, not needed for the BLE modules.
Mounting WT02C40C Module on the Host PCB
The mounting guidelines are applicable to modules with integrated antenna only.
• The size of antenna area varies From a module to another. A ground plane shall cover the castellated pin area of the
module or 17.6 mm from the bottom edge.
• EV-WT02C40C Gerber files can be used as a PCB layout example.
For the best Bluetooth and WiFi range performance, keep all external metal at least 30mm from the antenna area.
19

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Evaluation Board Schematics
EV-WM02C schematics, EV-WM02C Gerber, and EV-WT02P40P Schematics and Gerber files can be download. The
schematics and Gerber can be as a reference design, ground plane design, and signal routing guides of a host board
using WM02C series module. This EV board design can also be used for evaluation of the combo modules, WT02C40C
series.
Use for Evaluation of Combo Module WT02C40C
•WT02C40C is mounted at the U1 location. Pin 17 Vbat is for providing 3.3V DC power to nRF7002.
•U2 is not mounted.
•A power switch is embedded in WT02C40C. nRF5340 P0.31 is used to control the power switch to delay application of
power to nRF7002 GPIO pins during powering up.
•WT02P40P has 3-pin pads for connection to a BLE antenna on the host board. It also has 3-pin pads for for connection
to a WiFi antenna on the host board.
Use for Evaluation of Companion Module WM02C
•BT40F, an nRF5340 module is mounted at the U1 location. You can use S1 switch to select 1.8V or 3.3V DC power to
BT40F VDD, pin 9. This VDD also supplies power to WM02C VDDIO, pin 16 through a power switch, U7. BT40F P0.31
is used to control the power switch to delay application of power to WM02C GPIO pins during powering up.
•For long range Bluetooth/Thread applications, a BT40N or BT40NE can be mounted at this U1 location. VDD must be
set to 3.3V.
•The WiFi 6 module under evaluation, WM02C, is mounted at the U2 location.
•WM02C pin 8, Vbat power supply voltage is 3.3V nominal, 3.6V maximum.
•CP2102 operates at 3.3V. If you set VDDIO to 1.8V, LSF0204, U11 voltage level translator shall be enabled.
20

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
WT02C40C Schematics, page 1
21

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
EV-WT02C40C, page 2.
22

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Suggestion for Battery Power Application
Standby current consumption is important for battery-powered product. To reduce host board area, the
followings are embedded in modules:
• 32 MHz, 20PPM main crystal. You need to program the value of embedded load capacitance.
• 32.768 KHz, 20PPM sleep crystal and load capacitors.
• 40 MHz crystal for nRF7002.
• Inductors and capacitors required for VDD power supply DC to DC converter.
• Inductors and capacitors required for VDDH power supply DC to DC converter.
No external component is required. DCDC converter shall be enabled to reduce power consumption.
23

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
3. Firmware Development
Nordic Development Tools
A Nordic nRF5340 DK is recommended for programming this evaluation board. Visit Nordic website for nRR5340 DK
description and product brief.
Many application examples can be downloaded from Nordic website.
Some firmware, Android OS, and iOS app codes can be downloaded from Bluetooth 5 Codes section of this Fanstel
webpage.
http://www.fanstel.com/download-document/
BT40 firmware can be used in all nRF5340 modules without power amplifier, e.g., WT02E40C, WT02E40E, WT02C40C,
WT02V40V, WT02P40P, BT40F and BT40E. A Nordic nRF5340-DK is recommended for programming this evaluation
board. Nordic development tools can be downloaded from Nordic website.
Download and set up on your Windows PC or MAC, nRF command line tool 10.24.2 or newer.
https://www.nordicsemi.com/Products/Development-tools/nRF-Command-Line-Tools/Download
•Download and set up on your Windows PC or MAC, nRF Connect desktop 5.0.0 or newer.
https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-Desktop/Download
Use nRF Connect to program nRF5340 inside WT02C40C module.
•Connect Debug Out of the nRF5340DK to Debug In of an EV-WT02C40C.
•Connect nRF5340 DK to a PC USB port.
•Connect EV-WT02C40C to am USB power supply.
24

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Setting Up Development Environment
Use VS code, nRF Connect for desktop programmer or nrfjprog tool, either one can programming the code.
The connection between the nRF5340 and nRF7002 is the same as the PCA10143 nRF7002 DK. The default connection
is as follows:
GPIO 12 7002 Bucken
GPIO 13 qspi io0
GPIO 14 qspi io1
GPIO 15 qspi io2
GPIO 16 qspi io3
GPIO 17 qspi sck
GPIO 18 qspi csn
GPIO 19 uart0 rts
GPIO 20 uart0 tx
GPIO 21 uart0 cts
GPIO 22 uart0 rx
GPIO 23 host ireq
GPIO 24 radio grant
GPIO 28 radio req
GPIO 29 radio swctrl1
GPIO 30 radio status0
GPIO 31 7002 iovdd-ctrl
GPIO 106 led0
GPIO 110 radio btrf-switch
You can create a new project from Nordic's default WiFi example projects, such as WiFi_shell or WiFi_station, to try out
the integration. If you need to set up the hardware peripherals, please configure the board using a .overlay file.
25

UNII-1 5200 MHz 0 0 10.14 19.66
WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
The following is an example for setup the nRF7002_nrf5340_cpuapp.overlay file.
&pinctrl {
myuart0_default: uart0_default {
group1 {
psels = <NRF_PSEL(UART_TX, 0, 20)>,
<NRF_PSEL(UART_RTS, 0, 19)>;
};
group2 {
psels = <NRF_PSEL(UART_RX, 0, 22)>,
<NRF_PSEL(UART_CTS, 0, 21)>;
bias-pull-up;
};
};
myuart0_sleep: uart0_sleep {
group1 {
psels = <NRF_PSEL(UART_TX, 0, 20)>,
<NRF_PSEL(UART_RTS, 0, 19)>,
<NRF_PSEL(UART_RX, 0, 22)>,
<NRF_PSEL(UART_CTS, 0, 21)>;
low-power-enable;
};
};
};
&uart0 {
26

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
status = "okay";
current-speed = <115200>;
pinctrl-0 = <&myuart0_default>;
pinctrl-1 = <&myuart0_sleep>;
pinctrl-names = "default", "sleep";
};
&uart1 {
status = "disabled";
};
&spi4 {
status = "disabled";
};
&pwm0 {
status = "disabled";
};
&i2c1{
status = "disabled";
};
&led1{
status = "disabled";
};
&button0{
status = "disabled";
};
&button1{
status = "disabled";
};
After finish setup, build and flash the code.
The code is running.
27

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Use RTT Viewer log or UART to monitor the log to make sure the code is running normally.
28

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Codes for Evaluation Boards.
The codes preloaded into BT40F module to manage WM02C module on the evaluation board can be downloaded.
MAC addresses
Each WT02C40C module is programmed with two MAC addresses. The MAC addresses are pre-programmed into the
One Time Programmable(OTP) memory.
The address can be read by Nordic Wi-Fi radio test project command:
“wifi_radio_ficr_prog_read_params”
29

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Setting Regional Transmission Parameters
Spectral Emission Mask and Antenna Gain
Use backoff to reduce the transmit power on the band edge channels to be within regulatory limits.
In general, a device transmits at the maximum possible power for a given data rate. When using this maximum power for
band edge channels, the regulatory requirements for Spectral Emission Mask (SEM) might be violated. For such cases,
the transmit power is reduced by a particular value called backoff to meet SEM requirements. The backoff is applied only
for edge channels of a frequency band defined by the regulatory domain.
An example command to back off by 4 at the lower edge.
CONFIG_NRF700X_BAND_2G_LOWER_EDGE_BACKOFF_HT=4
An example command to back off by 5 at the higher edge.
CONFIG_NRF700X_BAND_2G_UPPER_EDGE_BACKOFF_DSSS=5
If a higher gain antenna is used, it can be compensated by reducing gain in firmware. The following command reduce
gain by 3 dB in the 2 .4 GHz frequency band.
CONFIG_NRF700X_ANT_GAIN_2G=3
USA FCC, Canada ISED, and Taiwan NCC have band edge specifications. Additional back off is required at the edge
channels. The gain is set to zero because no reduction is required.
Europe CE and UK, Australia RCM, and Japan TELEC do not have band edge specifications. Back off setting is zero.
However, the maximum transmission power is lower. A non zero setting is required for gain to reduce transmission power.
USA, Canada, and Taiwan
In the 2.4 GHz bands, the Gain and BackOff settings to pass FCC, ISED, and NCC specifications are below.
The conducted transmission powers measured in dBm for average and peak transmission power at room temperature
are listed below.
Settings
Mode Freq. Gain BackOff Avg. TX Peak TX.
802.11b 2437 MHz 0 5 16.09 19.90
802.11g 2437 MHz 0 5 12.26 21.56
802.11n HT20 2437 MHz 0 4 15.46 22.10
802.11ax HE20 2437 MHz 0 6 15.80 21.71
For your products to pass FCC, ISED and NCC specifications in the 2.4 GHz bands, use Nordic SDK 2.6.1 or newer and
include the following in your application codes. The peak gain for WM02V antenna is +4.28 dBi in the 2.4 GHz band. No
transmission power reduction is required by firmware. A back off parameter must be programmed for each radio
protocol.
The following CONFIG statements in .txt format can be downloaded.
30

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
CONFIG_NRF700X_ANT_GAIN_2G=0
CONFIG_NRF700X_BAND_2G_LOWER_EDGE_BACKOFF_DSSS=5
CONFIG_NRF700X_BAND_2G_LOWER_EDGE_BACKOFF_HT=4
CONFIG_NRF700X_BAND_2G_LOWER_EDGE_BACKOFF_HE=6
CONFIG_NRF700X_BAND_2G_UPPER_EDGE_BACKOFF_DSSS=5
CONFIG_NRF700X_BAND_2G_UPPER_EDGE_BACKOFF_HT=4
CONFIG_NRF700X_BAND_2G_UPPER_EDGE_BACKOFF_HE=6
In the 5GHz frequency bands, the Gain and BackOff settings to pass FCC, ISED, and NCC specifications are below.
The conducted transmission powers measured in dBm for average and peak transmission power at room temperature
are listed below.
Settings
| Mode    | Freq.    | Gain | BackOff | Avg. TX. | Peak TX |       |
| ------- | -------- | ---- | ------- | -------- | ------- | ----- |
| UNII-1  | 5200 MHz |      | 0       | 0        | 9.16    | 19.68 |
| UNII-2A | 5200 MHz |      | 0       | 0        | 9.68    | 19.49 |
| UNII-2C | 5200 MHz |      | 0       | 0        | 10.98   | 19.59 |
| UNII-3  | 5200 MHz |      | 0       | 0        | 11.18   | 20.45 |
| Mode    | Freq.    | Gain | BackOff | Avg. TX. | Peak TX |       |
| UNII-1  | 5300 MHz |      | 0       | 0        | 9.10    | 19.80 |
| UNII-2A | 5300 MHz |      | 0       | 0        | 9.11    | 19.53 |
| UNII-2C | 5300 MHz |      | 0       | 0        | 10.19   | 19.64 |
| UNII-3  | 5300 MHz |      | 0       | 0        | 9.57    | 20.06 |
| Mode    | Freq.    | Gain | BackOff | Avg. TX. | Peak TX |       |
| UNII-1  | 5580 MHz |      | 0       | 0        | 9.32    | 19.85 |
| UNII-2A | 5580 MHz |      | 0       | 0        | 8.49    | 19.54 |
| UNII-2C | 5580 MHz |      | 0       | 0        | 9.83    | 20.00 |
| UNII-3  | 5580 MHz |      | 0       | 0        | 8.14    | 20.06 |
| Mode    | Freq.    | Gain | BackOff | Avg. TX. | Peak TX |       |
| UNII-1  | 5785 MHz |      | 0       | 0        | 9.75    | 20.32 |
| UNII-2A | 5785 MHz |      | 0       | 0        | 8.21    | 20.06 |
| UNII-2C | 5785 MHz |      | 0       | 0        | 8.39    | 20.17 |
31

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
UNII-3 5785 MHz 0 0 8.60 20.48
For your products to pass FCC, ISED and NCC specifications in the 5 GHz bands, use Nordic SDK 2.6.1 or newer and
include the following in your application codes. Gain and back off are set to zero as no round off or attenuation is
required by firmware.
The following CONFIG statements in .txt format can be downloaded.
CONFIG_NRF700X_ANT_GAIN_5G_BAND1=0
CONFIG_NRF700X_ANT_GAIN_5G_BAND2=0
CONFIG_NRF700X_ANT_GAIN_5G_BAND3=0
CONFIG_NRF700X_BAND_UNII_1_LOWER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_1_LOWER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_1_UPPER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_1_UPPER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_2A_LOWER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_2A_LOWER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_2A_UPPER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_2A_UPPER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_2C_LOWER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_2C_LOWER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_2C_UPPER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_2C_UPPER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_3_LOWER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_3_LOWER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_3_UPPER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_3_UPPER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_4_LOWER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_4_LOWER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_4_UPPER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_4_UPPER_EDGE_BACKOFF_HE=0
32

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Europe, UK, and Australia
In the 2.4 GHz bands, the settings to pass CE and RCM specifications are:
Gain=0;
BackOff=0.
The conducted transmission powers measured in dBm at normal or room temperature and extreme or low temperature
are listed below.
|         |          | Settings |         | Normal    |       | Extreme   |       |
| ------- | -------- | -------- | ------- | --------- | ----- | --------- | ----- |
| Mode    | Freq.    | Gain     | BackOff | Conducted | EIRP  | Conducted | EIRP  |
| 802.11b | 2442 MHz |          | 0       | 0 12.76   | 16.80 | 14.26     | 18.30 |
| 802.11g | 2442 MHz |          | 0       | 0 10.92   | 15.36 | 12.45     | 16.89 |
| n20     | 2442 MHz |          | 0       | 0 10.78   | 15.24 | 12.32     | 16.78 |
| HE20    | 2442 MHz |          | 0       | 0 11.70   | 16.21 | 13,22     | 17.73 |
For your products to pass CE and RCM specifications in the 2.4 GHz bands, use Nordic SDK 2.6.1 or newer and include
the following in your application codes. Gain is set to 3 to reduce TX power by 3 dB in firmware. Back off is set to zero.
The following CONFIG statements in .txt format can be downloaded.
CONFIG_NRF700X_ANT_GAIN_2G=0
CONFIG_NRF700X_BAND_2G_LOWER_EDGE_BACKOFF_DSSS=0
CONFIG_NRF700X_BAND_2G_LOWER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_2G_LOWER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_2G_UPPER_EDGE_BACKOFF_DSSS=0
CONFIG_NRF700X_BAND_2G_UPPER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_2G_UPPER_EDGE_BACKOFF_HE=0
In the 5GHz bands, the settings to pass CE and RCM specifications are below. The conducted transmission powers
measured in dBm at normal or room temperature and extreme or low temperature are listed below.
33

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
|         |          | Settings |         | Normal    |       |       | Extreme   |       |       |
| ------- | -------- | -------- | ------- | --------- | ----- | ----- | --------- | ----- | ----- |
| Mode    | Freq.    | Gain     | BackOff | Conducted | EIRP  |       | Conducted | EIRP  |       |
| 802.11a | 5240 MHz |          | 0       | 0         | 7.50  | 8.09  |           | 9.00  | 9.59  |
| HT20    | 5180 MHz |          | 0       | 0         | 6.10  | 6.70  |           | 7.60  | 8.20  |
| VHT20   | 5240 MHz |          | 0       | 0         | 7.10  | 7.86  |           | 8.60  | 9.36  |
| HE20    | 5240 MHz |          | 0       | 0         | 7.60  | 8.40  |           | 9.10  | 9.90  |
|         |          | Settings |         | Normal    |       |       | Extreme   |       |       |
| Mode    | Freq.    | Gain     | BackOff | Conducted | EIRP  |       | Conducted | EIRP  |       |
| 802.11a | 5320 MHz |          | 0       | 0         | 7.20  | 7.79  |           | 8.60  | 9.20  |
| HT20    | 5320 MHz |          | 0       | 0         | 7.00  | 7.60  |           | 8.50  | 9.10  |
| VHT20   | 5320 MHz |          | 0       | 0         | 7.00  | 7.76  |           | 8.50  | 9.26  |
| HE20    | 5320 MHz |          | 0       | 0         | 7.40  | 8.20  |           | 8.90  | 9.70  |
|         |          | Settings |         | Normal    |       |       | Extreme   |       |       |
| Mode    | Freq.    | Gain     | BackOff | Conducted | EIRP  |       | Conducted | EIRP  |       |
| 802.11a | 5700 MHz |          | 0       | 0         | 12.20 | 12.20 |           | 13.70 | 14.29 |
| HT20    | 5700 MHz |          | 0       | 0         | 11.90 | 12.50 |           | 13.40 | 14.00 |
| VHT20   | 5700 MHz |          | 0       | 0         | 12.10 | 12.86 |           | 13.60 | 14.36 |
| HE20    | 5700 MHz |          | 0       | 0         | 12.50 | 13.30 |           | 14.00 | 14.80 |
|         |          | Settings |         | Normal    |       |       | Extreme   |       |       |
| Mode    | Freq.    | Gain     | BackOff | Conducted | EIRP  |       | Conducted | EIRP  |       |
| 802.11a | 5805 MHz |          | 2       | 0         | 6.68  | 12.62 |           | 7.61  | 13.55 |
| HT20    | 5745 Mhz |          | 2       | 0         | 6.46  | 12.40 |           | 7.30  | 13.24 |
| VHT20   | 5745 Mhz |          | 2       | 0         | 5.92  | 12.03 |           | 7.27  | 13.38 |
| HE20    | 5745 Mhz |          | 2       | 0         | 6.50  | 12.65 |           | 7.51  | 13.66 |
For your products to pass CE and RCM specifications in the 5 GHz bands, use Nordic SDK 2.6.1 or newer and include
the following in your application codes. Gain is set to 5 or 6 to reduce TX power by 5 or 6 dB using firmware. Back off is
set to zero.
The following CONFIG statements in .txt format can be downloaded.
CONFIG_NRF700X_ANT_GAIN_5G_BAND1=0
CONFIG_NRF700X_ANT_GAIN_5G_BAND2=0
34

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
CONFIG_NRF700X_ANT_GAIN_5G_BAND3=2
CONFIG_NRF700X_BAND_UNII_1_LOWER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_1_LOWER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_1_UPPER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_1_UPPER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_2A_LOWER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_2A_LOWER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_2A_UPPER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_2A_UPPER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_2C_LOWER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_2C_LOWER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_2C_UPPER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_2C_UPPER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_3_LOWER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_3_LOWER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_3_UPPER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_3_UPPER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_4_LOWER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_4_LOWER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_4_UPPER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_4_UPPER_EDGE_BACKOFF_HE=0
Japan TELEC
In the 2.4 GHz bands, the settings to pass TELEC specifications are:
Gain = 0;
BackOff = 0.
For products to meet TELEC specifications, use Nordic SDK 2.6.1 or newer and include the following in your application
codes.
The following CONFIG statements in .txt format can be downloaded.
35

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
CONFIG_NRF700X_ANT_GAIN_2G=0
CONFIG_NRF700X_BAND_2G_LOWER_EDGE_BACKOFF_DSSS=0
CONFIG_NRF700X_BAND_2G_LOWER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_2G_LOWER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_2G_UPPER_EDGE_BACKOFF_DSSS=0
CONFIG_NRF700X_BAND_2G_UPPER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_2G_UPPER_EDGE_BACKOFF_HE=0
In the 5 GHz bands, the settings to pass TELEC specifications are:
Gain = 0;
BackOff = 0.
For products to meet TELEC specifications, use Nordic SDK 2.6.1 or newer and include the following in your application
codes.
The following CONFIG statements in .txt format can be downloaded.
CONFIG_NRF700X_ANT_GAIN_5G_BAND1=0
CONFIG_NRF700X_ANT_GAIN_5G_BAND2=0
CONFIG_NRF700X_ANT_GAIN_5G_BAND3=0
CONFIG_NRF700X_BAND_UNII_1_LOWER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_1_LOWER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_1_UPPER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_1_UPPER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_2A_LOWER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_2A_LOWER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_2A_UPPER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_2A_UPPER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_2C_LOWER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_2C_LOWER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_2C_UPPER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_2C_UPPER_EDGE_BACKOFF_HE=0
36

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
CONFIG_NRF700X_BAND_UNII_3_LOWER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_3_LOWER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_3_UPPER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_3_UPPER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_4_LOWER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_4_LOWER_EDGE_BACKOFF_HE=0
CONFIG_NRF700X_BAND_UNII_4_UPPER_EDGE_BACKOFF_HT=0
CONFIG_NRF700X_BAND_UNII_4_UPPER_EDGE_BACKOFF_HE=0
37

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
4. Miscellaneous
Soldering Temperature-Time Profile for Re-Flow Soldering
Maximum number of cycles for re-flow is 2. No opposite side re-flow is allowed due to module weight.
Cautions, Design Notes, and Installation Notes
Failure to follow the guidelines set forth in this document may result in degrading of the product’s
functions and damage to the product.
Design Notes
(1)Follow the conditions written in this specification, especially the control signals of this module.
(2)The supply voltage has to be free of AC ripple voltage (for example from a battery or a low noise
regulator output). For noisy supply voltages, provide a decoupling circuit (for example a ferrite in
series connection and a bypass capacitor to ground of at least 47uF directly at the module).
(3)This product should not be mechanically stressed when installed.
(4)Keep this product away from heat. Heat is the major cause of decreasing the life of these
products.
(5)Avoid assembly and use of the target equipment in conditions where the products' temperature
may exceed the maximum tolerance.
(6)The supply voltage should not be exceedingly high or reversed. It should not carry noise and/or
spikes.
(7)this product away from other high frequency circuits.
38

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Notes on Antenna and PCB Layout
(1)Don’t use a module with internal antenna inside a metal case.
(2)For PCB layout:
• Avoid running any signal line below module whenever possible,
• No ground plane below antenna,
• If possible, cut-off the portion of main board PCB below antenna.
Installation Notes
(1)Reflow soldering is possible twice based on the time-temperature profile in this data sheets. Set up
the temperature at the soldering portion of this product according to this reflow profile.
(2)Carefully position the products so that their heat will not burn into printed circuit boards or affect
the other components that are susceptible to heat.
(3)Carefully locate these products so that their temperatures will not increase due to the effects of
heat generated by neighboring components.
(4)If a vinyl-covered wire comes into contact with the products, then the cover will melt and generate
toxic gas, damaging the insulation. Never allow contact between the cover and these products to
occur.
(5)This product should not be mechanically stressed or vibrated when reflowed.
(6)If you want to repair your board by hand soldering, please keep the conditions of this chapter.
(7)Do not wash this product.
(8)Refer to the recommended pattern when designing a board.
(9)Pressing on parts of the metal cover or fastening objects to the metal will cause damage to the
unit.
Usage Condi5on Notes
(1)Take measures to protect the unit against static electricity. If pulses or other transient loads (a large
load applied in a short time) are applied to the products, check and evaluate their operation before
assembly on the final products.
(2)Do not use dropped products.
(3)Do not touch, damage or soil the pins.
(4) Follow the recommended condition ratings about the power supply applied to this product.
(5)Electrode peeling strength: Do not add pressure of more than 4.9N when soldered on PCB
(6) Pressing on parts of the metal cover or fastening objects to the metal cover will cause damage.
(7) These products are intended for general purpose and standard use in general electronic
equipment, such as home appliances, office equipment, information and communication
equipment.
39

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Storage Notes
(1)The module should not be stressed mechanically during storage.
(2)Do not store these products in the following conditions or the performance characteristics of the
product, such as RF performance will be adversely affected:
• Storage in salty air or in an environment with a high concentration of corrosive gas.
• Storage in direct sunlight
• Storage in an environment where the temperature may be outside the range specified.
• Storage of the products for more than one year after the date of delivery storage period.
(3) Keep this product away from water, poisonous gas and corrosive gas.
(4) This product should not be stressed or shocked when transported.
(5) Follow the specification when stacking packed crates (max. 10).
Safety Conditions
These specifications are intended to preserve the quality assurance of products and individual
components. Before use, check and evaluate the operation when mounted on your products. Abide by
these specifications, without deviation when using the products. These products may short-circuit. If
electrical shocks, smoke, fire, and/or accidents involving human life are anticipated when a short
circuit occurs, then provide the following failsafe functions, as a minimum.
(1)Ensure the safety of the whole system by installing a protection circuit and a protection device.
(2)Ensure the safety of the whole system by installing a redundant circuit or another system to
prevent a dual fault causing an unsafe status.
Other Cautions
(1)This specification sheet is copyrighted. Reproduction of this data sheets is permissible only if
reproduction is without alteration and is accompanied by all associated warranties, conditions,
limitations, and notices.
(2)Do not use the products for other purposes than those listed.
(3)Be sure to provide an appropriate failsafe function on your product to prevent an additional damage
that may be caused by the abnormal function or the failure of the product.
(4)This product has been manufactured without any ozone chemical controlled under the Montreal
Protocol.
(5)These products are not intended for other uses, other than under the special conditions shown
below. Before using these products under such special conditions, check their performance and
reliability under the said special conditions carefully to determine whether or not they can be used
in such a manner.
• In liquid, such as water, salt water, oil, alkali, or organic solvent, or in places where liquid may
splash.
40

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
• In direct sunlight, outdoors, or in a dusty environment
• In an environment where condensation occurs.
• In an environment with a high concentration of harmful gas.
(6) If an abnormal voltage is applied due to a problem occurring in other components or circuits,
replace these products with new products because they may not be able to provide normal
performance even if their electronic characteristics and appearances appear satisfactory.
(7) When you have any question or uncertainty, contact Fanstel.
41

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Packaging and Lot Number
Production modules are delivered in reel, 1000 modules in each reel. Lot number for modules made
after May 2019, can be used to track silicon version of SoC, module PCB version, and production test
code version.
Federal Communications Commission (FCC) Statement
15.21
You are cautioned that changes or modifications not expressly approved by the part responsible for compliance could
void the user’s authority to operate the equipment.
15.105(b)
This equipment has been tested and found to comply with the limits for a Class B digital device, pursuant to part 15 of
the FCC rules. These limits are designed to provide reasonable protection against harmful interference in a residential
installation. This equipment generates, uses and can radiate radio frequency energy and, if not installed and used in
accordance with the instructions, may cause harmful interference to radio communications. However, there is no
guarantee that interference will not occur in a particular installation. If this equipment does cause harmful interference to
radio or television reception, which can be determined by turning the equipment off and on, the user is encouraged to try
to correct the interference by one or more of the following measures:
42

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
-Reorient or relocate the receiving antenna.
-Increase the separation between the equipment and receiver.
-Connect the equipment into an outlet on a circuit different from that to which the receiver is connected.
-Consult the dealer or an experienced radio/TV technician for help.
This device complies with Part 15 of the FCC Rules. Operation is subject to the following two conditions:
1) this device may not cause harmful interference, and
2) this device must accept any interference received, including interference that may cause undesired operation of the
device.
FCC RF Radiation Exposure Statement:
This equipment complies with FCC radiation exposure limits set forth for an uncontrolled environment. End users must
follow the specific operating instructions for satisfying RF exposure compliance. This transmitter must not be co-located
or operating in conjunction with any other antenna or transmitter.
43

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Revision History
• Feb. 2023, Ver. 0.90: Initial draft release
• April 2023, Ver.0.91: Update module photos and mechanical drawings
• April 2023, Ver. 0.92: Update module mechanical drawings.
• July 2023, Ver. 0.93: Update module photos.
• July 2024, Ver. 0.95: Update for production version.
• Oct. 2024, Ver. 0.96: Update drawings.
• Nov. 2024, Ver. 0.97: Update for production release
• Dec. 2024, Ver. 0.99: Add certification information.
• Jan. 2025, Ver. 1.00: Initial release.
• Jan. 2026, Ver. 1.01: Correct WT02E40E module description.
44

WT02C40C,WT02E40E BLE 5.4,802.15.4,WiFi6 Combo Module
Ver 1.01, Jan. 2026
Contact Us
United States:
Fanstel Corp.
7466 E. Monte Cristo Ave. Scottsdale AZ 85260
Tel. 1 480-948-4928
Fax. 1-480-948-5459
Email: info@fanstel.com
Website: www.fanstel.com
Taiwan:
Fanstel Corp.
10F-10, 79 Xintai Wu Road
Xizhu, New Taipei City, Taiwan 22101
泛世公司
臺灣省新北市汐止區新臺五路79號10樓之10, 22101
Tel. 886-2-2698-9328
Fax. 886-2-2698-4813
Email: info@fanstel.com
Website: www.fanstel.com
China:
Fanstel Technologies Corp.
11 Jiale Street
Ping-Dih, Long-Gang, Shen Zhen, GD 518117
泛世康科技(深圳)有限公司
廣東省深圳市龍崗區坪地鎮佳樂街11號
Tel. 86-755-8409-0928
Fax. 86-755-8409-0973
QQ. 3076221086
Email: info@fanstel.com
Website: www.fanstel.com
45