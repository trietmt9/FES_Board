ComputationalandStructuralBiotechnologyJournal28(2025)156–166
Contents lists available at ScienceDirect
Computational and Structural Biotechnology Journal
journal homepage: www.elsevier.com/locate/csbj
Research Article
High-resolution portable bluetooth module for ECG and EMG acquisition
|         | Luiza,b, |     | ,∗, Salviano |            | Soaresb,c,d, |           | Valenteb,e, |        | Barrosob,f, |       |     |
| ------- | -------- | --- | ------------ | ---------- | ------------ | --------- | ----------- | ------ | ----------- | ----- | --- |
| Luiz E. |          |     |              |            |              | , Antonio |             | , João |             | ,     |     |
|         | Leitãoa, |     |              | Teixeiraa, |              |           |             |        |             |       |     |
| Paulo   |          |     | , João P.    |            |              |           |             |        |             |       |     |
|         |          |     |              |            |              |           |             |        |             |       |     |
aResearch Centre in Digitalization and Intelligent Robotics (CeDRI), Laboratório Associado para a Sustentabilidade e Tecnologia em Regiões de Montanha (SusTEC),
|     |     |     |     |     |     |     |     |     |     |     |     |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
InstitutoPolitécnicodeBragança,Bragança,5300-253,Portugal
| bEngineering |     |     |       |     |     |     |       |     |       |       |     |
| ------------ | --- | --- | ----- | --- | --- | --- | ----- | --- | ----- | ----- | --- |
Department, School of Sciences and Technology, University of Trás-os-Montes and Alto Douro (UTAD), Quinta de Prados, 5000-801, Vila Real, Portugal
| cInstitute   |   of   Electronics |   and      |   Informatics |   Engineering |   of   Aveiro   (IEETA), |   University      |   of   Aveiro,   Aveiro,   3810-193, |   Portugal |     |     |     |
| ------------ | ------------------ | ---------- | ------------- | ------------- | ------------------------ | ----------------- | ------------------------------------ | ---------- | --- | --- | --- |
|              |                    |            |               |               |                          |                   |                                      |            |     |     |     |
| dIntelligent | Systems            | Associate  | Laboratory    | (LASI),       | University of Aveiro,    | Aveiro, 3810-193, | Portugal                             |            |     |     |     |
| eINESC       |                    |            |               |               |                          |                   |                                      |            |     |     |     |
|              | TEC—INESC          | Technology | and           | Science,      | Porto, 4200-465,         | Portugal          |                                      |            |     |     |     |
fINESC   TEC-Instituto   de   Engenharia   de   Sistemas   e   Computadores,   Tecnologia   e   Ciência,   Polo   da   UTAD,   5000-801,   Vila   Real,   Portugal
| A R T I C L E  |     |     | I N F O  |     | A B S T R A C T  |     |     |     |     |     |     |
| -------------- | --- | --- | -------- | --- | ---------------- | --- | --- | --- | --- | --- | --- |
Keywords: Problem: Portable ECG/sEMG acquisition systems for telemedicine often lack application flexibility (e.g., limited
configurability, signal validation) and efficient wireless data handling. Methodology: A modular biosignal
Electrocardiogram
Electromyogram acquisition system with up to 8 channels, 24-bit resolution and configurable sampling (1–4 kHz) is proposed,
|     |     |     |     |     |     |     |     |     |     |     |     |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
Printedcirc uitboard featuring per-channel gain/source adjustments, internal MUX-based reference drive, and visual electrode integrity

Acquisitionintegrity monitoring; Bluetooth® transmits data via a bit-wise packet structure (83.92% smaller than JSON, 7.28 times
Telemedicine
faster decoding with linear complexity based on input size). Results: maximum 6.7 μV input-referred noise;
| Wearable |     |     |     |     |     |     |     |     |     |     | rms |
| -------- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
harmonic signal correlations >99.99%, worst-case THD of -53.03 dBc, and pulse wave correlation >99.68%
Human-centred
in frequency-domain with maximum NMSE% of 6e-6%; and 22.3-hour operation (3.3 Ah battery @ 150 mA).
Conclusion: The system enables high-fidelity, power-efficient acquisition with validated signal integrity and
adaptable multi-channel acquisition, addressing gaps in portable biosensing.
1. Introduction accurate interpretation and clinical decision-making, easing complex
analysis.
Improvement in the analysis of humans’ daily routine through biosig-
The electrocardiogram (ECG) visually represents the heart’s func-
nal processing provides big data for earlier detection of diseases, in- tioning through electrical measurements. The resulting maximum peak
creases the effectiveness of treatment [1], and optimises athletes’ mus- voltage reaches up to 5 mV, changing depending on conditions such as
cular gain and recovery [2], allowing physicians a better follow-up in  sex and age [5]. The ECG frequency band depends on the intended objec-
cardiovascular and muscular rehabilitation.
tive. The most comprehensive typically range from 0.5 Hz to 150 Hz [6].
Multichannel biosignals’ real-time acquisition and analysis have be-
The ECG medical-grade acquisition using 10 electrodes can be uncom-
come increasingly important in various biomedical applications, includ-
fortable. Therefore, alternative configurations with only three electrodes
ing clinical monitoring, diagnostics and scientific research [3]. The need
are used [7,8], with the electrodes placed equidistant from the heart,
for robust and efficient systems that can acquire, process and transmit
these signals in real-time is fundamental to ensuring the accuracy and  known as the Einthoven triangle [7,9].
effectiveness of modern medical applications [4]. The electromyogram (EMG) represents muscle contraction through
Biosignals, such as the electrocardiogram (ECG) and electromyo- electrical measurement [10]. The surface electromyogram (sEMG) is the
non-intrusive way of acquiring EMG signals, typically using one pair of
gram (EMG), contain valuable information about an individual’s phys-
electrodes per muscle plus a reference [11,12]. The maximum amplitude
iological state. However, these signals’ complexity and unpredictabil-
ity nature require advanced tools for their acquisition and analysis.  varies with the contraction intensity, muscle, strength, sex, and age [13].
Extracting relevant characteristics from these signals is essential for  The frequency bandwidth stays within 10 to 500 Hz [11,14,10]. Its
* Corresponding author at: Research Centre in Digitalization and Intelligent Robotics (CeDRI), Instituto Politécnico de Bragança, Bragança, 5300-253, Portugal.
E-mail address :luiz.luiz@ipb.pt(L.E. Luiz).
https://doi.org/10.1016/j.csbj.2025.04.020
Received 7 March 2025; Received in revised form 10 April 2025; Accepted 13 April 2025
Availableonline15April2025
2001-0370/©2025TheAuthors.PublishedbyElsevierB.V.onbehalfofResearchNetworkofComputationalandStructuralBiotechnology.Thisisanopenaccess
articleundertheCCBYlicense(http://creativecommons.org/licenses/by/4.0/).

L.E.Luiz,S.Soares,A.Valenteetal. ComputationalandStructuralBiotechnologyJournal28(2025)156–166
applications include medical diagnosis, muscle rehabilitation and the
control of electric prostheses [11].
Some developments conceive this objective of a portable analysis of
biosignals using synthetical simulator-generated signals to test its re-
sults. In [15], the acquisition module was developed, but the signal
came from a potential simulator. Other developments depend on pre-
recorded data, such as [16] developing an interface for interpreting ECG
from pre-existing datasets, where certain frequency bands may have
already been processed, restricting some studies beyond the standard
frequency bands. Similarly, there are EMG-focused systems that start
from a pre-existing database, as in [11] or [17], use pre-existing acqui-
sition systems, as in [14], or without a focus on processing the signal
parameters, as in [10].
Some studies include hardware and interfaces but focus on a single
biosignal. In [18], a 16-bit ECG acquisition is sent via USB to a mobile
device, which then transmits it to a server where the signals are pre-
sented graphically. In [19], the development was aimed at education,
using frequency bands that prioritise the clear visual recognition of ECG
points instead of a complete signal’s frequency spectrum.
There are also developments focused on specific applications.
In [20], an sEMG acquisition was created as feedback for an electri-
cal stimulation system for rehabilitation. The data is sent to a mobile
phone using BLE (Bluetooth® low-energy), which calculates electrical
stimulation parameters based on the patient, pathology, and sEMG read-
ing.
The work [21] presented a wearable acquisition system for signals, Fig. 1 .Hardware block structure.
including sEMG and ECG, sent to a mobile phone using BLE. This system
creates a platform to predict fall occurrences and electrical stimulation These constraints limit end-users, whether through immutable hard-
routines to prevent falls. ware/software configurations or delayed data transmission.
Using ECG and sEMG as feedback in portable rehabilitation systems This work, focusing on adaptability to user workflows and environ-
is solid in the state-of-the-art to improve patient safety and personalised ments as a human-centred design principle, introduces a modular, wire-
treatment [20,21]. However, multipurpose systems that seek ECG and lessly accessible acquisition platform designed to prioritize a flexible
sEMG acquisition trade off their signal-specific analogue conditioning user operation. By integrating configurable multi-channel architecture
blocks to make the system versatile [22]. These systems require the user (1-8 channels, 24-bit resolution), intuitive gain configuration, signal
to select the wanted signal before the acquisition, as in [23]. source adjustments, and electrode integrity monitoring, the system seeks
From the presented systems, it is clear that there is a preference for to allow clinicians and researchers to flexibly and reliably acquire bio-
BLE protocol in wireless communication to reduce power consumption. logical signals. Simultaneously, its optimized bit-wise Bluetooth®packet
However, BLE is preferable in applications where the data is sparse in structure aims to overcome traditional bandwidth limitations.
time, meaning that for this application, its reduced consumption is dep- The work is divided into this introduction, followed by a section
recated. At the same time, BLE also has a reduced data transmission focused on the module design, explaining the hardware developed, the
volume, constraining larger sampling frequencies or data resolution. firmware algorithm, and the interface to control the module and receive
Another topic that requires improvement is the resolution of signal the data. Then, the results are shown as validation of the acquired data,
conversion, where common wireless commercial devices support a max- followed by the conclusion.
imum of 10-bit conversion with only 4 channels [24], 14-bit sampled at
300 kHz for intermittent 6 leads [25], or 24-bit with only 1 channel [26]. 2. Design
Although wireless signal transmission brings the added value of not
connecting the user to a third-party device, wireless protocols are lim- The proposed system encompasses the acquisition module (i.e., hard-
ited regarding the size of data packets. This leads developers to choose ware), microcontroller algorithm (i.e., firmware) and the user interface
lower-resolution ADC, smaller sampling frequency or fewer channels to control the acquisition and visualize the results. The design section is
to comply with this size constraint. However, acquisition modules de- equally divided to expose the methods and the resulting development.
veloped for electrophysiology signal acquisition should have an input-
referred noise of less than 5 μV [27], not achievable with less than 2.1. Hardware description
rms
20-bit resolution ADC with a symmetrical range of 2.4 V without ana-
logue gain (a least significant bit of 4.58 μV ). Therefore, if a smaller Oriented by the requirements for the PCB (printed circuit board), a
rms
number of analogue conditioning parts is defined as objective, the ADC hardware block design was developed as a reference for the develop-
should have a resolution higher than 20-bit. This also increases the mod- ment, as shown in Fig. 1.
ule flexibility, as it becomes less dependent on immutable hardware From the PCB external connection, medical-grade compatible cables
Regarding flexibility within the frequency band of conditioning, com- are used to maintain flexibility, with a snap-button connector for the
mercial devices such as BioPac MP35 and BITalino (r)evolution have electrode and D-sub for the PCB. Furthermore, an audio jack input is
predefined conditioning circuits that limit studies that explore flexible also included as one of the channels for an easily ready-to-use cable
bands [28]. Another lack of flexibility is noticed with possible sampling with the same snap-button connector for the electrodes but with a 3-
frequencies, kept at a maximum of 1 kHz [24] (minimum sEMG sam- pole audio output to connect to the PCB. This ambiguity facilitates user
pling frequency [11]) to fit in wireless bandwidth, constraining other interaction when using the module, depending on the number of re-
signal analyses [25]. Commercial devices also limit the cables that quired channels, adapting the module to the real and different needs of
the system is compatible with instead of ensuring compatibility with users as a human-centred design principle. Commercial devices lack this
medical-grade certified cables. flexibility, making the module require a company-specific cable with
157

L.E.Luiz,S.Soares,A.Valenteetal. ComputationalandStructuralBiotechnologyJournal28(2025)156–166
less certification than medical grade (e.g., BioPac MP35 and BITalino
(r)evolution [24]).
Within the PCB, the initial objective is to have multiple channel in-
puts, a high-resolution analogue-to-digital converter (ADC) and easily
configurable parameters. Based on the state-of-the-art, the ADS129x in-
tegrated circuit (IC) family was selected as the analogue front end of
the acquisition module. This IC family has a 24-bit resolution ADC,
delta-sigma modulation, highly configurable with programmable gain
(1 to 12), configurable sampling frequency, and internal multiplexers
for Right-Leg-Drive (RLD) configuration and Wilson Central Terminal
(WCT). Multiplexing the input to read different PCB-internal features,
such as input noise, power supply, and temperature, is also possible.
The supply was projected to focus on portability, requiring one
18650 3350 mAh Li-Ion battery with a charge pump voltage inverter
for the negative voltage and voltage regulator for the different ADS129x
supply requirements. Namely, in this application, the ADS129x requires
a symmetrical analogue supply (+ -2.5 V) and a digital supply (+ 3 V), Fig. 2 .Track of the developed microcontroller firmware as a block diagram.
and the microcontroller requires another digital supply (+ 3 .3 V).
A linear voltage regulator was used for every positive voltage supply.
The negative value was achieved by inverting the battery voltage and
then connecting it to a negative voltage regulator. This order is required
as the inverter is not regulated, but the ADS129x requires a highly stable,
noise-free supply to achieve good results.
All the inputs were connected to an antialiasing RC (resistor-
capacitor) low-pass filter with a cut-off frequency of 10 kΩto reduce the
signal bandwidth. The path is then interrupted by channel-independent
mechanical switches that turn the inputs manually on and off if required.
If the channel is not used, the switch can be turned off, and the input’s
positive and negative sides will be short-circuited to the analogue posi-
tive supply, as recommended by the IC manufacturer. This change does
not affect the ADC channels, which remain active if required.
The chosen microcontroller was the ESP32, which, based on the liter- Fig. 3 .Representation of correlation between analogue and digital values. (a)
ature, provided embedded wireless communication circuitry with flex- represents the original translation in the ADS129x’s output. (b) represents the
ibility regarding communication protocols, with high operating clock resulting translation after the distortion correction function is applied.
frequency, reaching 240 MHz with two independent cores. For the PCB,
its ESP32-DevKitC-32D model was used to provide easy access to exter- ADS129x GPIOs and input individual connection (i.e., contains informa-
nal pins as a prototype design. tion regarding connection condition of every positive and negative elec-
As the ADS129x digital supply is +3 V and, therefore, its SPI protocol trode). This function also initiates a tertiary function to correct polarity
drives this 3 V as a digital high, a logic level translator was used to ease inversion on the binary to voltage scales. As shown in Fig. 3, the neg-
digital communication between the ADS129x and the ESP32, since its ative and positive analogue values are inverted in the digital scale; the
digital high is 3.3 V. Although communication could result in success positive analogue values vary between 0x000000 and 0x7FFFFF, and the
based on the susceptibility to lower voltages, this reduces pressure and negative analogue values are placed from 0x800000 until 0xFFFFFF. The
allows viable higher communication frequency [29]. tertiary function adjusts this inversion to maintain the values linearly.
The PCB was designed with low-tolerance surface-mount compo- It is made by subtracting 0x800000 from values higher than 0x7FFFFF
nents to increase reliability and linearity and reduce weight and size. and adding 0x800000 to values lower than 0x7FFFFF
While the algorithm waits for interruption, it enters the main loop.
2.2. Firmware description From this point, the algorithm has different branches depending on the
situation.
The firmware follows the algorithm summarised in Fig. 2. The send data function is activated when the send flag is on dur-
After the power-up, the algorithm starts the setup function. The setup ing the loop function. It transmits the transmission buffer through the
function initializes the Bluetooth® device, then initiates the ADS129x Bluetooth®channel.
reset routine configuring the IC registers. Finally, it creates the inter- The read and write registers are activated inside different secondary
ruption that represents the data-ready function. functions or through user requests. They are programmed to aid in the
The data ready function is an interruption attached to the DRDY pin ADS129x register configuration while maintaining the SPI-specific tim-
from the ADS129x. The “read ADS129x output” function is activated ing requirements.
when a falling edge is detected. This function concatenates every sam- The ADS129x reset function can also be activated from inside the
ple in the acquisition buffer and increases the sample counter. Suppose loop when the GUI resets or initiates, forcing the ADS129x to initiate in
the sample counter reaches the number of messages per package (40 a known state. All the configuration is done register-wise.
messages, each comprising one time and one sample of each requested The ADS129x initial state is defined as a known base when the sys-
channel, e.g., an ECG and a sEMG sample). In that case, this function tem initiates, or the module receives a reset command. In CONFIG1
will clear the counter, copy the information from the acquisition buffer (0x01), the output data rate (i.e., sampling rate) is set to 1 kSPS (0x85).
to the transmission buffer and activate the sending flag. In CONFIG3 (0x03), the RLD buffer is turned on for RLD usage, and the
The read ADS129x output data function is activated inside the data reference voltage is set to 2.4 V (0xDC). In CONFIG4 (0x17), the com-
ready function. It reads the ADS129x SPI output, which comprises 24 parators used for electrode connection integrity are turned on (0x02).
bits per available channel (i.e., 4 channels in ADS1294, 6 in ADS1296(R) In LOFF (0x04), the electrode connection integrity drive is defined as
and 8 in ADS1298(R)), plus 24 status bits that have information on the an AC source signal with a 6 nA current and a threshold of 95% (0x03).
158

|     |     |     |     |     |     |     |     |
| --- | --- | --- | --- | --- | --- | --- | --- |
L.E.Luiz,S.Soares,A.Valenteetal. ComputationalandStructuralBiotechnologyJournal28(2025)156–166
Table 1 Since the objective was to control the module interactively, this ini-
Register map configured as the firmware’s initial state. tial interface is compatible with only two parallel graph plots.
It was developed using an object-oriented algorithm in an event-
| Address   | Register |     | Bit   |     |     |     |     |
| --------- | -------- | --- | ----- | --- | --- | --- | --- |
driven approach, attaching specific functions to specific GUI events and
|     |     |     |     |     |     |       |     |
| --- | --- | --- | --- | --- | --- | ----- | --- |
7 6 5 4 3 2 1 0 a respective priority between functions. Therefore, the GUI algorithm is
0x00   ID1 x   x   x   x   x   x   x   x   not linear, and it is inherently based on how the user intends to use the
| 0x01   | CONFIG1 |     | 1   0 |   0   0 |   0   | 1   0   1   |           |
| ------ | ------- | --- | ----- | ------- | ----- | ----------- | --------- |
|        |         |     |       |         |       |             | software. |
| 0x02   | CONFIG2 |     | 0 0   | 0 0     | 0     | 0 0 0       |           |
                    Using the module presented in this work, a sample signal was ac-
| 0x03 | CONFIG3 |     | 1 1 | 0 1 | 1   | 1 0 0 |     |
| ---- | ------- | --- | --- | --- | --- | ----- | --- |
0x04   LOFF   0   0   0   0   0   0   1   1   quired to act as an example and to be shown in the interface figures. The
|     |     |     |     |     |     |       |     |
| --- | --- | --- | --- | --- | --- | ----- | --- |
0x05 to 0x0C CHxSET 0 1 1 0 0 0 0 0 sample signal was acquired using the hardware module and recorded
|     |     |     |     |     |     |       |     |
| --- | --- | --- | --- | --- | --- | ----- | --- |
0x0D RLD_SENSP 0 0 0 0 0 0 0 1 through the interface. The CS5 lead was used for the ECG, and the ref-
| 0x0E   | RLD_SENSN |     |   0   0 |   0   0 |   0   | 0   0   1   |     |
| ------ | --------- | --- | ------- | ------- | ----- | ----------- | --- |
erence electrode served as RLD based on the other two ECG potential
| 0x0F   | LOFF_SENSP |     |   0   0 |   0   0 |   0   | 0   0   0   |     |
| ------ | ---------- | --- | ------- | ------- | ----- | ----------- | --- |
                    electrodes. For the sEMG, the two potential electrodes were connected
| 0x10 | LOFF_SENSN |     | 0 0 | 0 0 | 0   | 0 0 0 |     |
| ---- | ---------- | --- | --- | --- | --- | ----- | --- |
                    3 cm apart over the biceps, with the reference electrode connected to the
| 0x11 | LOFF_FLIP |     | 0 0 | 0 0 | 0   | 0 0 0 |     |
| ---- | --------- | --- | --- | --- | --- | ----- | --- |
0x12   LOFF_STATP   0   0   0   0   0   0   0   0   elbow, with two medium-intensity consecutive contractions. The signal
|     |     |     |     |     |     |       |     |
| --- | --- | --- | --- | --- | --- | ----- | --- |
0x13 LOFF _STATN 0 0 0 0 0 0 0 0 had a sampling frequency of 1 kHz. The resulting signal is shown in
|        |      |     |       |         |       |             |         |
| ------ | ---- | --- | ----- | ------- | ----- | ----------- | ------- |
| 0x14   | GPIO |     | 0 0   | 0 0     | 1     | 1 1 1       | Fig. 4. |
| 0x15   | PACE |     | 0   0 |   0   0 |   0   | 0   0   0   |         |
0x16   RESP   0   0   0   0   0   0   0   0   The interface allows the user to connect, pause and disconnect from
|     |     |     |     |     |     |       |     |
| --- | --- | --- | --- | --- | --- | ----- | --- |
0x17 CONFIG4 0 0 0 0 0 0 1 0 the module Bluetooth®channel. The user can also download or clear the
|     |     |     |     |     |     |       |     |
| --- | --- | --- | --- | --- | --- | ----- | --- |
0x18 WCT1 0 0 0 0 1 0 0 1 data acquired. The user can also select which channel should appear in
| 0x19   | WCT2 |     | 1   1 |   1   1 |   1   | 0   0   0   |     |
| ------ | ---- | --- | ----- | ------- | ----- | ----------- | --- |
each of the graphs presented in the interface.
1 The ID register is a read-only register that stores information regarding the  Furthermore, when the user pauses the acquisition without discon-
device-specific characteristics within the ADS129x family. Considering that the  necting, the “Test Functionalities” button is enabled, allowing the user
IC used in this development was the ADS1296, a reading command in the ID  to enter a new screen where the ADS129x can be tested and/or visually
register would return the binary word 10011001.
edit the registers’ configuration. This new window is shown in Fig. 5.
When the user enters this configuration state, the previous screen gets
In RLD_SENSP (0x0D), the positive source of the RLD is defined as the  temporarily disabled.
channel 1 positive input (0x01). Similarly, in RLD_SENSN (0x0E), the  While in the test functionalities window, Fig. 5, the user can see
negative source of the RLD is defined as the channel 1 negative input
which of the available channels have its positive and/or negative elec-
(0x01). Then, all the channels (0x05 to 0x0C) are set to have normal
trode correctly connected to the patient body, green name representing
electrode input with a gain of 12x (0x60). Finally, in WCT1 (0x18), the  connection and red representing disconnection (i.e., in Fig. 5, both pos-
first (of three) WCT comparator is turned on and connected to the neg- itive and negative electrodes are connected in channels 1, 2 and 5, and
ative side of input 1 (0x09). In WCT2 (0x19), the second comparator is  not connected in channels 3, 4 and 6). Furthermore, since the ADS129x
turned on and connected to the negative side of input 2, and the third  in the connected device is the ADS1296 (information gathered by the
comparator is turned on and connected to the positive side of input 1  ID register and shown in the GUI’s southeast corner), channels 7 and 8
(0xD8). The final register map, including the registers kept in their reset  are disabled in Fig. 5.
values, is shown in Table 1. The user can change the channel input source between Normal Elec-
From within the loop function, the data received from the Blue- trode (as in CH1), Channel Noise Measurement (as in CH2), RLD Mea-
tooth® channel is also read. Every package is a byte representing the  surement (as in CH3), Power Supply Measurement (as in CH4), Temper-
user’s configuration in the GUI. Converting the byte to decimal: 1, 2
ature (as in CH5), Internal Test Signal (as in CH6), and drive the RLD
and 4 represent sampling frequency changes, respectively 1, 2 and 4
through the channel’s positive or negative electrode. It can also change
kSPS; 10 plays the acquisition/transmission and 11 pauses it; 12 enters  the channel-specific gain between the IC’s PGA (programmable gain)
in the system test routine and transmits to the interface all RLD, WCT  options (i.e., 1, 2, 3, 4, 6, 8 or 12 V/V).
and input connection state; 14 ends the system test routine; 20 to 28  As for the RLD information, the user can choose which positive
defines the first transmitted signal source, the possible options are the  and/or negative is required to calculate the RLD signal. Any available
status word or any of the 8 channels (considering the ADS1298); 30 to  channels, positive and negative electrodes, can be routed to the RLD
38 defines the second transmitted signal source; 201 is a register-write
driver.
command; and 255 is an ADS129x reset command. For the Wilson Central Terminal, the user can choose one electrode
Considering that the interface developed is still compatible with only  from channels 1 to 4 for each of the three WCT comparators and can
two simultaneous graphs, the channels that are not requested are turned  route the mean value of pairs of the WCT comparators to channels 4
off. Similarly, when manually acquiring the data (with no interface), the
to 7 (if available) negative electrode. This allows the user to measure
user can have between 1 and 8 channel readings, where the channels  augmented ECG leads.
that are not requested are equally turned off to reduce power consump- Finally, the user can reset the ADS129x to the initial configuration
| tion. |     |     |     |     |     |     | shown in Table 1. |
| ----- | --- | --- | --- | --- | --- | --- | ----------------- |
Furthermore, when the system test mode is activated, the acquisition  When the test functionalities window is opened, the acquisition mod-
is already paused; the system, therefore, sends the GUI data regarding  ule sends all the actual IC registers to the GUI to adjust the interface to
each positive and negative input connection status and which ADS129x  the real state. The lead-off detection status is updated every 0.5 s. After
it is (i.e., ADS1294, ADS1296 or ADS1298 and if it has respiration func- all the changes have been made, when the user closes the window, all
tionality, represented by the suffix R (ADS1298R)) for the debugging  the new configuration is sent to the hardware module, which will then
write the registers with the new values.
process.
Each register’s new data is sent to the module as a binary 8-bit word.
2.3. Controller interface Inside the microcontroller firmware, these packages are handled inside
the loop function, as mentioned in Section 2.2. For every word, one write
A GUI was developed inside the MATLAB R2021a App Designer, re- register command is sent to the ADS129x. Likewise, all the required reg-
sulting in a stand-alone application that does not require any MATLAB  isters are read through a read register command at the start of the test
subscription to use the application. functionalities. As for the lead-off continuous refresh, to avoid deliver-
159

L.E.Luiz,S.Soares,A.Valenteetal. ComputationalandStructuralBiotechnologyJournal28(2025)156–166
Fig. 4 .GUI in acquisition tab with sample signal plotted. The controls are positioned on the left, with: Bluetooth®options to connect, disconnect, pause or resume
acquisition; two selectors for the channel that will source the graphs; a sampling frequency slider for 1, 2 or 4 kHz; a graph sliding window size; and buttons to clear
or download the acquired data. On the right, the two graphs show the acquired signal in real-time. Between the graphs, when the acquisition is paused, the user can
scroll the window back to visualize the previous.
Fig. 5 . The test functionalities window in connected state and acquisition
paused.
ing two read register commands every refresh period of 0.5 s (one for
positive electrodes and another for negative electrodes), this informa-
tion is gathered from the status word present on the ADS129x’s normal Fig. 6 . Assembled module with 10-electrode medical grade cable and 3-
output data stream, mentioned in the Section 2.2. electrode to audio jack cable.
3. Results The current draw by the PCB changes regarding the number of ADC
channels activated. Excluding the current used by the microcontroller,
To validate the development, one prototype was assembled and the maximum remaining current with all 6 channels activated is 9 mA,
tested. The ADS1296 was used in this prototype. The assembled mod- reducing approximately 0.8 mA for each deactivated channel, reaching
ule is shown in Fig. 6with both electrode cables (i.e., the 10-electrode a minimum of 4.2 mA when no channel is on.
medical grade cable and the 3-electrode to audio jack cable). Two tests were made with two different 18650 3350 mAh Li-Ion
batteries fully charged, connecting the system to a receiver and sending
3.1. Battery autonomy evaluation data from all 6 channels until turning off. On the first, the module ac-
quired and sent data during 22 hours and 20 minutes and on the second,
When the module is turned on, it automatically enters Bluetooth® during 22 hours and 43 minutes.
pairing mode. In this state and with all ADC channels turned on, the
system draws a mean of 72 mA from the battery. If a receiver device is 3.2. Parallel acquisition
connected, the system enters the acquisition mode, which draws a mean
of 150 mA. In both states, 9 mA was used to power the ADS1296, the To test the system’s maximum parallel acquisition (i.e., 8 channels),
logic level translator IC, and all LDO. The ESP32 development board an acquisition was made outside of the controller interface. Since the
used the remaining current. ADS1296 was used in the prototype, the readings of channels 7 and 8
160

L.E.Luiz,S.Soares,A.Valenteetal. ComputationalandStructuralBiotechnologyJournal28(2025)156–166
Table 2
Time required per iteration to decode 90,000 data messages with the three dif-
ferent algorithms considering 40 samples per message and ‘N’ messages per
iteration.
|     | N   | Algorithm |     | RequiredTime(μs)   |     |     |     | Time   |
| --- | --- | --------- | --- | ------------------ | --- | --- | --- | ------ |

|     |     |     |     |           |          |          |     | usage(%) |
| --- | --- | --- | --- | --------- | -------- | -------- | --- | -------- |
|     |     |     |     | Mean      | Min      | Max      |     |          |
|     |     | 1   |     | 3587.53   | 307.30   | 13240.50 |     | 3.59%    |
|     |     |     |     |           |          |          |     |          |
|     | 1   | 2   |     | 187.51    | 14.80    | 1543.40  |     | 0.19%    |
|     |     |     |     |           |          |          |     |          |
|     |     | 3   |     | 211.11    | 45.40    | 842.60   |     | 0.21%    |
|     |     |     |     |           |          |          |     |          |
|     |     | 1   |     | 8048.35   | 565.50   | 41491.50 |     | 4.02%    |
|     | 2   | 2   |     | 231.94    | 15.70    | 1000.40  |     | 0.12%    |
|     |     |     |     |           |          |          |     |          |
|     |     | 3   |     | 467.99    | 78.60    | 2055.40  |     | 0.23%    |
Fig. 7 .Acquisition of the 8 simultaneous signals from the module. Channels 1
|     |     | 1   |     | 20160.77 | 1387.40 | 62911.50 |     | 4.03% |
| --- | --- | --- | --- | -------- | ------- | -------- | --- | ----- |
to 6 represent the ECG leads I, II, III, and V1, and sEMG from right and left
|     | 5   | 2   |     | 235.42 | 27.10 | 654.20 |     | 0.05% |
| --- | --- | --- | --- | ------ | ----- | ------ | --- | ----- |
flexor carpi radialis. Channels 7 and 8 are not accessible in the ADS1296 used,  3   959.20   202.30   2960.30   0.19%
resulting in a constant 0 V. A factor of 1.5 mV shifts the signals to improve
|             |     | 1   |     | 35305.62 |   2754.20 |   76631.50 |     | 3.53%   |
| ----------- | --- | --- | --- | -------- | --------- | ---------- | --- | ------- |
| visibility. |     |     |     |          |           |            |     |         |
|             | 10  | 2   |     | 222.90   | 36.50     | 628.80     |     | 0.02%   |
|             |     |     |     |          |           |            |     |         |
|             |     | 3   |     | 1469.93  | 398.50    | 3542.60    |     | 0.15%   |
were constant at zero. The remaining channels were connected respec-
tively for: ECG lead I, II, III and V1, and sEMG of right and left flexor
format, for a package with 40 samples of time, ECG and sEMG, test-
carpi radialis. The signals were offset by a factor of 1.5 mV and plotted
ing on 5400000 packages, the resulting size varied between 606 and
together. This resulted in Fig. 7.
862 bytes with a mean of 735.69 bytes. This results in a mean overhead
of 335.69 bytes.
3.3. Data formatting comparison On the receiver side, three different algorithms were tested. Two
to decode the bit-wise (one based on for loops and the other based on
The proposed approach concatenates the raw binary data in peri- vector reshaping operation) and the third as a standard JSON decode
odic sections in a constant stream of information. Therefore, it requires
algorithm.
knowledge of how the information is being arranged, synchronism to
The ‘algorithm 1’ iterates on a matrix of 400xN, where ‘N’ represents
when the packages start and end, and the package’s expected size.
the number of messages to read per iteration, casting each sample from
Hence, the information is not wrongly read.
binary to 32-bit integers (native size on MATLAB).
The transmitted packets are divided into messages; each message  The ‘algorithm 2’ process is shown in Fig. 8. It takes the 400xN ma-
is subdivided into individual samples (one for time plus one for each  trix and reshapes it to a 10x40*N, where the 4 first lines are data from
channel being read); and the samples are divided into bytes (4 bytes for  the time, the middle 3 from the ECG and the last three from the sEMG.
time and 3 bytes for each signal sample) Afterwards, the matrix is divided into three matrices containing only the
When using the controller interface, acquiring two parallel signals,  respective lines (i.e., 4x40*N, 3x40*N, and 3x40*N). To make the three
the firmware formats the data in periods of 10 bytes, embracing one  matrices compatible with 32-bit (4 bytes) integer casting, the ECG and
sample of time, ECG and sEMG in an LSB-first format with 4, 3 and 3  sEMG matrices are concatenated with one line of zeros in its least signif-
bytes, respectively. To build a package, 40 samples are concatenated in  icant byte position. These final matrices are reshaped into three vectors
a buffer with a resulting size of 400 bytes that will be sent on the data
and then cast to 32-bit integers. If a different number of channels were
stream. being acquired, the number of input lines would equally increase, reach-
To maintain data integrity, the receiver is forced to read the stream  ing a maximum of 1120 bytes per packet when acquiring 8 channels (40
in groups of 400 bytes. The Bluetooth® cache is always erased when  messages containing 4+ 3 *8 bytes).
connected to avoid residual data after the previous disconnection. This  The ‘algorithm 3’ scans the transmitted data string to find the delim-
same approach must be applied if the transmission is paused and re- iter, creating a struct with time, ECG and sEMG samples. This approach
sumed. has increased robustness compared to the bit-wise, as the receiver has a
The signal integrity was validated in 3 tests of 30 minutes, acquiring  specific delimiter to base the decoding, knowing every message begin-
signals with a sampling frequency of 1 kHz. During this test, the mod- ning and ending, thus making the package’s size irrelevant. Since the
ule was between 1 and 1.5 meters from the receiver. Further distance  JSON approach cannot stack multiple messages, ‘N’ was kept as 1, i.e.,
tests were not evaluated since standard Bluetooth protocols were imple- the algorithm is tested reading only one message each iteration. In cases
mented and real-scenario obstacles are not predictable. The validation
where the reading of multiple messages per iteration is tested (N > 1),
was made by checking the time samples received by the module. Since  the algorithm is activated inside a loop to activate it ‘N’ times.
all the timestamps were spaced in 1 ms, it is concluded that no data was  The time required by all three algorithms to decode 1 hour of the sig-
corrupted in the transmission. nal (90,000 messages) was tested, considering 40 samples per message
The communication relies on classic Bluetooth® automatic Cyclic  and 1, 2, 5, or 10 messages per iteration, achieving the result shown
Redundancy Check (CRC). If a packet does not match the CRC at the  in Table 2. Besides the direct time required to decode each iteration, it
receiver, it is automatically discarded, and a new sample is requested  is also helpful to know how much from the interval between iterations
from the sender. If this 40-sample packet fails to be sent multiple times  is used to decode the message. Based on this variable, it is possible to
and the sending vector is overwritten at the sender, the receiver will  estimate the remaining time available to process the signal (e.g., filter,
miss the complete packet (e.g., the complete 40 samples); therefore, the  feature extraction).
sync will continue. The case with only one message per iteration (N = 1) was the most
To validate the efficiency of the proposed Bluetooth® transmission  straightforward to all three algorithms, allowing a direct comparison of
in a bit-wise format, the method is compared against JSON format based  their decoding efficiency. The ‘algorithm 2’ was the most efficient, be-
on its established use in the literature. ing, on average, 21.07 times faster than the ‘algorithm 1’ and 1.38 times
In the bit-wise approach, the package size in the microcontroller  faster than the ‘algorithm 3’. The ‘algorithm 2’ also achieved the shortest
is precisely 400 bytes since it is the raw data. However, in the JSON  recorded time in a specific iteration. When comparing each algorithm’s
161

L.E.Luiz,S.Soares,A.Valenteetal. ComputationalandStructuralBiotechnologyJournal28(2025)156–166
Fig. 8 .Visual Representation of the data reshaping performed by ‘algorithm 2’.
highest required time, the ‘algorithm 3’ achieves the best result, with
the ‘algorithm 2’ in a second. As for the time usage, ‘algorithm 2’ and
‘algorithm 3’ were close at 0.19% and 0.21%, with the ‘algorithm 1’
algorithm being considerably less efficient.
However, in the case studies where it is considered that the user de-
creased the data refresh rate on the receiver, i.e., increased the number
of messages to read between iterations (N > 1), the ‘algorithm 2’ al-
gorithm stood out even more. As the number of messages per iteration
grew, the efficiency of the ‘algorithm 2’ algorithm diverged from that
of the others. In the worst scenario, it was 152.55 times faster than the
‘algorithm 1’ and 7.28 times faster than the ‘algorithm 3’.
To compare the algorithm’s complexity, the big-O notation was ap-
plied to describe how the runtime of the algorithms grows relative to
the input size. Within this approach, the log from the average time
consumed (‘T’) from each algorithm and from every input size (‘N’) is
calculated, resulting in Fig. 9, where each algorithm has a slope ‘P’ in
the complexity function
𝑂(𝑁𝑃).
For the ‘algorithm 1’, the resulting complexity is near 𝑂(𝑁), mean- Fig. 9 .Complexity from the three algorithms based on the big-o annotation. T
ing that the runtime grows linearly in function of the input size. For represents the time consumed to iterate the input of size N.
the ‘algorithm 2’, the notation shows a nearly constant time (𝑂(1)). In
the ‘algorithm 3’, although sub-linear growth, it is still near to 𝑂(𝑁),
meaning that the required time still increases with the input size.
erated from the PCB without considering external factors besides the
This result is also seen in the time usage, as the ‘algorithm 2’ is the
power-line induction on the PCB itself. This test was made by using the
only one that can constantly and significantly reduce the time when the
short-input option from the CHxSET registers, acquiring a sample signal
input increases.
of at least 30 s for each channel with the PGA configured to 1, 6 and 12
Therefore, in both the algorithms’ analysis, in terms of time usage
to analyse the PGA effect on the noise, and sampling between 1, 2 and
and in the big-O notation, ‘algorithm 2’ achieved the best results, show- 4 kHz. The result is presented in Table 3.
ing that the combination of bit-wise communication with vector reshape With the results shown in Table 3it is concluded that increasing the
operations is the most efficient. PGA gain decreases the noise. In the same aspect, the slightest noises
were measured with lower sampling frequencies. This shows an advan-
3.4. Noise analysis tage in maintaining the acquisition with the highest gain and with the
1 kHz sampling unless otherwise required.
The first validation step is to calculate the input-referred noise from The higher sampling frequency behaviour is an effect of the delta-
each channel of the PCB. This showcases how much noise may be gen- sigma architecture of the ADC. Since the ADC always samples the signal
162

|     |       |     |     |     |     |     |     |
| --- | ----- | --- | --- | --- | --- | --- | --- |
L.E.Luiz,S.Soares,A.Valenteetal. ComputationalandStructuralBiotechnologyJournal28(2025)156–166
Table 3 with the resulting file downloaded from the interface. To align the ac-
PCB input-referred noise in μV with differential inputs short-circuited and  quired signal with the digitally synthesised, the original signal is shifted
rms
resulting noise transmitted to GUI. to have its maximum point on the first index, and the synthetical is
generated as a cosine to start at maximum. Then, all the signals were
| 𝑓   | Channel   | Programmable |   Gain   |     |     |     |     |
| --- | --------- | ------------ | -------- | --- | --- | --- | --- |
𝑠
      limited to a 30 s length, and the signal normalised mean squared error
|     |     | 1   | 6   | 12  |     |     |     |
| --- | --- | --- | --- | --- | --- | --- | --- |
(NMSE%) and Pearson correlation coefficient (PCC%) were calculated
|     | CH1   | 3.1290   | 0.1624   | 0.1493   |     |     |     |
| --- | ----- | -------- | -------- | -------- | --- | --- | --- |
        in reference to the synthetical signal, in addition to the original signal
|      | CH2   | 3.0951   | 0.1383   | 0.1490                                          |         |         |     |
| ---- | ----- | -------- | -------- | ----------------------------------------------- | ------- | ------- | --- |
|      |       |          |          |   total harmonic distortion (THD [dBc]).        |         |         |     |
|      | CH3   | 3.1717   | 0.1645   | 0.0693                                          |         |         |     |
| 1kHz |       |          |          |   The NMSE% was calculated through the equation |         |         |     |
|      | CH4   | 3.0759   | 0.1364   | 0.0624                                          |         |         |     |
|      | CH5   | 3.0588   | 0.1237   | 0.0554                                          |         |         |     |
|      |       |          |          |                                                 |         |         |     |
|      | CH6   | 3.3482   | 0.1440   | 0.0554                                          | 1 ∑ (   | )       |     |
|      |       |          |          |                                                 | 𝑁       | 𝐴 𝑖−𝐵 2 |     |
|      |       |          |          |                                                 | 𝑁 𝑖=1   | 𝑖       |     |
|      | CH1   | 4.3066   | 0.2408   | 0.0883 NMSE%=                                   |         | ×100%   | (2) |
|      |       |          |          |                                                 | max(𝐴)2 |         |     |
|      | CH2   | 4.2789   | 0.1929   | 0.0936                                          |         |         |     |
|      | CH3   | 4.3864   | 0.1732   | 0.0886                                          |         |         |     |
2   kHz         where 𝐴is the synthetic signal, 𝐵 is the acquired signal, and 𝑁 is the
|     | CH4 | 4.2355 | 0.1671 | 0.0736                                               |     |     |     |
| --- | --- | ------ | ------ | ---------------------------------------------------- | --- | --- | --- |
|     |     |        |        |   length of them in samples (for 30 s, equals 30 kS) |     |     |     |
|     | CH5 | 4.2514 | 0.1804 | 0.0672                                               |     |     |     |
CH6   4.6464   0.1691   0.0792 For the PCC% (𝜌), the equation
|     | CH1   | 6.4391   | 0.2745   | 0.1218   | (         | ) ( ) |     |
| --- | ----- | -------- | -------- | -------- | --------- | ----- | --- |
|     |       |          |          |          | ∑ 𝑁 𝐴𝑖−𝜇𝐴 | 𝐵𝑖−𝜇𝐵 |     |
|     | CH2   | 6.5022   | 0.2679   | 0.1238   |           | ×     |     |
|     |       |          |          |          | 𝑖=1 𝜎𝐴    | 𝜎𝐵    |     |
  C H 3 6 . 5 7 5 5 0 . 2 7 1 4 0 . 1 1 0 3 𝜌(𝐴,𝐵)= ×100% (3)
| 4 kHz | C H 4   | 6 . 4 8 6 9   | 0 . 2 5 8 1   | 0 . 1 0 6 0   |     | 𝑁−1  |     |
| ----- | ------- | ------------- | ------------- | ------------- | --- | ---- | --- |
CH5   6.4569   0.2538   0.0943   is used, where 𝐴and 𝐵represents both signals, 𝜇is the mean and 𝜎is
|     |     |        |        |        |     |     |     |
| --- | --- | ------ | ------ | ------ | --- | --- | --- |
|     | CH6 | 6.6943 | 0.2736 | 0.0971 |     |     |     |
the standard deviation. The closer 𝜌is from 100%, the more correlated
the signals are.
at 512 kHz and then averages multiple samples to achieve the configured  For the THD, the signal is converted to the frequency domain through
data rate, when the required data rate increases (i.e., from 1 kHz to 2  FFT (fast Fourier transform). Then, the RMS voltage of the fundamental
or 4 kHz), the number of samples per average decreases, increasing the  frequency’s harmonics (𝑉 𝑘), up to the last before the Nyquist frequency,
resulting noise [30]. is measured and compared with the carrier RMS voltage (𝑉 ). This is
1
Furthermore, the noise reduction when increasing the PGA is ex-
explained by the equation
plained by the PGA bandwidth behaviour when varying the gain. Fol-
lowing the IC datasheet and the configuration proposed here, it can  ⌊ ⌋
|     |     |     |     |     | ⎛∑  | 𝑓𝑠 ⎞ |     |
| --- | --- | --- | --- | --- | --- | ---- | --- |
be established that the PGA bandwidth starts at 237 kHz when PGA  2∗𝑓1 𝑉2⎟
⎜
is 1, goes to 64 kHz with PGA 6, and reaches its minimum bandwidth  THD=10⋅log ⎜ 𝑘= 2 𝑘 ⎟ [dBc]. (4)
|     |     |     |     |     | 10  | 𝑉   |     |
| --- | --- | --- | --- | --- | --- | --- | --- |
|     |     |     |     |     | ⎜   | 2 ⎟ |     |
of 32 kHz with PGA 12. This showcases that the ADC is reducing the  ⎝ 1 ⎠
amount of noise being amplified.
The results are shown in Table 4, where the behaviour between
| With a maximum input-referred noise of 6.7 μV |     |     | , it is proven that  |     |     |     |     |
| --------------------------------------------- | --- | --- | -------------------- | --- | --- | --- | --- |
rm s higher frequency causing visually higher noise is noticeable. As fre-
| very small noise magnitudes are generated in the |     |     |  P CB and that the  |                                                                    |     |     |        |
| ------------------------------------------------ | --- | --- | ------------------- | ------------------------------------------------------------------ | --- | --- | ------ |
|                                                  |     |     |                     | quency increases, it reaches values closer to the Nyquist limit (𝑓 |     |     | 𝑠∕2),  |
higher the configured PGA, the higher the signal-to-noise ratio. Further-
more, except for the configuration of 4 kHz sampling frequency with  and since no interpolation is made, the result diverges visually from the
gain 1, all the other accomplish the required 5 μV [27]. shape of a sinusoidal wave; however, since the PCC% is kept high and
rms
Considering the worst-case of 6.7 μV , with a 24-bit (𝑅) ADC scale  the THD increases, the NMSE% is demonstrated as a visual fault.
rms
range from -2.4 (𝑉 ) to 2.4 V (𝑉 ), the resulting minimum ef- Furthermore, the higher noise on the 20 mVpp samples may be ex-
|     | 𝑅𝐸𝐹− | 𝑅𝐸𝐹+ |     |     |     |     |     |
| --- | ---- | ---- | --- | --- | --- | --- | --- |
fective number of bits (ENOB) regarding input-referred noise (𝑁 𝑖𝑟) is  plained by the function generator’s higher noise in creating low voltage
estimated by the equation sine waves, as its error is intrinsically related to the signal-to-noise ratio,
becoming more evident as the signal amplitude decreases.
|     | ⎛ 𝑁 | ⎞   |     |     |     |     |     |
| --- | --- | --- | --- | --- | --- | --- | --- |
⎜ 𝑖𝑟 ⎟, Finally, to test the ADC with frequency-domain complex signals, a
| ENOB=24−log |     |     |     | (1) |     |     |     |
| ----------- | --- | --- | --- | --- | --- | --- | --- |
2⎜ 𝑉𝑅𝐸𝐹+ − 𝑉𝑅𝐸𝐹− ⎟ m u lt i p l e- s in c   w a v e  w a s   g e n e r a t e d ,  a s   it   r e s e m b l e s  a   c a r d i a c  p u ls e- li k e
|     | ⎝ 𝑅 | ⎠   |     |     |     |     |     |
| --- | --- | --- | --- | --- | --- | --- | --- |
2 sig n a l .  T h e  s i g n a l  w as   g e n e r a t e d   in   t h e   f u n c t io n   g e n e r a t o r ,  w it h  m a x i -
resulting in an ENOB between 19 and 20 bits in the worst scenario. mum amplitude fixed at 150 mV and minimum at 0 V. The number of
side lobes was generated based on the generator aspect. The fundamen-
3.5. Data validation tal frequency 𝑓 was tested at 0.6 Hz (lower than a heart’s minimum
0
expected beats per minute (BPM)) and at 5 Hz (higher than the max-
The next validation step was to identify how trustworthy a signal ac-
|     |     |     |     | imum expected BPM). Every sinc-wave had 40∗𝑓 |     | ∗𝜋 side lobes but  |     |
| --- | --- | --- | --- | -------------------------------------------- | --- | ------------------ | --- |
quired with the module, transmitted and stored using the interface is.  0
were truncated to force one sinc-peak every 1∕𝑓 . Bo th sinc-waves were
Therefore, a known sine wave was generated using an AFG1022 Tek- 0
acquired with the module, using channel 6, 12x gain and 1 kHz sampling
tronix® function generator and connected to channel 1. The sampling
rate. The acquired and synthesised signals were compared with the same
frequency was kept at 1 kHz and with a 12-time PGA gain. The signal’s
NMSE% and PCC% in the time and frequency domains, normalising the
amplitude and frequency were tested between 20 to 400 mVpp and 1
to 200 Hz. The voltage ranges from the function generator’s minimum  FFT to calculate the NMSE%. The results are shown in Table 5.
peak-to-peak voltage with minimum noise up to the ADC saturation  Analysing the results from Table 5, with Pearson correlation coef-
ficients exceeding 96% in both the time and frequency domains and
level. As the voltage increases, the PGA makes the signal approaches the
normalised mean square errors (NMSE%) below 0.15% in time and even
| ADC saturation state (i.e., as V |     | = 2.4 V is defined in CONFIG1, the  |     |     |     |     |     |
| -------------------------------- | --- | ----------------------------------- | --- | --- | --- | --- | --- |
REF lower in frequency, the system demonstrates strong accuracy and mini-
ADS129x ADC voltage ranges from -2.4 V up to 2.4 V and the 400 mVpp
mal distortion. The slightly better performance in the frequency domain
sine varies from -200 to 200 mV, which multiplied by the PGA gain,
reaches 2.4 V), limiting the maximum amplitude in 400 mVpp. suggests that the system is particularly effective at preserving spectral
The acquired signal was transmitted to the interface, where at least  content. This proves that the system is highly capable and provides re-
30 s were stored. Then, a digital signal was generated and compared  liable signal acquisition with minimal noise and distortion.
163

|     |     |       |     |     |     |     |     |     |     |     |     |
| --- | --- | ----- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
L.E.Luiz,S.Soares,A.Valenteetal. ComputationalandStructuralBiotechnologyJournal28(2025)156–166
Table 4
Calculated normalised mean square error (NMSE%) and Pearson correlation coefficient (PCC%) from function generator si-
nusoidal wave converted to digital, transmitted and stored, compared with a digitally synthesised wave, and, total harmonic
distortion (THD [dBc]) from the original stored signal.
|     | Frequency |     | Measure |     | Amplitude |     |         |     |           |           |           |
| --- | --------- | --- | ------- | --- | --------- | --- | ------- | --- | --------- | --------- | --------- |
|     |           |     |         |     |           |     |         |     |           |           |           |
|     |           |     |         |     | 20mVpp    |     | 100mVpp |     | 150mVpp   | 200mVpp   | 400mVpp   |
|     |           |     | NMSE%   |     | 0.00627   |     | 0.00081 |     | 0.0002    | 0.00005   | 0.0001    |
|     |           |     |         |     |           |     |         |     |           |           |           |
|     | 1Hz       |     | PCC%    |     | 100.0     |     | 100.0   |     | 100.0     | 100.0     | 100.0     |
|     |           |     |         |     |           |     |         |     |           |           |           |
|     |           |     | THD     |     | -56.89    |     | -60.74  |     | -64.29    | -70.17    | -63.59    |
|     |           |     |         |     |           |     |         |     |           |           |           |
|     |           |     | NMSE%   |     | 0.01123   |     | 0.00038 |     | 0.00452   | 0.00967   | 0.00297   |
|     | 10Hz      |     | PCC%    |     | 99.96     |     | 100.0   |     | 99.98     | 99.96     | 99.99     |
|     |           |     |         |     |           |     |         |     |           |           |           |
|     |           |     | THD     |     | -56.03    |     | -62.2   |     | -66.1     | -68.31    | -65.22    |
|     |           |     |         |     |           |     |         |     |           |           |           |
|     |           |     | NMSE%   |     | 0.46047   |     | 0.32828 |     | 0.43976   | 0.07903   | 0.48533   |
|     |           |     |         |     |           |     |         |     |           |           |           |
|     | 100Hz     |     | PCC%    |     | 98.15     |     | 98.72   |     | 98.26     | 99.78     | 98.08     |
|     |           |     | THD     |     | -60.03    |     | -64.8   |     | -70.22    | -73.46    | -68.66    |
|     |           |     | NMSE%   |     | 0.59264   |     | 0.98633 |     | 1.07644   | 1.88901   | 0.93097   |
|     |           |     |         |     |           |     |         |     |           |           |           |
|     | 200Hz     |     | PCC%    |     | 98.95     |     | 97.15   |     | 96.71     | 92.76     | 97.44     |
|     |           |     |         |     |           |     |         |     |           |           |           |
|     |           |     | THD     |     | -64.32    |     | -71.3   |     | -77.08    | -76.15    | -76.52    |
Table 5 Considering the worst-case, the system maintains a strong signal
Calculated normalised mean square error (NMSE%) and Pearson correlation co- correlation (PCC% > 92%) and low harmonic distortion (THD <
efficient (PCC%) from time-domain and frequency-domain of multiple-sinc wave  -64dBc), higher than the requirements for biosignal acquisition mod-
acquired with the module compared with a digitally synthesised.
ule [32,33], confirming the effectiveness of the acquisition module for
    low-to-mid frequency analogue waveforms, which comprises the elec-
| 𝑓 0 |     | Measure |     | Domain |     |     |     |     |     |     |     |
| --- | --- | ------- | --- | ------ | --- | --- | --- | --- | --- | --- | --- |
trocardiogram [34] and electromyogram [35] bandwidth of interest.
|     |     |     |     | Time   |     | Frequency |     |     |     |     |     |
| --- | --- | --- | --- | ------ | --- | --------- | --- | --- | --- | --- | --- |
      This concludes that the system can reliably acquire, transmit and store
|         |     | MSE % |     | 0.149 396 |     | 0.000016 |     |                   |     |     |     |
| ------- | --- | ----- | --- | --------- | --- | -------- | --- | ----------------- | --- | --- | --- |
| 0.6 mHz |     |       |     |           |     |          |     | harmonic signals. |     |     |     |
|         |     | PCC   |     | 96.00     |     | 99.68    |     |                   |     |     |     |
The analysis and comparison of biosignals can be highly subjec-
|     |     |     |     |     |     |     |     |     |     |     |     |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
5   Hz MSE % 0.038 759 0.000 006 tive, making it difficult to standardise results compared to a commercial
|     |     | PCC   |     | 99.31   |     | 99.90 |     |     |     |     |     |
| --- | --- | ----- | --- | ------- | --- | ----- | --- | --- | --- | --- | --- |
medical-grade acquisition device. Other means to validate the acquisi-
tion were used, such as analysing the noise and the transmission trust-
4. Discussion and conclusion worthiness, but the comparison with signals acquired with different
devices is still a limitation.
This article proposes a novel biosignals acquisition module for non-
Furthermore, inside the device validation regarding its noise, when
specific research purposes. The main novelty of the module is its highly
the signal acquired from the function generator is compared with an al-
configurable characteristic, allowing the acquisition of ECG and sEMG  gorithmically generated wave, the real efficiency can be decreased based
in parallel, with up to 8 different channels. on generator noise instead of an acquisition problem, falsely reducing
The research resulted in a portable module that is compatible with  the system’s credibility. In most cases, this was not a problem since the
medical-grade cables and other commonly used 3-pole jack cables for  system accuracy was kept high with different parameters, which was a
better user experience. This is an improvement to commercial devices,
limitation with a very low voltage signal.
where company-specific cables are required (e.g., BITalino) [24], sol-
However, compared with the literature, the system requires improve-
dered cable [26], or built-in electrodes (e.g., KardiaMobile6L) [25].
ment in different aspects. As further works, it is proposed to enhance the
The system also brings a high-resolution ADC with configurable sam-
control interface with more simultaneous graphs and feature extraction
pling frequency while maintaining wireless data transmission. While the
functionalities, increasing the system’s usability. Other wireless commu-
presented system wireless acquires a 24-bit signal of 8 channels, com-
nication protocols will be tested and implemented to validate against
mercial devices focus on resolutions smaller than 16 bits, with fixed
|     |     |     |     |     |     |     |     | classic Bluetooth® | regarding energy consumption and data volume.  |     |     |
| --- | --- | --- | --- | --- | --- | --- | --- | ------------------ | ---------------------------------------------- | --- | --- |
sampling frequencies close to the Nyquist limit. To achieve 24-bit res-
Further on energy consumption, the flexibility will be implemented in
olution, devices use wired data transmission [25]. This is achieved by
packet structure to allow for low-power operation of the ADC, reducing
employing a bit-wise transmission approach; the packet was reduced by
consumption and resolution. It is also intended to apply for the required
83.92% if compared to JSON formatting. In the same topic, the decod-
ethical approval to acquire signals from volunteers and create a diverse
ing algorithm was also optimised to reduce the required time between
dataset to test the proposed feature extraction functionalities and miti-
iterations, being up to 7.28 times faster than using JSON and using a
gate bias during validation.
maximum of 0.19% from the time available between iterations for the
decoding process.
During tests, the module achieved more than 22 hours of acquisition
CRediT authorship contribution statement
and wireless transmission when powered by an 18650 3350 mAh Li-Ion
battery. The input-referred noise from the module’s channels achieved a
maximum value of 6.7 μV . When comparing a sine wave acquired us- Luiz E. Luiz : Writing – original draft, Visualization, Software,
rms
ing the module with a digitally synthesised one, it achieved correlations  Methodology, Investigation, Formal analysis, Data curation, Concep-
tualization .Salviano Soares :Writing – review & editing, Supervision .
higher than 99.99%.
Antonio Valente :Writing – review & editing .João Barroso :Writing –
The system also achieved a maximum total harmonic distortion of
-53 dBc in the worst case an average of -66.8 dBc and a minimum of  review & editing, Supervision .Paulo Leitão :Writing – review & editing,
-77.08 dBc (best case), while the BITalino (r)evolution achieved a min- Supervision, Resources, Funding acquisition .João P. Teixeira :Writing
imum of -59.80 dBc [24], and other research achieved a minimum of  – review & editing, Validation, Supervision, Resources, Project admin-
-45.53 dBc [31]. istration, Funding acquisition, Formal analysis.
164

L.E.Luiz,S.Soares,A.Valenteetal. ComputationalandStructuralBiotechnologyJournal28(2025)156–166
Ethics statement [11] Córdova-Manzo JF, Leija-Salas L, Vera-Hernández A, Gutiérrez-Salgado JM. Graphi-
calinterfacefortimeandfrequencypatternsextractioninsurfaceelectromyography
The signals presented during this work and used for validation were
signalsusingHilbe rt-Hua ngtr ansform.In: 2023globalmedic aleng ineerin g physics
exchanges/Pacifichealthcareengineering(GMEPE/PAHCE).IEEE;2023.p.1–6.
acquired from the main author, Luiz E. Luiz. No other human subject
[12] RayGC,GuhaSK.Relationshipbetweenthesurfacee.m.g.andmuscularforce.Med
was used during the experimentation. This work did not require ethical Biol Eng Comput 1983 9;21:579–86. https://doi.org/10.1007/BF02442383.
clearance. [13] Wu R, Delahunt E, Ditroilo M, Lowery M, Vito GD. Effects of age and sex on
neuromuscular-mechanical determinants of muscle strength. AGE 2016 6;38:57.
Declaration of generative AI and AI-assisted technologies in the
https://doi.org/10.1007/s11357-016-9921-2.
writing process
[14] GutierrezSJ,Cardi elE,He rnan dezPR.A musclefa tigu emon itorba sedont hesurface
electromyographysignalsandfrequencyanalysis.In:2016globalmedicalengineer-
ingphysicsexchanges/panAmericanhealthcareexchanges(GMEPE/PAHCE).IEEE;
During the preparation of this work, the authors used Grammarly 2016. p. 1–6.
to improve language and readability. After using this tool, the authors [15] Belgacem N, Assous S, Bereksi-Reguig F. Bluetooth portable device and Matlab-based
reviewed and edited the content as needed and take full responsibility GUIfo rECGsign alac quisit ionandanali sys.In:Int ernati onalw or kshoponsystems,
signalprocessingandtheirapplications,WOSSPA.IEEE;2011.p.87–90.
for the content of the publication.
[16] MondeloV,LadoMJ,MéndezAJ.ECGDT:agraphicalsoftwaretoolforECGdiagno-
sis.MultimedToolsAppl202310;83:42799–815.https://doi.org/10.1007/s11042-
Declaration of competing interest 023-17101-2.
[17] MengarelliA,CardarelliS,VerdiniF,BurattiniL,FiorettiS,NardoFD.AMATLAB-
The authors declare that they have no known competing financial
base dgraphicaluserint erfacefo rt heide ntific ationof muscularact ivationsfro ms ur-
faceelectromyographysignals.In:201638thannualinternationalconferenceofthe
interests or personal relationships that could have appeared to influence IEEE engineering in medicine and biology society (EMBC). IEEE; 2016. p. 3646–9.
the work reported in this paper. [18] Babušiak B, Šmondrk M, Janoušek L. Design of ECG system for remote data collec-
tion.Lékařatechnika-ClinTechnol202112;51:53–8.https://doi.org/10.14311/
Acknowledgement CTJ.2021.1.08.
[19] LaiYH,SuK,YouYQ,LiangYX,LanL,LeeMH,etal.DevelopmentofArduinobased
ECGdeviceforSTEMeducationandHRVapplications.In:20228thinternational
The authors are grateful to the Foundation for Science and Tech- conference on control, automation and robotics (ICCAR). IEEE; 2022. p. 329–33.
nology (FCT, Portugal) for financial support through national funds [20] Sestrem L, Kaizer R, Gonçalves J, Leitão P, Teixeira J, Lima J, et al. Data acquisi-
FCT/MCTES (PIDDAC) to CeDRI, UIDB/05757/2020 (DOI: 10.54499/ tion, conditioning and processing system for a wearable-based biostimulation. In:
UIDB/05757/2020) and UIDP/05757/2020 (DOI: 10.54499/UIDB/ Proceedingsofthe15thinternationaljointconferenceonbiomedicalengineering
05757/2020), and SusTEC, LA/P/0007/2020 (DOI: 10.54499/LA/P/
sy stemsandtechnologies.SCITEPRESS-ScienceandTechnologyPublications;2022.
p.223–30.
0007/2020).
[21] KaizerR,SestremL,FrancoT,GonçalvesJ,TeixeiraJ,LimaJ,etal.Dataacquisition
Additionally, this work is financed by the NextGeneration EU fund- system for a wearable-based fall prevention. In: Proceedings of the 16th international
ing through Portugal’s Recovery and Resilience Plan – Mobilizing Agen- joint conference on biomedical engineering systems and technologies. SCITEPRESS
das for Business Innovation, within the project Agenda Drivolution -scienceandtechnologypublications;2023.p.701–10.
(Project 23, 02/C05-i01.02/2022.PC644913740-00000022). [22] LuizLE ,da Silv aW J,Soa resS,Leitão P,TeixeiraJ P.P ortablesyst emanduse rin-
terfaceforECGandEMGacquisition,conditioning,andparametersextraction.Proc
ComputSci2025;256:1216–23.https://doi.org/10.1016/j.procs.2025.02.231.
References [23] Ahamed MA, Ahad MAU, Sohag MHA, Ahmad M. Development of low cost wire-
lessbiosignalacquisitionsystemforECGEMGandEOG.In:20152ndinternational
[1] Kumari P, Mathew L, Syal P. Increasing trend of wearables and multimodal inter- conference on electrical information and communication technologies (EICT). IEEE;
face for human activity monitoring: a review. Biosens Bioelectron 2017;90:298–307. 2015. p. 195–9.
https://doi.org/10.1016/j.bios.2016.12.001. [24] daSilvaHP,GuerreiroJ,LourençoA,FredA,MartinsR.BITalino:anovelhardware
[2] Al-Ayyad M, Owida HA, De Fazio R, Al-Naami B, Visconti P. Electromyography mon- framework for physiological computing. In: Proceedings of the international confer-
itoring systems in rehabilitation: a review of clinical applications, wearable devices ence on physiological computing systems -volume 1: PhyCS. INSTICC. SciTePress;
and signal acquisition methodologies. Electronics 2023;12(7). https://doi.org/10. 2014. p. 246–53.
3390/electronics12071520. [25] Dzikowicz DJ. A scoping review of varying mobile electrocardiographic devices.
[3] AthavaleY,KrishnanS.Biosignalmonitoringusingwearables:observationsand BiolResNurs2024;26(2):303–14.https://doi.org/10.1177/10998004231216923.
opportunities.BiomedSignalProcessControl2017;38:22–33.https://doi.org/10. PMID:38029286.
1016/j.bspc.2017.03.011. [26] Guermandi M, Benatti S, Benini L. A noncontact ECG sensing system with a
[4] Albahri OS, Albahri AS, Mohammed KI, Zaidan AA, Zaidan BB, Hashim M, et al. Sys- micropower, ultrahigh impedance front-end, and BLE connectivity. IEEE Sens J
tematic review of real-time remote health monitoring system in triage and priority- 2024;24(4):4609–17. https://doi.org/10.1109/JSEN.2023.3347100.
based sensor technology: taxonomy, open challenges, motivation and recommenda- [27] Pochet C, Hall DA. 2. In: Harpe P, Baschirotto A, Makinwa KAA, editors. VCO-based
tions. J Med Syst 2018 Mar;42(5):80. https://doi.org/10.1007/s10916-018-0943-4. ADCs for direct digitization of ExG signals. Cham: Springer International Publishing;
[5] Rijnbeek PR, van Herpen G, Bots ML, Man S, Verweij N, Hofman A, et al. Nor- 2023. p. 21–44.
malvaluesoftheelectrocardiogramforages16–90years.JElectrocardiol2014
[28] BatistaD,PlácidodaSilvaH,FredA,MoreiraC,ReisM,FerreiraHA.Benchmark-
11;47:914–21.https://doi.org/10.1016/j.jelectrocard.2014.07.022.
ingoftheBITalinobiomedicaltoolkitagainstanestablishedgoldstandard.Healthc
[6] NgKA,ChanPK.ACMOSanalogfront-endICforportableEEG/ECGmonitoring
TechnolLett2019;6(2):32–6.https://doi.org/10.1049/htl.2018.5037.
applications.IEEETransCircuitsSystI,RegulPap200511;52:2335–47.https://
[29] YangB,LuH.Thekeyissuesofdual-supplylogiclevelconversion.In:201224th
doi.org/10.1109/TCSI.2005.854141.
Chinesecontrolanddecisionconference(CCDC);2012.p.3217–21.
[7] Samuel TR. Unlocking the cardiac vector theory and Einthoven equilateral tri-
[30] ZhengY,ZhaoY,ZhouN,WangH,JiangD.Ashortreviewofsomeanalog-to-digital
anglemodelforanefficientteachingtoolinECGinterpretation.ARCJCardiol
converters resolution enhancement methods. Measurement 2021 8;180:109554.
2023;8:12–22.https://doi.org/10.20431/2455-5991.0801002.
https://doi.org/10.1016/j.measurement.2021.109554.
[8] JangY,NohHW,LeeIB,SongY,ShinS,LeeS.Abasicstudyforpatchtypeam-
bulatory 3-electrode ECG monitoring system for the analysis of acceleration signal [31] Mahajan A, Bidhendi AK, Wang PT, McCrimmon CM, Liu CY, Nenadic Z, et al. A
and the limb leads and augmented unipolar limb leads signal. In: 2010 annual inter- 64-channel ultra-low power bioelectric signal acquisition system for brain-computer
national conference of the IEEE engineering in medicine and biology. IEEE; 2010. interface. In: 2015 IEEE biomedical circuits and systems conference (BioCAS); 2015.
p. 3864–7. p. 1–4.
[9] Wilson FN, Johnston FD, Rosenbaum FF, Barker PS. On Einthoven’s triangle, the the- [32] Hsu YP, Liu Z, Hella MM. A 1.8 μW -65 dB THD ECG acquisition front-end IC
ory of unipolar electrocardiographic leads, and the interpretation of the precordial using a bandpass instrumentation amplifier with class-AB output configuration.
electrocardiogram. Am Heart J 1946 9;32:277–310. https://doi.org/10.1016/0002- IEEE Trans Circuits Syst II, Express Briefs 2018;65(12):1859–63. https://doi.org/
8703(46)90791-0. 10.1109/TCSII.2018.2809470.
[10] daSilvaIAR,dosSantosECBF,CarvalhoEM,DantasDO.Lowcosthardwareand [33] MahmoudSA,BamakhramahA,Al-TunaijiSA.Low-noiselow-passfilterforECG
softwareplatformformultichannelsurfaceelectromyography.In:2018IEEEsym- portabledetectionsystemswithdigitallyprogrammablerange.CircuitsSystSignal
posiumoncomputersandcommunications(ISCC).IEEE;2018.p.01114–9. Process2013Oct;32(5):2029–45.https://doi.org/10.1007/s00034-013-9564-9.
165

|     |     |       |     |     |     |     |     |     |     |     |     |
| --- | --- | ----- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
L.E.Luiz,S.Soares,A.Valenteetal. ComputationalandStructuralBiotechnologyJournal28(2025)156–166
|     |     |     |     |     |     |     |     |     |       |       |     |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | ----- | ----- | --- |
[34] Tereshchenko LG, Josephson ME. Frequency content and characteristics of ven- approach—partIII:otherbiosignals.Sensors20219;21:6064.https://doi.org/10.
| tricularconduction.JElectrocardiol2015;48(6):933–7.https://doi.org/10.1016/j. |     |     |     |     |     |     |     | 3390/s21186064. |     |     |     |
| ----------------------------------------------------------------------------- | --- | --- | --- | --- | --- | --- | --- | --------------- | --- | --- | --- |
jelectrocard.2015.08.034.
|               |          |                     |                   |          |                |           |        |     |     |     |     |
| ------------- | -------- | ------------------- | ----------------- | -------- | -------------- | --------- | ------ | --- | --- | --- | --- |
| [35] Martinek | R,       | Ladrova M, Sidikova | M,                | Jaros R, | Behbehani K,   | Kahankova | R, et  |     |     |     |     |
|               |          |                     |                   |          |                |           |        |     |     |     |     |
| al.           | Advanced | bioelectrical       | signal processing | methods: | past, present, | and       | future |     |     |     |     |
166