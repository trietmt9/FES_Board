#pragma once

// Common interface for anything that produces EMG sample blocks.
//
// A source is the *producer* for a SampleRing and writes into it directly, on
// its own thread. Signals here are strictly low-rate metadata - roughly one per
// second, or one per log line - so the hot path never crosses a queued
// connection and never allocates a QVariant per sample.

#include "core/EmgFilter.h"
#include "core/HeartRate.h"
#include "core/FrameParser.h"
#include "core/SampleLoss.h"
#include "core/SampleRing.h"

#include <array>
#include <atomic>
#include <cstdint>

#include <QByteArray>
#include <QObject>
#include <QString>

class ISampleSource : public QObject {
    Q_OBJECT

public:
    explicit ISampleSource(QObject *parent = nullptr) : QObject(parent) {}
    ~ISampleSource() override = default;

    // Set before start(). The source does not take ownership.
    void setRing(emg::SampleRing *ring) { m_ring = ring; }

    /// Second ring receiving the band-passed signal. Both are filled so the
    /// spectrum can stay on the raw stream (where mains hum is diagnostic)
    /// while the waveform shows the conditioned signal.
    void setFilteredRing(emg::SampleRing *ring) { m_filteredRing = ring; }

    /// Per-channel RMS envelope, written here and read by the GUI thread.
    /// Atomic because it crosses threads; a stale read is harmless for a
    /// readout, and the alternative is a signal per sample.
    using EnvelopeBank = std::array<std::atomic<float>, emg::kMaxChannels>;
    void setEnvelopes(EnvelopeBank *e) { m_envelopes = e; }

    /// Per-channel heart rate in BPM, 0 when no QRS is discernible.
    void setHeartRates(EnvelopeBank *b) { m_heartRates = b; }

public slots:
    virtual void start() = 0;
    virtual void stop() = 0;

    /// Runtime filter change. Invoked on the source's own thread so the filter
    /// state is only ever touched there.
    void applyFilterConfig(const emg::FilterConfig &cfg)
    {
        m_filterCfg = cfg;
        reconfigureFilters();
    }

signals:
    void started();
    void stopped();
    void errorOccurred(const QString &message);

    void infoReceived(const emg::InfoFrame &info);
    void textLine(const QString &line);

    // Emitted about once a second, and once more on stop. `lostSamples` is the
    // device-side conversion loss inferred from t_ms - see SampleLossTracker.
    void statsUpdated(const emg::ParserStats &stats, quint64 overflows,
                      quint64 lostSamples);

    /// Which ADS channels the streamed traces are (DataFrame::chMask). Emitted
    /// only when it changes, which in practice is once per connection.
    void channelMapChanged(quint8 mask);

    // Verbatim bytes as received, for the recorder. Only emitted while a
    // recording is active - see StreamController::setForwardRaw().
    void rawBytes(const QByteArray &bytes);

protected:
    /// See core/SampleLoss.h - the loss measurement, and why it matters.
    using SampleLossTracker = emg::SampleLossTracker;

    /// Note a decoded DATA frame: tracks loss and reports a channel-map change.
    void noteDataFrame(const emg::DataFrame &f)
    {
        m_loss.note(f.tMs, f.nSamp, m_filterRate);

        if (!m_haveMask || f.chMask != m_chMask) {
            m_haveMask = true;
            m_chMask = f.chMask;
            emit channelMapChanged(m_chMask);
        }
    }

    /// Rebuild every channel filter for the current config and sample rate.
    void reconfigureFilters()
    {
        for (auto &f : m_filters) {
            f.configure(m_filterCfg, m_filterRate);
        }
        for (auto &h : m_hr) {
            h.configure(m_filterRate);
        }
    }

    /// Run one block through the filters. @p interleaved is sample-major raw
    /// microvolts; @p out receives the filtered result in the same layout.
    void filterBlock(const float *interleaved, float *out, int nCh, int nSamp)
    {
        const int n = nCh < static_cast<int>(emg::kMaxChannels)
                          ? nCh : static_cast<int>(emg::kMaxChannels);

        for (int s = 0; s < nSamp; ++s) {
            for (int c = 0; c < n; ++c) {
                const std::size_t i = static_cast<std::size_t>(s) * nCh + c;
                const auto ch = static_cast<std::size_t>(c);

                out[i] = m_filters[ch].process(interleaved[i]);

                // The QRS detector gets the RAW sample: it runs its own 5-15 Hz
                // band-pass, so it is unaffected by whatever the display filter
                // is set to - including an EMG preset that would gut an ECG.
                m_hr[ch].process(interleaved[i]);
            }
        }

        if (m_envelopes) {
            for (int c = 0; c < n; ++c) {
                (*m_envelopes)[static_cast<std::size_t>(c)].store(
                    static_cast<float>(m_filters[static_cast<std::size_t>(c)].envelope()),
                    std::memory_order_relaxed);
            }
        }
        if (m_heartRates) {
            for (int c = 0; c < n; ++c) {
                (*m_heartRates)[static_cast<std::size_t>(c)].store(
                    static_cast<float>(m_hr[static_cast<std::size_t>(c)].bpm()),
                    std::memory_order_relaxed);
            }
        }
    }

    emg::SampleRing *m_ring = nullptr;
    emg::SampleRing *m_filteredRing = nullptr;
    EnvelopeBank *m_envelopes = nullptr;
    EnvelopeBank *m_heartRates = nullptr;

    SampleLossTracker m_loss;
    std::uint8_t m_chMask = 0;
    bool m_haveMask = false;

    emg::FilterConfig m_filterCfg;
    double m_filterRate = 1000.0;
    std::array<emg::ChannelFilter, emg::kMaxChannels> m_filters;
    std::array<emg::HeartRateDetector, emg::kMaxChannels> m_hr;
};
