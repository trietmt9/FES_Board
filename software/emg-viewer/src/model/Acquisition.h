#pragma once

// The acquisition engine, as the rest of the application sees it.
//
// This is the spec's "AcquisitionWorker" (Biosignal Monitor - Qt Spec.md,
// section 10) seen from the GUI thread: it owns the source threads, the sample
// rings and the recorder, and republishes what the UI needs as properties. It
// was StreamController before the rebuild, minus everything that was really
// view state (sweep speed, mm/mV, channel schemes) - that now lives in
// SignalModel, where the spec puts it.
//
// Threading is unchanged and deliberately confined to this class: sources and the
// recorder are moved onto worker threads here, the hot path never crosses a
// queued connection, and every cross-thread edge is a queued signal carrying a
// QByteArray or a small struct. See ARCHITECTURE.md section 6.3.

#include "core/EmgFilter.h"
#include "core/FrameParser.h"
#include "core/SampleRing.h"
#include "io/ISampleSource.h"
#include "model/LinkStats.h"
#include "model/LogModel.h"

#include <QObject>
#include <QUrl>

#include <type_traits>
#include <QtQml/qqmlregistration.h>

QT_BEGIN_NAMESPACE
class QThread;
class QQmlEngine;
class QJSEngine;
QT_END_NAMESPACE

class SerialSource;
class ReplaySource;
class Recorder;

class Acquisition : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(Mode mode READ mode NOTIFY modeChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)

    // Device identity, all from the INFO frame - never hardcoded.
    Q_PROPERTY(int sampleRate READ sampleRate NOTIFY streamInfoChanged)
    Q_PROPERTY(int channelCount READ channelCount NOTIFY streamInfoChanged)
    Q_PROPERTY(int gain READ gain NOTIFY streamInfoChanged)
    Q_PROPERTY(double vrefVolts READ vrefVolts NOTIFY streamInfoChanged)
    Q_PROPERTY(QString chipName READ chipName NOTIFY streamInfoChanged)
    Q_PROPERTY(QString firmwareVersion READ firmwareVersion NOTIFY streamInfoChanged)
    Q_PROPERTY(bool haveInfo READ haveInfo NOTIFY streamInfoChanged)

    // True while DATA frames are actually arriving. A port can be open with the
    // board silent, and "Live" over a flat trace is a lie.
    Q_PROPERTY(bool dataFlowing READ dataFlowing NOTIFY dataFlowingChanged)

    Q_PROPERTY(LinkStats stats READ stats NOTIFY statsChanged)
    Q_PROPERTY(LogModel *log READ log CONSTANT)

    Q_PROPERTY(bool recording READ isRecording NOTIFY recordingChanged)
    Q_PROPERTY(QString recordingPath READ recordingPath NOTIFY recordingChanged)
    Q_PROPERTY(qint64 recordedSamples READ recordedSamples NOTIFY recordingProgress)

    // Display freeze. Acquisition, recording and the link statistics all carry
    // on - the spec is explicit that freeze must not stop recording. Only what
    // is drawn stops.
    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged)

public:
    enum class Mode { Idle, Live, Replay };
    Q_ENUM(Mode)

    /// Deliberately NO default argument. Qt 6.4's QML engine builds a singleton with
    /// its default constructor when one exists and IGNORES a static create() -
    /// so with `parent = nullptr` QML got a second, blank Acquisition (status
    /// "Idle", zero statistics) while the model drove the real one. Requiring the
    /// argument removes the default constructor, which forces QML through create().
    explicit Acquisition(QObject *parent);
    ~Acquisition() override;

    /// QML singleton factory: the instance is created in main() and owned there.
    static Acquisition *create(QQmlEngine *, QJSEngine *);
    static void setInstance(Acquisition *a) { s_instance = a; }

    bool isRunning() const { return m_running; }
    Mode mode() const { return m_mode; }
    QString statusText() const { return m_status; }

    int sampleRate() const { return m_sampleRate; }
    int channelCount() const { return m_channelCount; }
    int gain() const { return m_gain; }
    double vrefVolts() const { return m_vrefUv / 1e6; }
    QString chipName() const;
    QString firmwareVersion() const { return m_fwVersion; }
    bool haveInfo() const { return m_haveInfo; }
    bool dataFlowing() const { return m_dataFlowing; }

    LinkStats stats() const { return m_stats; }
    LogModel *log() { return &m_log; }

    bool isRecording() const { return m_recording; }
    QString recordingPath() const { return m_recordingPath; }
    qint64 recordedSamples() const { return m_recordedSamples; }

    bool isPaused() const { return m_paused; }
    void setPaused(bool paused);

    /// What the waveform and spectrum read: the conditioned stream, frozen when
    /// paused. Always the FILTERED ring - the spec shows its filter state in the
    /// toolbar and has no raw/filtered switch.
    emg::SampleRing *displayRing() { return m_paused ? &m_frozenFiltered : &m_filteredRing; }

    /// The live ring, never frozen. Marker positions are stamped against this.
    emg::SampleRing *liveRing() { return &m_filteredRing; }

    /// Heart rate for @p channel in BPM, 0 when no QRS is discernible.
    double heartRate(int channel) const;

    /// Running RMS of the filtered signal on @p channel, in microvolts, over the
    /// envelope window set by setFilter().
    double envelope(int channel) const;

    /// Apply a filter band. Takes effect on the source's own thread.
    void setFilter(double highpassHz, double lowpassHz, bool notchEnabled,
                   double notchHz, double envelopeMs);

public slots:
    /// Open @p portName at @p baud and start streaming.
    void connectSerial(const QString &portName, int baud);
    void openReplay(const QUrl &fileUrl);
    void disconnectSource();

    /// Replay pacing, 1 = recorded speed. For development and tests; a live
    /// source ignores it.
    void setReplaySpeed(double multiplier);

    void startRecording(const QUrl &folderUrl, bool raw, bool csv);
    void stopRecording();

    void clearLog();

signals:
    void runningChanged();
    void modeChanged();
    void statusTextChanged();
    void streamInfoChanged();
    void statsChanged();
    void dataFlowingChanged();
    void recordingChanged();
    void recordingProgress();
    void pausedChanged();

    /// The rings were rebuilt and are empty: sample indices (markers) from
    /// before this are meaningless.
    void ringsReset();

    /// A source failed to open or died. @p message is for the user.
    void sourceError(const QString &message);

private slots:
    void onInfo(const emg::InfoFrame &info);
    void onStats(const emg::ParserStats &stats, quint64 overflows, quint64 lostSamples);
    void onSourceError(const QString &message);
    void onSourceStopped();

private:
    void teardownSource();
    void attachSource(ISampleSource *src);
    void setStatus(const QString &text);
    void setMode(Mode m);
    void setRunning(bool running);
    void setDataFlowing(bool flowing);
    void reconfigureRing();
    void pushFilterConfig();

    emg::SampleRing m_ring;           // raw microvolts
    emg::SampleRing m_filteredRing;   // conditioned microvolts
    emg::SampleRing m_frozenRaw;      // snapshots taken when the display freezes
    emg::SampleRing m_frozenFiltered;
    bool m_paused = false;

    ISampleSource::EnvelopeBank m_envelopes{};
    ISampleSource::EnvelopeBank m_heartRates{};
    emg::FilterConfig m_filter;
    LogModel m_log;

    QThread *m_ioThread = nullptr;
    QThread *m_recThread = nullptr;
    Recorder *m_recorder = nullptr;
    ISampleSource *m_active = nullptr;
    SerialSource *m_serial = nullptr;
    ReplaySource *m_replay = nullptr;

    bool m_running = false;
    Mode m_mode = Mode::Idle;
    QString m_status = QStringLiteral("Idle");

    // Defaults mirror the firmware's compiled-in configuration; they are
    // replaced the moment an INFO frame arrives.
    int m_sampleRate = 1000;
    int m_channelCount = 4;
    int m_gain = 6;
    double m_vrefUv = 2400000.0;
    quint8 m_chipId = 0;
    QString m_fwVersion;
    bool m_haveInfo = false;

    LinkStats m_stats;
    emg::ParserStats m_lastParser;
    qint64 m_lastStatsMs = 0;
    qint64 m_startedMs = 0;
    bool m_dataFlowing = false;

    bool m_recording = false;
    QString m_recordingPath;
    qint64 m_recordedSamples = 0;

    static inline Acquisition *s_instance = nullptr;
};

// QML must reach the instance main() owns, never one it makes itself. See the
// constructor. Checked here so reintroducing a default constructor fails the build
// instead of producing a silently blank diagnostics panel.
static_assert(!std::is_default_constructible_v<Acquisition>,
              "a default constructor makes QML ignore Acquisition::create()");
