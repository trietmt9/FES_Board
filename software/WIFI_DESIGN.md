# Wi-Fi link between the viewer and the nRF7002

**Status (2026-10-06): the GUI is built (section 4); the link is not.** Build order is in
section 8; steps 1-3 need no radio.

## 1. Two modes, one preferred

| | **A. Lab network (preferred)** | **B. PC hotspot (fallback)** |
|---|---|---|
| PC | station on the lab Wi-Fi (or wired) | access point it creates |
| nRF7002 | station on the lab Wi-Fi | station on the PC's hotspot |
| Who listens | **nRF** is the TCP server | **PC** is the TCP server |
| How the PC finds it | mDNS name, or a typed IP | fixed: gateway `10.42.0.1` |
| Needs | lab network lets the two talk | a Wi-Fi card that supports AP mode |
| Code | discovery + TCP client | `nmcli` controller + TCP server |

This is a **lab PC that is also on the lab's Wi-Fi**, so A is the default. B stays
documented because lab networks often break A (section 2), and it is the only mode that
works when the building network will not carry the traffic.

Both modes carry the **same bytes**: the existing 0xAA55 framed stream. `FrameParser`,
the recorder and everything downstream are untouched; only the `ISampleSource` differs.

## 2. Does mode A work on this network? Test first

**Test before writing any code.** Put the nRF7002DK on the lab Wi-Fi (Zephyr's Wi-Fi
shell sample), note the address DHCP gives it, then from this PC:

```bash
ping <nrf-address>
```

| Result | Meaning | Next |
|---|---|---|
| replies | same subnet, no isolation | mode A |
| 100 % loss, both devices online | **client isolation** or different VLANs | ask IT, or mode B |
| nRF cannot join at all | login type (see below) | ask IT |

Things on lab networks that break A:
- **Client isolation** - the AP refuses station-to-station traffic. Both are online and
  still cannot reach each other.
- **Wired and Wi-Fi on different VLANs.** This PC is currently on Ethernet (`enp6s0`).
- **WPA2-Enterprise (802.1X), captive portal, registered-MAC rule.** A shared WPA2
  password is easy for the nRF7002; enterprise login is heavy for first bring-up.
- **mDNS blocked.** Multicast is often filtered on managed Wi-Fi. Hence the manual-IP
  fallback in section 4.
- **DHCP address changes**, so nothing may rely on a fixed address.

## 3. Mode A: roles and connection

```
nRF7002  -- joins lab Wi-Fi --  gets 192.168.x.y by DHCP
         -- advertises  _fesboard._tcp  as  fes-nrf7002.local  port 5000
         -- listens on TCP 5000

PC       -- on the lab network (any interface)
         -- finds the nRF by mDNS (or the user types the IP)
         -- opens ONE TCP connection to it
         -- reads frames exactly as it reads the serial port
```

**The nRF is the server** because only the PC's address is unstable and unknown to the
nRF; the nRF's name is stable. This also fits the spec's Wi-Fi dialog, which is a
*list of devices to pick from*.

- **One client at a time.** A second connection is refused and logged on the nRF, so a
  forgotten viewer window cannot silently steal the stream.
- **Disable Nagle** (`TCP_NODELAY` on the nRF, `LowDelayOption` on the PC): at ~100 kbit/s
  the frames are small and would otherwise wait ~40 ms to be coalesced.
- **TCP, not UDP,** for the first version: ordering and retransmission, so the drop
  counters measure real loss, not radio loss. Revisit for the FES control loop, where a
  late sample is worse than a lost one.
- Stream rate is about 4 ch x 1 kSPS x 3 B = 100 kbit/s - trivial for Wi-Fi, but expect
  **latency jitter of tens of ms** on a shared lab AP. The viewer is not latency-bound;
  closed-loop control must not run over this link unless that is measured.

## 4. The button and the dialog

**Built 2026-10-06** (`qml/WifiDialog.qml`, `qml/Header.qml`, `DeviceManager`). The
dialog gained a USB / Wi-Fi switch and the Wi-Fi view below. The spec's palette has **no
warning colour**, so states are told apart by words and icon, never red or amber. Review
every state with `emg-viewer --preview-wifi <phase>` (phases: manual, searching,
connecting, connected, nodata, error; add `--open-devices` for the dialog). What works
today: address validation, remembering the last usable address, the tab, the labels.
What does not: connecting - Connect says "not built yet". Rescan and the discovered-device
list are left out until discovery exists, rather than shown dead.

Reuse the spec's Wi-Fi button and dialog (section 8 of the spec) - mode A is what they
were drawn for. Only the labels change meaning:

| State | Button label |
|---|---|
| Nothing selected | `Connect device` |
| Searching | `Searching...` |
| Connecting | `Connecting...` |
| Streaming | device name, e.g. `FES-nRF7002` |
| Connected but silent | `FES-nRF7002 · no data`, with the dimmer `wifi-medium` icon |

Dialog list, one section for each transport already in `DeviceManager`:

```
DEVICE CONNECTION
Connect amplifier

  USB
  o ttyACM0            STM32 STLink . 921600 baud

  Wi-Fi  (lab network)                                  [ Rescan ]
  o FES-nRF7002        192.168.4.37 . port 5000         Connect
  o (searching...)

  Not listed?   Address [ 192.168.4.37 ] Port [ 5000 ]  [ Connect ]
```

- **The manual address row is not optional.** mDNS is the first thing a managed network
  drops; without this row the feature simply does not work there.
- Remember the **last used address** in `QSettings` and offer it first.
- `DeviceListModel::Entry` already has `kind = Wifi`, `level` and `needsPasskey`.
  `level` has no meaning here (the PC is not measuring the nRF's radio); leave it unset
  rather than invent a signal strength. `needsPasskey` is unused: the passphrase belongs
  to the lab network and lives in the nRF firmware, not in this dialog.

## 5. State machine (mode A)

```
Off -- select --> Connecting -- TCP up --> Connected -- first valid frame --> Streaming
 ^                    |                        |                                  |
 |                    +-- refused / timeout    +-- 5 s, no frame: "no data"       |
 |                          --> Error                                             |
 +------------------ disconnect / app quit / peer closed -------------------------+
```

- **Connected vs Streaming** is deliberate: "TCP is up but nothing arrives" (firmware not
  sending, wrong port) is the commonest failure and must look different from success.
- Streaming -> back to **Searching/Off** after 3 s with no valid frame, matching how the
  viewer already treats a stalled serial link.
- A connect timeout is ~4 s; report the real reason (`refused` vs `timed out` vs
  `host unreachable`), because they mean different things: nothing listening, wrong
  address / isolation, and wrong subnet.

## 6. PC-side code (mode A)

```
src/io/TcpClientSource : ISampleSource     connect to host:port, feed FrameParser
src/io/MdnsBrowser     (QObject)           finds _fesboard._tcp services
src/model/DeviceManager                    + a Wifi entry per discovered service
```

- **Qt has no mDNS client.** Options, in order of preference: run
  `avahi-browse -rpt _fesboard._tcp` through `QProcess` and parse its machine-readable
  output (Avahi is present on desktop Ubuntu; fine for a lab PC, Linux-only); or send the
  service query over `QUdpSocket` on 224.0.0.251:5353 yourself (portable, ~150 lines,
  more to get wrong). Start with `avahi-browse`, behind an interface so it can be swapped.
- **`QProcess` argument lists only** - never build a shell string from a discovered name.
  A device on the network chooses its own name, so it is untrusted input.
- A discovered entry is shown by name and address only; nothing from the TXT record is
  trusted for anything but display.
- Same **IO thread** as the serial source; nothing new on the GUI thread. The recorder
  already forwards raw bytes before parsing, so recording over Wi-Fi works unchanged.

## 7. nRF7002 side (the contract the PC relies on)

Not part of this work, but fixed here so the two sides agree.

- Join the lab network with credentials from **`prj.conf`/a private `credentials.conf`
  kept out of git** for now. Runtime provisioning (Zephyr `wifi_credentials`, or BLE -
  the nRF5340 has it) is a later step.
- DHCP client on; **`CONFIG_MDNS_RESPONDER`** advertising `_fesboard._tcp` and the host
  name `fes-nrf7002`; TCP server on port **5000**; accept one connection; `TCP_NODELAY`.
- On connect, send an **INFO frame immediately**, then the usual data frames - the viewer
  needs the sample rate before it can scale anything.
- **Security:** the stream is unencrypted physiological data on a shared lab network.
  Acceptable for bench testing with a phantom or yourself; **do not put patient data on
  this** until the link is encrypted (TLS, or a WPA2/3 network you control). This is a
  deliberate gap, not an oversight.

## 8. Build order (each step is testable alone)

1. **`TcpClientSource` + test.** A `QTcpServer` in the test plays the nRF, sends the
   firmware's own encoder output (as `tst_frameparser` does), and the test checks the
   decoded samples, a mid-stream disconnect, and a refused connection. `localhost` only.
2. **Connection state machine** from section 5, tested with the same fake server: silent
   server -> "no data" after 5 s, closed socket -> back to Off.
3. **Dialog**: manual address row first (it needs no discovery), then the status labels.
   Run it against the fake server.
4. **`MdnsBrowser`** (`avahi-browse` wrapper), tested against a stub script that prints
   canned `avahi-browse` output - no network needed.
5. **Hardware:** the `ping` test in section 2, then the nRF firmware, then stream.

Steps 1-4 are done without the nRF7002 and without any radio.

---

## Appendix: mode B, the PC hotspot (fallback)

Use only if section 2's test fails. The PC creates the network and the nRF joins it, so
the roles swap: the **PC is the TCP server** (`TcpServerSource`, listen on port 5000,
one connection) and the nRF connects to the gateway address `10.42.0.1`, which it needs
no configuration to know. Everything in sections 4-5 still applies; the dialog replaces
the Wi-Fi list with a hotspot switch, a generated passphrase and the joined device.

**Creating the AP.** NetworkManager's *shared* mode does the AP, DHCP and addressing in
one step. Run it through `QProcess` with argument lists (the SSID and passphrase are user
input), Linux only:

```bash
nmcli connection add type wifi ifname wlp7s0 con-name fes-hotspot ssid FES-Board-Setup \
      mode ap ipv4.method shared ipv4.addresses 10.42.0.1/24 \
      wifi-sec.key-mgmt wpa-psk wifi-sec.psk '<passphrase>' \
      802-11-wireless.band bg 802-11-wireless.channel 6
nmcli connection up fes-hotspot      # and: nmcli connection down fes-hotspot
```

**What can stop it, in the order to check:**
1. **The card must support AP mode.** `iw` is not installed on this PC, so this is
   unconfirmed: `sudo apt install iw && iw list | grep -A8 "interface modes"` must list
   `AP`. Many cards do not.
2. **`wlp7s0` was `unavailable`** when last checked - probably the radio is off or
   rfkill-blocked (`nmcli radio wifi on`, `rfkill list`).
3. **One radio.** Hosting the hotspot on the card that is also this PC's lab Wi-Fi
   client **drops the lab connection**. It is only workable because the PC is also on
   Ethernet - and a hotspot on the lab's own radio spectrum is something to ask IT
   about before doing it.
4. **Polkit** may prompt for a password when a normal user creates the connection.
5. **Firewall** must allow inbound TCP 5000 on the hotspot interface.

**Do not break the PC's other network.** Record the active connections before starting
and restore them on stop. Stop the hotspot on quit; name the connection `fes-hotspot` so
a stale one left by a crash can be found and removed on the next launch.

**Surface `nmcli`'s stderr** in the dialog ("radio off", "not supported", "insufficient
privileges", "port already in use"). Swallowing it turns "the hotspot does not start"
into an hour of guessing.

## Decisions I need from you

1. **Run the `ping` test** (section 2) and tell me the result: it decides A or B.
2. **TCP or UDP** for the first version? (Recommendation: TCP.)
3. **Is the lab Wi-Fi a shared password or enterprise login?** That decides whether the
   nRF can join it at all.
4. **Is this link only for bench testing, or will it carry patient data?** If the latter,
   encryption has to be designed in now, not later.
