# WifiFirmware — nRF7002DK Wi-Fi

Learning/test app for the **nRF7002DK**, separate from `../firmware` (the
ADS1298 reader). The point of this tree is to learn the nRF70 Wi-Fi stack on a
dev kit before any of it goes near the FES board's WT02C40C module.

Current app: **scan for access points and print them.** Nothing else yet.

## Why a scan first

A scan needs no credentials, so almost nothing can be wrong except the parts
you are actually testing: the SPI link to the nRF7002, the firmware patch the
driver loads into it, and the Wi-Fi stack. If SSIDs appear, all three work.
Connecting adds credentials, DHCP and real entropy — three more things that can
fail — so it is the wrong first step.

## Layout

```
WifiFirmware/
  CMakeLists.txt     builds src/main.c, inc/ on the include path
  prj.conf           Kconfig; every block is required, see comments
  boards/
    nrf7002dk_nrf5340_cpuapp.overlay    devicetree changes for the DK
  inc/wifi_app.h     the small API
  src/main.c         scan and print
```

### The devicetree overlay

`boards/nrf7002dk_nrf5340_cpuapp.overlay` is picked up **automatically**:
Zephyr looks for `boards/<board target with / replaced by _>.overlay`, so
building `-b nrf7002dk/nrf5340/cpuapp` loads exactly that file. Rename it and
it is silently ignored with no warning.

Nothing in it is required — the DK's own board files describe the nRF7002
completely and the scan works with the overlay empty. It holds worked examples,
commented out, of the changes you are actually likely to want: slowing the QSPI
bus (first thing to try if the radio will not initialise), disabling coex to
free P0.24/28/29/30 as GPIO, and adding a node for off-board hardware. All
three were verified to apply before being commented out.

Confirm an override landed by reading the merged devicetree, never by assuming:

```bash
grep -n qspi-frequency build/zephyr/zephyr.dts
```

Each line in `zephyr.dts` carries a comment naming the file it came from, so
you can see at a glance whether the board or your overlay won.

## Toolchain — vanilla Zephyr, NOT nRF Connect SDK

This is worth stating because most nRF7002 documentation online assumes NCS.
**You do not need it.** Upstream Zephyr 4.3.99 (the same `~/zephyrproject`
workspace `../firmware` uses) already carries:

- the board — `zephyr/boards/nordic/nrf7002dk`
- the driver — `zephyr/drivers/wifi/nrf_wifi`
- the nRF70 library — `modules/lib/nrf_wifi`

The only extra setup is the firmware blobs, which are **not** in git and
without which the build fails at link:

```bash
cd ~/zephyrproject && west blobs fetch nrf_wifi
west blobs list nrf_wifi --format '{status} {path}'   # all should read A
```

Already done once; re-run it after any `west update`.

## Build and flash

```bash
cd WifiFirmware
source ~/zephyrproject/.venv/bin/activate
export ZEPHYR_BASE=~/zephyrproject/zephyr
export ZEPHYR_SDK_INSTALL_DIR=~/zephyr-sdk-0.17.4

west build -b nrf7002dk/nrf5340/cpuapp .
west flash
```

Board target is `nrf7002dk/nrf5340/cpuapp`. There is also a
`.../cpuapp/nrf7001` variant for the 1-stream part, and a `cpunet` target for
the network core — **you do not need the network core for Wi-Fi.** It is only
needed for Bluetooth, and that matters here for one reason, below.

Console is the DK's on-board J-Link VCOM at 115200 8N1.

Current footprint: **FLASH 50 %, RAM 56 %** of the application core. The Wi-Fi
stack is genuinely that big — budget for it before adding much.

## Expected output

```
=====================================
 nRF7002DK Wi-Fi scan
=====================================
Wi-Fi interface ready
scanning...
#    | SSID                             | band      | chan    | RSSI     | security
1    | MyNetwork                        | 2.4GHz    | ch   6  |  -42 dBm | WPA2-PSK
2    | <hidden>                         | 5GHz      | ch  36  |  -67 dBm | OPEN
scan complete: 2 access point(s)
```

`no Wi-Fi interface after 2 s` means the nRF7002 never came up — check the
blobs above and the board target.

## Two things that will bite you

**IPv4 must stay enabled even though a scan uses no IP.** The driver calls
`net_if_mcast_mon_register()` under `CONFIG_NRF70_STA_MODE`, and that function
only exists when `CONFIG_NET_NATIVE_IPV4` or `_IPV6` is set. Turn both off and
the build fails at *link*, with an undefined reference rather than a config
error — which is not an obvious place to look.

**Randomness is a placeholder.** `CONFIG_TEST_RANDOM_GENERATOR=y` is **not
cryptographically secure**. On the nRF5340 application core the DTS sets
`zephyr,entropy = &rng_hci`, so real entropy arrives over the HCI link from the
*network* core — meaning Bluetooth must be enabled and a controller flashed to
the net core just to get random numbers. The upstream Wi-Fi sample sidesteps it
the same way. It is fine for scanning, which uses no keys. **Fix it before
connecting to WPA2/WPA3 or using TLS**, or the handshake will be built on a
predictable PRNG.

## Next steps, roughly in order

1. **Connect to an AP** — `NET_REQUEST_WIFI_CONNECT` with
   `struct wifi_connect_req_params`. Needs `CONFIG_NET_DHCPV4=y`, and fix the
   entropy problem above first.
2. **Watch link state** — add `NET_EVENT_WIFI_CONNECT_RESULT` and
   `NET_EVENT_WIFI_DISCONNECT_RESULT` to `WIFI_EVENTS`.
3. **Send something** — a UDP socket is the smallest useful test.
4. **Then, and only then**, think about the FES board. The WT02C40C is an
   nRF5340 + nRF7002 in one module, so the driver and most of this config
   carry over — but the board target does not, and the module's SPI wiring and
   pin assignment need their own devicetree.

For reference, `zephyr/samples/net/wifi/shell` gives you an interactive
`wifi` shell command and is the best thing to compare against when something
here does not behave.
