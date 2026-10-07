#include "HeartRate.h"

#include <algorithm>
#include <cmath>

namespace emg {
namespace {

// Pan-Tompkins integration window. 150 ms is about the width of the widest
// normal QRS, so the integrator produces one bump per complex rather than
// separate bumps for the R upstroke and downstroke.
constexpr double kIntegrationMs = 150.0;

// Adaptive threshold weights, as in the original paper.
constexpr double kLevelAlpha = 0.125;   // how fast the level estimates track
constexpr double kThresholdFrac = 0.35; // where the threshold sits between them

// How many intervals the median is taken over. 8 covers a few seconds at
// typical rates - long enough to be stable, short enough to follow a change.
constexpr std::size_t kRrHistory = 8;

// Observation period before any beat is reported, per Pan-Tompkins. Two seconds
// covers at least one beat down to 30 BPM.
constexpr double kLearnMs = 2000.0;

// Filter settling before learning even starts. The 5 Hz high-pass has a ~32 ms
// time constant, so 1 s is about 30 of them - enough for the electrode-offset
// step to have rung out completely.
constexpr double kSettleMs = 1000.0;

// Slowest rate the detector is expected to track. Everything time-related
// below is derived from it rather than picked, because the failure mode is
// silent: a constant that is merely "a long time" by everyday standards can sit
// inside a legitimate interval at bradycardia.
constexpr double kSlowestBpm = 25.0;                         // 2400 ms RR

// No beat for this long means the threshold is probably stuck high.
//
// This MUST stay well clear of a legitimate interval at kSlowestBpm, or genuine
// bradycardia is mistaken for lost signal: the decay pulls the threshold down,
// T and P waves start crossing it, and a 30 BPM trace reads as 60 or 90 before
// the level estimates thrash and the rate drops out entirely. Three times the
// slowest interval leaves room for one missed beat without tripping.
constexpr double kStuckMs = 3.0 * 60000.0 / kSlowestBpm;     // 7200 ms

// T-wave rejection window and relative size.
//
// The T wave follows the QRS by roughly 200-400 ms - OUTSIDE the 200 ms
// refractory - so at slow rates it is counted as a second beat unless something
// discriminates. Classic Pan-Tompkins compares slopes; the integrated signal
// here is already the square of the derivative, so comparing its magnitude
// against the previous QRS is the same test with less machinery.
constexpr double kTWaveMs = 360.0;
constexpr double kAdaptiveRefracFrac = 0.6;

// How far the stuck-detection decay may pull the signal level down. Without a
// floor it keeps falling until the threshold reaches the noise, at which point
// the detector happily "finds" a heart rate in pure noise.
constexpr double kDecayFloorSnr = 2.0;

// Separation between the signal and noise estimates required before a rate is
// reported at all. A real QRS sits orders of magnitude above the between-beat
// excursions; when the two estimates converge it means there is nothing here
// that looks like a QRS, and saying so is better than inventing a number.
constexpr double kReportSnr = 4.0;

// Where the noise level is seeded relative to the observed peak.
constexpr double kNoiseSeedFrac = 0.05;

} // namespace

void HeartRateDetector::configure(double sampleRate)
{
    m_rate = sampleRate > 0.0 ? sampleRate : 1000.0;

    // 5-15 Hz: where QRS energy dominates. Below 5 Hz is baseline wander, P and
    // T waves; above 15 Hz is mostly muscle, mains and artifact.
    //
    // Fourth order, not second. The two Q values are the Butterworth pole pairs
    // for a 4th-order response: 1/(2cos(pi/8)) and 1/(2cos(3pi/8)). Cascading
    // them gives 24 dB/octave instead of 12, which matters specifically because
    // the next stage is a derivative - it amplifies by frequency, so anything
    // the band-pass leaves behind comes back scaled up.
    constexpr double kButterQ1 = 0.54119610;
    constexpr double kButterQ2 = 1.30656296;

    m_hp1 = designHighpass(5.0, m_rate, kButterQ1);
    m_hp2 = designHighpass(5.0, m_rate, kButterQ2);
    m_lp1 = designLowpass(15.0, m_rate, kButterQ1);
    m_lp2 = designLowpass(15.0, m_rate, kButterQ2);

    const auto n = static_cast<std::size_t>(
        std::max(1.0, kIntegrationMs * 1e-3 * m_rate));
    m_window.assign(n, 0.0f);

    m_refractorySamples =
        static_cast<std::size_t>(kRefractoryMs * 1e-3 * m_rate);
    m_stuckSamples = static_cast<std::size_t>(kStuckMs * 1e-3 * m_rate);

    // Halve the excess signal level per second of silence.
    m_stuckDecay = std::pow(0.5, 1.0 / m_rate);

    reset();
}

void HeartRateDetector::reset()
{
    m_hp1.reset();
    m_hp2.reset();
    m_lp1.reset();
    m_lp2.reset();

    std::fill(m_window.begin(), m_window.end(), 0.0f);
    m_windowIndex = 0;
    m_windowSum = 0.0;

    m_prevFiltered = 0.0;
    m_signalLevel = 0.0;
    m_noiseLevel = 0.0;

    m_inExcursion = false;
    m_peak = 0.0;
    m_beatThisExcursion = false;

    m_learning = true;
    m_learnRemaining = static_cast<std::size_t>(kLearnMs * 1e-3 * m_rate);
    m_settleRemaining = static_cast<std::size_t>(kSettleMs * 1e-3 * m_rate);

    m_sinceBeat = 0;
    m_beats = 0;
    m_lastRrMs = 0.0;
    m_rr.clear();
}

bool HeartRateDetector::process(float uv)
{
    if (m_window.empty()) {
        return false;
    }

    // Band-pass -> derivative -> square.
    const double band =
        m_lp2.process(m_lp1.process(m_hp2.process(m_hp1.process(uv))));
    const double deriv = band - m_prevFiltered;
    m_prevFiltered = band;
    const double sq = deriv * deriv;

    // Moving-window integration, O(1) via a running sum.
    m_windowSum -= m_window[m_windowIndex];
    m_window[m_windowIndex] = static_cast<float>(sq);
    m_windowSum += sq;
    m_windowIndex = (m_windowIndex + 1) % m_window.size();
    if (m_windowSum < 0.0) {
        m_windowSum = 0.0; // float cancellation guard
    }
    const double integrated = m_windowSum / double(m_window.size());

    ++m_sinceBeat;

    // --- settling -----------------------------------------------------------
    // Discard entirely: the filters are still ringing down the offset step.
    if (m_settleRemaining > 0) {
        --m_settleRemaining;
        return false;
    }

    // --- learning phase -----------------------------------------------------
    // Watch, do not decide. The largest excursion seen becomes the initial
    // signal level; the threshold derived from it is what makes the first real
    // detection trustworthy.
    if (m_learning) {
        m_peak = std::max(m_peak, integrated);
        if (--m_learnRemaining == 0) {
            m_signalLevel = m_peak;
            m_noiseLevel = m_peak * kNoiseSeedFrac;
            m_peak = 0.0;
            m_learning = false;
            m_sinceBeat = m_refractorySamples + 1; // ready to fire immediately
        }
        return false;
    }

    const double threshold =
        m_noiseLevel + kThresholdFrac * (m_signalLevel - m_noiseLevel);

    bool beat = false;

    // Self-heal. If nothing has been seen for longer than any plausible beat
    // interval, bleed the signal level down until detection resumes rather than
    // staying deaf to a signal that is genuinely there.
    if (m_sinceBeat > m_stuckSamples && m_signalLevel > m_noiseLevel) {
        m_signalLevel = std::max(m_noiseLevel * kDecayFloorSnr,
                                 m_noiseLevel + (m_signalLevel - m_noiseLevel) * m_stuckDecay);
    }

    if (integrated > threshold) {
        // Inside an excursion: track its peak, and fire at most once.
        m_inExcursion = true;
        m_peak = std::max(m_peak, integrated);

        // Rate-adaptive refractory. A fixed 200 ms is correct for tachycardia
        // but far too short at bradycardia, where the T wave lands outside it.
        // Scaling with the observed interval blocks the T wave when beats are
        // slow without ever blocking a genuine fast one: at 30 BPM this works
        // out to 360 ms, while at 180 BPM (RR 333 ms) it stays at 200 ms.
        std::size_t refractory = m_refractorySamples;
        const double medianRr = medianRrMs();
        if (medianRr > 0.0) {
            const double adaptiveMs =
                std::min(kTWaveMs, kAdaptiveRefracFrac * medianRr);
            refractory = std::max(
                refractory,
                static_cast<std::size_t>(adaptiveMs * 1e-3 * m_rate));
        }

        if (!m_beatThisExcursion && m_sinceBeat > refractory) {
            beat = true;
            m_beatThisExcursion = true;

            const double rrMs = double(m_sinceBeat) / m_rate * 1000.0;
            m_sinceBeat = 0;
            ++m_beats;

            // The first interval is measured from the end of the learning
            // phase, not from a real beat, so it is not a genuine RR.
            if (m_beats > 1) {
                m_lastRrMs = rrMs;
                m_rr.push_back(rrMs);
                if (m_rr.size() > kRrHistory) {
                    m_rr.pop_front();
                }
            }
        }
    } else if (m_inExcursion) {
        // Excursion just ended - now fold its peak into the right estimate.
        // A peak that produced a beat teaches us about signal; one that did not
        // teaches us about noise.
        if (m_beatThisExcursion) {
            m_signalLevel = kLevelAlpha * m_peak + (1.0 - kLevelAlpha) * m_signalLevel;
        } else {
            m_noiseLevel = kLevelAlpha * m_peak + (1.0 - kLevelAlpha) * m_noiseLevel;
        }
        m_inExcursion = false;
        m_beatThisExcursion = false;
        m_peak = 0.0;
    }

    return beat;
}

double HeartRateDetector::bpm() const
{
    if (m_rr.size() < 2) {
        return 0.0;
    }

    // Refuse to report unless the QRS estimate stands clear of the noise
    // estimate. On a flat or disconnected channel the two converge, and every
    // noise excursion starts looking like a beat.
    if (m_signalLevel < kReportSnr * m_noiseLevel) {
        return 0.0;
    }

    const double medianMs = medianRrMs();
    if (medianMs <= 0.0) {
        return 0.0;
    }
    return 60000.0 / medianMs;
}

double HeartRateDetector::medianRrMs() const
{
    if (m_rr.size() < 2) {
        return 0.0;
    }

    std::vector<double> sorted(m_rr.begin(), m_rr.end());
    std::sort(sorted.begin(), sorted.end());

    const std::size_t mid = sorted.size() / 2;
    return (sorted.size() % 2 == 0) ? 0.5 * (sorted[mid - 1] + sorted[mid])
                                    : sorted[mid];
}

} // namespace emg
