#!/usr/bin/env python3
"""Analyse a .emgraw capture without the app, the filters, or the display.

The measurement that matters here is the TRUE sample rate.

Every DATA frame carries `t_ms`, the MCU's own uptime at the moment the block
closed. That clock comes from the STM32's crystal, which is completely
independent of the ADS1298's conversion clock. So the time between consecutive
frames divided by the samples in a frame gives the real conversion rate,
regardless of what the INFO frame claims. If those two disagree, every trace in
the viewer is time-scaled by the ratio and every derived rate is wrong.

    python3 decode_capture.py capture.emgraw
"""
import struct
import sys

MAGIC = b"\xaa\x55"
HDR = 6
TYPE_DATA, TYPE_INFO = 1, 2


def crc16(data):
    crc = 0xFFFF
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


def s24be(b):
    v = (b[0] << 16) | (b[1] << 8) | b[2]
    return v - 0x1000000 if v & 0x800000 else v


def parse(buf):
    """Yield (type, payload). Mirrors the host FrameParser: validate the header
    before trusting `len`, and surrender exactly one byte on a bad frame."""
    i, bad = 0, 0
    while True:
        j = buf.find(MAGIC, i)
        if j < 0 or j + HDR > len(buf):
            break
        typ, ver, plen = buf[j + 2], buf[j + 3], struct.unpack_from("<H", buf, j + 4)[0]
        if typ not in (TYPE_DATA, TYPE_INFO, 3) or ver != 1 or plen > 1024:
            i = j + 1
            continue
        end = j + HDR + plen + 2
        if end > len(buf):
            break
        if crc16(buf[j + 2:j + HDR + plen]) != struct.unpack_from("<H", buf, end - 2)[0]:
            bad += 1
            i = j + 1
            continue
        yield typ, buf[j + HDR:j + HDR + plen]
        i = end
    if bad:
        print(f"  !! {bad} frames failed CRC")


def find_beats(x, rate):
    """Crude but honest QRS detection: difference, square, threshold, refractory.
    Deliberately not the app's detector - the point is an independent opinion."""
    if len(x) < 10:
        return []
    d = [x[i + 1] - x[i] for i in range(len(x) - 1)]
    e = [v * v for v in d]
    peak = max(e)
    if peak <= 0:
        return []
    thresh = 0.25 * peak
    refractory = int(0.2 * rate)
    beats, last = [], -refractory
    for i, v in enumerate(e):
        if v > thresh and i - last > refractory:
            beats.append(i)
            last = i
    return beats


def main(path):
    buf = open(path, "rb").read()
    adv_rate = vref = gain = None
    chans, mask, seqs, tms = None, 0, [], []

    for typ, p in parse(buf):
        if typ == TYPE_INFO:
            adv_rate, vref = struct.unpack_from("<HI", p, 0)
            gain = p[6]
        elif typ == TYPE_DATA:
            seq, t_ms, n_ch, n_samp, _f, mask = struct.unpack_from("<IIBBBB", p, 0)
            seqs.append(seq)
            tms.append(t_ms)
            if chans is None:
                chans = [[] for _ in range(n_ch)]
            s = 12
            for k in range(n_samp):
                for c in range(n_ch):
                    o = s + (k * n_ch + c) * 3
                    chans[c].append(s24be(p[o:o + 3]))
            nsamp_last = n_samp

    if not chans or len(tms) < 3:
        print("not enough DATA frames decoded")
        return 1

    uv = vref / gain / 8388608.0 if vref and gain else 0.0
    n_ch = len(chans)
    n_samp = len(chans[0]) // len(tms)

    print(f"file        {path} ({len(buf)} B)")
    print(f"INFO        advertises {adv_rate} SPS, VREF {vref} uV, gain {gain}")
    print(f"frames      {len(seqs)}, gaps {seqs[-1] - seqs[0] + 1 - len(seqs)}, "
          f"{n_ch} ch x {n_samp} samp, ch_mask 0x{mask:02X}")

    # ---- the decisive measurement -----------------------------------------
    deltas = [b - a for a, b in zip(tms, tms[1:]) if 0 < b - a < 10000]
    if deltas:
        deltas.sort()
        med = deltas[len(deltas) // 2]
        true_rate = n_samp * 1000.0 / med
        span_s = (tms[-1] - tms[0]) / 1000.0
        overall = (len(tms) - 1) * n_samp / span_s if span_s > 0 else 0
        print()
        print(f"  frame period   {med} ms median  ({min(deltas)}-{max(deltas)} ms)")
        print(f"  TRUE rate      {true_rate:.1f} SPS   (from the MCU uptime clock)")
        print(f"  overall        {overall:.1f} SPS   (over {span_s:.1f} s)")
        if adv_rate:
            ratio = true_rate / adv_rate
            print(f"  ratio          {ratio:.3f} x advertised", end="")
            print("   <-- TIMEBASE IS WRONG" if abs(ratio - 1) > 0.05 else "   OK")
    else:
        true_rate = adv_rate or 1000

    # ---- signal + independent rate ----------------------------------------
    print()
    for c, d in enumerate(chans):
        lo, hi = min(d), max(d)
        span = hi - lo
        beats = find_beats(d, true_rate)
        print(f"CH{c + 1}  p-p {span:>9} codes = {span * uv:8.1f} uV   "
              f"mean {sum(d) / len(d):>10.0f}")
        if len(beats) >= 3:
            iv = [b - a for a, b in zip(beats, beats[1:])]
            iv.sort()
            m = iv[len(iv) // 2]
            spread = (max(iv) - min(iv)) / m if m else 9
            print(f"      {len(beats)} peaks, median interval {m} samples")
            print(f"      -> {60.0 * true_rate / m:6.1f} bpm at the TRUE rate")
            if adv_rate:
                print(f"      -> {60.0 * adv_rate / m:6.1f} bpm at the ADVERTISED rate")
            # A real ECG is regular. Wild spread means these are noise crossings,
            # and any rate computed from them is meaningless.
            print(f"      interval spread {spread * 100:.0f} %"
                  + ("   <-- IRREGULAR, likely noise not beats" if spread > 0.5 else ""))

            # Regular is NOT the same as cardiac. Mains pickup is perfectly
            # periodic, so this crude detector reports it with a confident,
            # tight spread. It has no band-pass, unlike the app's - all it has
            # is a 200 ms refractory, which caps it at 5 Hz / 300 BPM. Anything
            # faster than that therefore comes out PINNED at the ceiling, and a
            # rate pinned at the ceiling means the input is faster than any
            # heart, not that the heart is fast.
            bpm_true = 60.0 * true_rate / m if m else 0.0
            if bpm_true > 250.0:
                print(f"      <-- pinned at the 300 BPM refractory ceiling: the"
                      f" input is faster than any heart rate.")
                print(f"          Almost certainly mains pickup. Check the"
                      f" Frequency tab for a spike at 50/60 Hz.")
        else:
            print("      no repeating peaks found")

        # ---- B-020 signature: the same conversion transmitted more than once.
        # Level-polled DRDY stays asserted until the result is read, so a loop
        # that re-reads before the next conversion pads the stream with copies.
        # CRC, sequence numbers and frame timing all stay perfect through this,
        # so the duplication is only ever visible in the data itself. A run
        # histogram peaked well above 1 means the advertised rate is inflated
        # by that factor and every derived BPM is high by it.
        runs = []
        n = 1
        for a, b in zip(d, d[1:]):
            if a == b:
                n += 1
            else:
                runs.append(n)
                n = 1
        runs.append(n)
        mean_run = sum(runs) / len(runs)
        longest = max(runs)
        print(f"      repeat factor {mean_run:.2f}x  (longest identical run"
              f" {longest})")
        if mean_run > 1.5:
            print(f"      <-- DUPLICATED SAMPLES (B-020). True conversion rate"
                  f" is about {true_rate / mean_run:.0f} SPS, and every BPM"
                  f" reads ~{mean_run:.1f}x high.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1] if len(sys.argv) > 1 else "capture.emgraw"))
