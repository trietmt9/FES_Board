# Reference notes — Luiz et al. 2025, *High-resolution portable bluetooth module for ECG and EMG acquisition*

Comput Struct Biotechnol J 28 (2025) 156–166 · CC BY · ADS129x, 8 ch, 24-bit,
1–4 kHz, ECG + sEMG. Close enough to this project to be worth reading as a
reference implementation.

**Their IC is the ADS1296** (ID readback `10011001`), ours is the ADS1298. Same
register map, six channels instead of eight.

## Register map, theirs vs ours

Decoded from their Table 1 and the prose around it.

| Reg | Paper | Ours | Same? |
|---|---|---|---|
| CONFIG1 `0x01` | `0x85` | `0xC5` | **no** — see DAISY_EN below |
| CONFIG2 `0x02` | `0x00` | `0x00` | yes — independently confirms B-001 |
| CONFIG3 `0x03` | `0xDC` | `0xCC` | differs only in RLD_MEAS |
| LOFF `0x04` | `0x03` | *not written* | **no** |
| CHnSET `0x05–0x0C` | `0x60` (gain **12**) | `0x00` (gain 6) | **no** |
| RLD_SENSP `0x0D` | `0x01` | `0x02` | same *principle* |
| RLD_SENSN `0x0E` | `0x01` | `0x02` | same *principle* |
| CONFIG4 `0x17` | `0x02` | *not written* | **no** |
| WCT1 `0x18` | `0x09` | `0x00` (3-electrode) | n/a |
| WCT2 `0x19` | `0xD8` | `0x00` (3-electrode) | n/a |

### What actually matters

**1. RLD is derived from ONE channel's own two inputs.** `RLD_SENSP = 0x01`,
`RLD_SENSN = 0x01` — channel 1 positive and channel 1 negative, nothing else.
Their wording: *"the reference electrode served as RLD based on the other two ECG
potential electrodes."*

This is independent confirmation of **B-012**. Never feed the RLD loop an input
whose electrode is not attached. They avoid it structurally by deriving from a
single channel's own pair.

**2. They sample at 1 kHz.** CONFIG1 DR = `101` = `fmod_div_512`, exactly our
setting. Their sampling is configurable 1–4 kHz and 1 kHz is what they use for
the published ECG and sEMG traces. Our rate choice needs no defending.

**3. Gain 12, not 6.** `CHnSET = 0x60`. Doubles resolution against a halved
±200 mV headroom. Reasonable with good gelled electrodes; ours at gain 6 is the
more forgiving choice on an unknown front end. Worth revisiting once the signal
is trustworthy.

**4. CONFIG1 bit 6 (DAISY_EN) differs.** Theirs 0 (daisy-chain), ours 1
(multiple-readback). For a single device with no daisy chain, multiple-readback
is the defensible setting — but note that a working published system uses the
other value, so this is not a candidate root cause for anything.

**5. CONFIG3 differs only in RLD_MEAS** (their bit 4 = 1). That routes RLD into a
channel for measurement, which is a GUI feature of theirs. Not needed here.

### Two places the paper contradicts itself

Both matter if the tables are copied verbatim.

- **WCT2**: the table prints `1111 1000` = `0xF8`, the prose says `0xD8`. Only
  `0xD8` matches the prose's own description (WCTB on input 2 negative, WCTC on
  input 1 positive). **Trust `0xD8`.**
- **LOFF**: the prose says *"an AC source signal with a 6 nA current"*, but their
  value `0x03` sets `FLEAD_OFF = 11`, which the ADS1298 datasheet (§9.6.1.5)
  defines as **DC** lead-off detection. AC would be `01`, i.e. `0x01`.

## Lead-off detection — the feature we lack

Their "visual electrode integrity monitoring": per-electrode connected/not,
refreshed every 0.5 s, green/red in the GUI. **This is the single most useful
thing in the paper for us** — it answers "is this electrode actually attached?"
in hardware, without trusting any of our own signal processing.

**We already receive the answer and throw it away.** The 24-bit RDATAC status
word we read every sample is:

```
1100 | LOFF_STATP[8] | LOFF_STATN[8] | GPIO[4]
```

`ads_emg_read_frame_masked()` checks the leading `1100` nibble (B-002) and
discards the remaining 20 bits.

**Their Table 1 is not sufficient to enable it.** It lists
`LOFF_SENSP = LOFF_SENSN = 0x00`, which leaves every channel's detection off —
the comparators are powered but nothing is monitored. The complete recipe is:

| Reg | Value | Why |
|---|---|---|
| `LOFF` `0x04` | `0x03` | DC detect, 6 nA, 95 % threshold |
| `CONFIG4` `0x17` | `0x02` | `PD_LOFF_COMP = 1`, power up the comparators |
| `LOFF_SENSP` `0x0F` | bit per channel | **omitted from their table** |
| `LOFF_SENSN` `0x10` | bit per channel | **omitted from their table** |

Then decode from the status word:

```
LOFF_STATP = ((raw[0] & 0x0F) << 4) | (raw[1] >> 4)
LOFF_STATN = ((raw[1] & 0x0F) << 4) | (raw[2] >> 4)
```

Bit *n* set = `INnP`/`INnN` electrode is **off**.

## Reference figures worth quoting

| Quantity | Their result |
|---|---|
| Input-referred noise | **6.7 µV rms** worst case, inputs shorted |
| Worst-case THD | −53.03 dBc |
| Harmonic correlation | > 99.99 % |
| Battery life | 22.3 h @ 150 mA, 3.3 Ah |
| ECG band | 0.5–150 Hz |
| sEMG band | 10–500 Hz |

Our ADS1298 datasheet figure is 4 µV_PP at gain 6 over 150 Hz — different metric
(pp vs rms, different bandwidth), so not directly comparable, but their 6.7 µV
rms is a realistic **whole-PCB** number to measure ourselves against.
