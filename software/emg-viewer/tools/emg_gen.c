/*
 * emg_gen - synthesise a .emgraw capture.
 *
 * Links the firmware's own encoder, so the output is bit-identical to what the
 * board will emit. That makes it useful for three things: developing the UI
 * with no hardware attached, regression-testing the replay path, and proving
 * the host can keep up with a full-rate stream before the firmware exists.
 *
 * The signal is a crude surface-EMG stand-in - band-limited noise gated by a
 * burst envelope, on a per-channel DC offset - not a physiological model. It is
 * shaped to exercise the viewer: DC that must be removed for the trace to be
 * visible, bursts that must survive min/max decimation, and channels with
 * deliberately different amplitudes.
 *
 * Usage: emg_gen <out.emgraw> [seconds] [--rate HZ] [--drop N] [--corrupt N]
 */
#include "emg_frame.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

static int sample_rate = 1000;   /* --rate */
static double hum_uv = 0.0;      /* --hum: mains interference amplitude */
static double hum_hz = 50.0;     /* --hum-hz */
static double drift_uv = 0.0;    /* --drift: sub-Hz baseline wander amplitude */
static int    ecg_mode = 0;      /* --ecg */
static double ecg_bpm = 60.0;    /* --bpm */
static double ecg_mv = 1.0;      /* --mv: R-wave amplitude, millivolts */
static int    eeg_mode = 0;      /* --eeg */
#define N_CH        4
#define N_SAMP      32
#define STRIDE      N_SAMP

/* Scale used by the firmware: VREF 2.4 V, gain 6 -> ~0.0477 uV/LSB. */
static double uv_to_code(double uv)
{
    return uv / (2400000.0 / 6.0 / 8388608.0);
}

/* Synthetic ECG: P, Q, R, S and T as Gaussians at their usual offsets and
 * relative amplitudes. Not a physiological model - it is a stand-in with the
 * right morphology and, more to the point, an exactly known rate and R-wave
 * amplitude, which is what makes it useful for checking the receiver.
 *
 * Offsets are relative to the R peak; amplitudes are relative to R.
 *   P  -160 ms, +0.15, width 25 ms      T  +300 ms, +0.30, width 50 ms
 *   Q   -20 ms, -0.10, width  8 ms
 *   S   +20 ms, -0.25, width 10 ms
 */
static double ecg_wave(double phase_s)
{
    /* phase_s is time since the last R peak, wrapped into [-RR/2, +RR/2). */
    static const double off[5] = { -0.160, -0.020, 0.0,   0.020, 0.300 };
    static const double amp[5] = {  0.15,  -0.10,  1.0,  -0.25,  0.30  };
    static const double wid[5] = {  0.025,  0.008, 0.010, 0.010, 0.050 };

    double v = 0.0;
    for (int i = 0; i < 5; ++i) {
        const double d = (phase_s - off[i]) / wid[i];
        v += amp[i] * exp(-0.5 * d * d);
    }
    return v;
}

static unsigned rng_state = 12345u;

static double frand(void)
{
    rng_state = rng_state * 1103515245u + 12345u;
    return (double)((rng_state >> 16) & 0x7FFF) / 32768.0 * 2.0 - 1.0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <out.emgraw> [seconds] [--rate HZ] [--hum UV] [--hum-hz HZ]\n"
                "          [--drift UV] [--drop N] [--corrupt N]\n"
                "          [--ecg [--bpm N] [--mv A]] [--eeg]\n",
                argv[0]);
        return 2;
    }

    const char *path = argv[1];
    int seconds = argc > 2 ? atoi(argv[2]) : 30;
    if (seconds <= 0) {
        seconds = 30;
    }

    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "--ecg") == 0) {
            ecg_mode = 1;
        } else if (strcmp(argv[i], "--eeg") == 0) {
            eeg_mode = 1;
        }
    }

    /* Optional fault injection, for exercising the host's error paths. */
    int drop_every = 0;
    int corrupt_every = 0;
    for (int i = 2; i < argc - 1; ++i) {
        if (strcmp(argv[i], "--drop") == 0) {
            drop_every = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "--corrupt") == 0) {
            corrupt_every = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "--rate") == 0) {
            sample_rate = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "--hum") == 0) {
            hum_uv = atof(argv[i + 1]);
        } else if (strcmp(argv[i], "--hum-hz") == 0) {
            hum_hz = atof(argv[i + 1]);
        } else if (strcmp(argv[i], "--drift") == 0) {
            drift_uv = atof(argv[i + 1]);
        } else if (strcmp(argv[i], "--bpm") == 0) {
            ecg_bpm = atof(argv[i + 1]);
        } else if (strcmp(argv[i], "--mv") == 0) {
            ecg_mv = atof(argv[i + 1]);
        }
    }
    if (sample_rate <= 0) {
        sample_rate = 1000;
    }

    FILE *f = fopen(path, "wb");
    if (!f) {
        perror(path);
        return 1;
    }

    uint8_t frame[EMG_FRAME_MAX_SIZE];
    int32_t block[N_CH * STRIDE];

    /* Per-channel character: DC offset (uV), burst amplitude (uV), burst
     * period (s), duty. Channel 4 is deliberately near-silent, so a bad
     * autoscale that amplifies noise to full height is obvious. */
    const double dc[N_CH]    = { 4200.0, -2600.0, 900.0, 150.0 };
    const double amp[N_CH]   = { 220.0,  140.0,   380.0, 8.0   };
    const double period[N_CH]= { 2.0,    3.0,     1.5,   5.0   };
    const double duty[N_CH]  = { 0.35,   0.5,     0.25,  0.4   };
    const double noise[N_CH] = { 6.0,    6.0,     8.0,   4.0   };

    /* Banner, matching what the firmware prints, to exercise the parser's
     * ASCII passthrough alongside binary frames. */
    fprintf(f, "[00:00:00.001,000] <inf> emg_read_example: "
               "========================================\r\n");
    fprintf(f, "[00:00:00.002,000] <inf> emg_read_example: "
               " ADS1298 EMG - CH1-CH4\r\n");
    fprintf(f, "[00:00:00.003,000] <inf> emg_read_example: Chip ID = 0x92\r\n");

    const long total_blocks = (long)seconds * sample_rate / N_SAMP;
    uint32_t seq = 0;
    long sample_index = 0;
    int next_info_block = 0;

    struct emg_info info;
    memset(&info, 0, sizeof(info));
    info.sample_rate_hz = (uint16_t)sample_rate;
    info.vref_uv = 2400000;
    info.gain = 6;
    info.chip_id = 0x92;
    info.n_ch_active = N_CH;
    info.hr_mode = 1;
    memcpy(info.fw_version, "synth", 5);

    for (long b = 0; b < total_blocks; ++b) {
        const uint32_t t_ms = (uint32_t)(sample_index * 1000 / sample_rate);

        /* INFO once per second, as the firmware does. */
        if (b >= next_info_block) {
            info.uptime_s = t_ms / 1000;
            int n = emg_frame_build_info(frame, sizeof(frame), &info);
            fwrite(frame, 1, (size_t)n, f);
            next_info_block = (int)(b + sample_rate / N_SAMP);
        }

        for (int s = 0; s < N_SAMP; ++s) {
            const double t = (double)(sample_index + s) / sample_rate;

            for (int c = 0; c < N_CH; ++c) {
                const double phase = t / period[c];
                const double frac = phase - (double)(long)phase;
                const int bursting = frac < duty[c];

                /* Envelope with soft edges, so bursts do not click. */
                double env = 0.0;
                if (bursting) {
                    const double x = frac / duty[c];
                    env = x < 0.1 ? x / 0.1 : (x > 0.9 ? (1.0 - x) / 0.1 : 1.0);
                }

                if (eeg_mode) {
                    /* A resting, eyes-closed EEG stand-in: a 10.2 Hz alpha rhythm
                     * that is strongest over the occipital channels (3 and 4 =
                     * O1 and O2), with weaker delta, theta and beta underneath.
                     * Amplitudes are peak microvolts. Not physiological - it has
                     * a known dominant frequency and a known band mix, which is
                     * what lets the receiver's band-power card be checked. */
                    static const double alpha_uv[N_CH] = { 6.0, 12.0, 38.0, 30.0 };
                    static const double theta_uv[N_CH] = { 7.0,  5.0,  4.0,  4.0 };
                    static const double beta_uv[N_CH]  = { 5.0,  6.0,  3.0,  3.0 };
                    static const double delta_uv[N_CH] = { 10.0, 8.0,  6.0,  6.0 };
                    const double w = 2.0 * 3.14159265358979 * t;
                    const double uv_eeg =
                        dc[c]
                        + alpha_uv[c] * sin(w * 10.2 + 0.7 * c)
                        + theta_uv[c] * sin(w * 6.0 + 1.3 * c)
                        + beta_uv[c]  * sin(w * 21.0 + 0.4 * c)
                        + delta_uv[c] * sin(w * 2.0 + 2.1 * c)
                        + 2.5 * frand()
                        + hum_uv * sin(w * hum_hz)
                        + drift_uv * (sin(2.0 * 3.14159265358979 * 0.17 * t) +
                                      0.6 * sin(2.0 * 3.14159265358979 * 0.41 * t));
                    block[c * STRIDE + s] = (int32_t)uv_to_code(uv_eeg);
                    continue;
                }

                if (ecg_mode) {
                    /* Same beat on every channel; a real 12-lead would scale
                     * and invert per lead, which is not what we are testing. */
                    const double rr = 60.0 / ecg_bpm;
                    double phase = fmod(t, rr);
                    if (phase > rr / 2.0) {
                        phase -= rr;   /* centre the complex on the R peak */
                    }
                    const double uv_ecg =
                        dc[c]
                        + ecg_mv * 1000.0 * ecg_wave(phase)
                        + noise[c] * frand()
                        + hum_uv * sin(2.0 * 3.14159265358979 * hum_hz * t)
                        + drift_uv * (sin(2.0 * 3.14159265358979 * 0.17 * t) +
                                      0.6 * sin(2.0 * 3.14159265358979 * 0.41 * t));
                    block[c * STRIDE + s] = (int32_t)uv_to_code(uv_ecg);
                    continue;
                }

                const double uv = dc[c] + env * amp[c] * frand() +
                                  noise[c] * frand() +
                                  hum_uv * sin(2.0 * 3.14159265358979 * hum_hz * t) +
                                  /* Two incommensurate sub-Hz tones: electrode
                                   * settling and movement, the artifact that
                                   * actually buries EMG on a DC-coupled input. */
                                  drift_uv * (sin(2.0 * 3.14159265358979 * 0.17 * t) +
                                              0.6 * sin(2.0 * 3.14159265358979 * 0.41 * t));
                block[c * STRIDE + s] = (int32_t)uv_to_code(uv);
            }
        }

        sample_index += N_SAMP;

        /* Simulate loss upstream: skip the write but still burn the sequence
         * number, which is exactly what the host should detect. */
        if (drop_every > 0 && (b % drop_every) == (drop_every - 1)) {
            ++seq;
            continue;
        }

        int n = emg_frame_build_data(frame, sizeof(frame), seq++, t_ms, 0,
                                     block, STRIDE, N_CH, N_SAMP,
                                     EMG_CH_MASK_CONTIGUOUS);
        if (n < 0) {
            fprintf(stderr, "encode failed at block %ld\n", b);
            fclose(f);
            return 1;
        }

        if (corrupt_every > 0 && (b % corrupt_every) == (corrupt_every - 1)) {
            frame[n / 2] ^= 0x01; /* one flipped bit -> CRC must reject it */
        }

        fwrite(frame, 1, (size_t)n, f);

        /* An occasional warning line, interleaved mid-stream. */
        if ((b % 500) == 499) {
            fprintf(f, "[%02u:%02u:%02u.000,000] <wrn> emg_read_example: "
                       "synthetic warning at block %ld\r\n",
                    t_ms / 3600000, (t_ms / 60000) % 60, (t_ms / 1000) % 60, b);
        }
    }

    fclose(f);

    printf("wrote %s: %d s, %d ch @ %d SPS, %ld blocks\n", path, seconds, N_CH,
           sample_rate, total_blocks);
    if (ecg_mode) {
        printf("  ECG: %.0f BPM, R wave %.2f mV\n", ecg_bpm, ecg_mv);
    }
    if (drop_every) {
        printf("  injected a dropped frame every %d blocks\n", drop_every);
    }
    if (corrupt_every) {
        printf("  injected a corrupt frame every %d blocks\n", corrupt_every);
    }
    return 0;
}
