#include "Acquisition.h"

#include "io/Recorder.h"
#include "io/ReplaySource.h"
#include "io/SerialSource.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QQmlEngine>
#include <QThread>

#include <algorithm>
#include <cstdlib>

namespace {
// Seconds of history per channel. The spec asks for 12 s (section 10), which is
// the ceiling on any window the UI can ask for - the longest is 10 s - so the
// window can never outrun the ring.
//
// That relationship has bitten before: when the ring was shorter than the window
// derived from the sweep speed, the window was silently clamped and the trace
// stretched across a grid that said something else. Keep the ring comfortably
// longer than the longest window.
constexpr double kRingSeconds = 12.0;

// A connected board that has sent no DATA frame for this long is reported as
// silent rather than live. The stats arrive about once a second, so anything
// below ~2 s would flicker.
constexpr qint64 kSilentAfterMs = 3000;
} // namespace

Acquisition *Acquisition::create(QQmlEngine *, QJSEngine *)
{
    // Owned by main(); QML must not delete it.
    if (s_instance) {
        QQmlEngine::setObjectOwnership(s_instance, QQmlEngine::CppOwnership);
    }
    return s_instance;
}

Acquisition::Acquisition(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<emg::InfoFrame>("emg::InfoFrame");
    qRegisterMetaType<emg::ParserStats>("emg::ParserStats");
    qRegisterMetaType<emg::FilterConfig>("emg::FilterConfig");

    // The display always reads the conditioned stream, so the filter is always
    // "enabled"; ChannelFilter itself never consults the flag.
    m_filter.enabled = true;
    m_filter.envelopeMs = 250.0;   // spec section 7: EMG RMS over a 250 ms window

    reconfigureRing();

    m_ioThread = new QThread(this);
    m_ioThread->setObjectName(QStringLiteral("emg-io"));
    m_ioThread->start();

    m_recThread = new QThread(this);
    m_recThread->setObjectName(QStringLiteral("emg-rec"));
    m_recThread->start();

    m_recorder = new Recorder;
    m_recorder->moveToThread(m_recThread);

    connect(m_recThread, &QThread::finished, m_recorder, &QObject::deleteLater);
    // A recording failure must not look like a lost device, so it goes to the
    // log (visible in diagnostics) rather than through sourceError(), which the
    // device dialog treats as a failed connection.
    connect(m_recorder, &Recorder::errorOccurred, this, [this](const QString &m) {
        m_log.append(QStringLiteral("<err> recording: %1").arg(m));
    });
    connect(m_recorder, &Recorder::progress, this, [this](qint64, qint64 samples) {
        m_recordedSamples = samples;
        emit recordingProgress();
    });
    connect(m_recorder, &Recorder::recordingStarted, this,
            [this](const QString &raw, const QString &csv) {
                m_recording = true;
                m_recordingPath = raw.isEmpty() ? csv : raw;
                emit recordingChanged();
            });
    connect(m_recorder, &Recorder::recordingStopped, this, [this](qint64, qint64) {
        m_recording = false;
        emit recordingChanged();
    });
}

Acquisition::~Acquisition()
{
    teardownSource();

    if (m_ioThread) {
        m_ioThread->quit();
        m_ioThread->wait();
    }
    if (m_recThread) {
        m_recThread->quit();
        m_recThread->wait();
    }
}

// ------------------------------------------------------------------ getters

QString Acquisition::chipName() const
{
    // Low 5 bits == 0x12 identifies the 8-channel parts; 0x92 is the ADS1298
    // and 0xD2 the ADS1298R. Same check the firmware makes after ads_read_id().
    switch (m_chipId) {
    case 0x92:
        return QStringLiteral("ADS1298");
    case 0xD2:
        return QStringLiteral("ADS1298R");
    case 0x00:
        return QStringLiteral("unknown");
    default:
        return QStringLiteral("0x%1 (unexpected)").arg(m_chipId, 2, 16, QLatin1Char('0'));
    }
}

double Acquisition::heartRate(int channel) const
{
    if (channel < 0 || channel >= static_cast<int>(m_heartRates.size())) {
        return 0.0;
    }
    return static_cast<double>(m_heartRates[static_cast<std::size_t>(channel)].load(
        std::memory_order_relaxed));
}

double Acquisition::envelope(int channel) const
{
    if (channel < 0 || channel >= static_cast<int>(m_envelopes.size())) {
        return 0.0;
    }
    return static_cast<double>(m_envelopes[static_cast<std::size_t>(channel)].load(
        std::memory_order_relaxed));
}

// ------------------------------------------------------------------ setters

void Acquisition::setFilter(double highpassHz, double lowpassHz, bool notchEnabled,
                            double notchHz, double envelopeMs)
{
    emg::FilterConfig cfg = m_filter;
    cfg.highpassHz = highpassHz;
    cfg.lowpassHz = lowpassHz;
    cfg.notchEnabled = notchEnabled;
    cfg.notchHz = notchHz;
    // One notch, because the UI's tag says "Notch 50 Hz" and shows the whole
    // filter state. A second one at 2x would quietly sit in the middle of the
    // EMG band (100/120 Hz) with nothing on screen to say so.
    cfg.notchHarmonics = 1;
    cfg.envelopeMs = envelopeMs;
    if (cfg == m_filter) {
        return;
    }
    m_filter = cfg;
    pushFilterConfig();
}

void Acquisition::pushFilterConfig()
{
    if (!m_active) {
        return;
    }
    // Queued: the filter state belongs to the source's thread.
    QMetaObject::invokeMethod(m_active, "applyFilterConfig", Qt::QueuedConnection,
                              Q_ARG(emg::FilterConfig, m_filter));
}

void Acquisition::setPaused(bool paused)
{
    if (m_paused == paused) {
        return;
    }

    if (paused) {
        // Snapshot before flipping the flag, so the first frozen repaint already
        // has data to draw. Snapshotting - rather than remembering a read
        // position - matters: the producer keeps running while frozen and would
        // overwrite the very samples being looked at within one ring capacity.
        m_ring.snapshotInto(m_frozenRaw);
        m_filteredRing.snapshotInto(m_frozenFiltered);
    }
    m_paused = paused;

    // In replay the source is ours to stop, and letting it run past a frozen
    // view only throws away capture the user wanted to look at. A live source is
    // never paused: freeze must not stop acquisition or recording.
    if (m_replay && m_mode == Mode::Replay) {
        QMetaObject::invokeMethod(m_replay, paused ? "pause" : "resume", Qt::QueuedConnection);
    }

    emit pausedChanged();
}

void Acquisition::setStatus(const QString &text)
{
    if (m_status == text) {
        return;
    }
    m_status = text;
    emit statusTextChanged();
}

void Acquisition::setMode(Mode m)
{
    if (m_mode == m) {
        return;
    }
    m_mode = m;
    emit modeChanged();
}

void Acquisition::setRunning(bool running)
{
    if (m_running == running) {
        return;
    }
    m_running = running;
    emit runningChanged();
}

void Acquisition::setDataFlowing(bool flowing)
{
    if (m_dataFlowing == flowing) {
        return;
    }
    m_dataFlowing = flowing;
    emit dataFlowingChanged();
}

void Acquisition::reconfigureRing()
{
    const auto capacity =
        static_cast<std::size_t>(kRingSeconds * (m_sampleRate > 0 ? m_sampleRate : 1000));
    m_ring.configure(m_channelCount, capacity);
    m_filteredRing.configure(m_channelCount, capacity);
    m_frozenRaw.configure(m_channelCount, capacity);
    m_frozenFiltered.configure(m_channelCount, capacity);

    // Clearing the rings voids every sample index handed out so far.
    emit ringsReset();
}

// ------------------------------------------------------------------ sources

void Acquisition::teardownSource()
{
    if (!m_active) {
        return;
    }

    // stop() must run on the source's own thread.
    QMetaObject::invokeMethod(m_active, "stop", Qt::BlockingQueuedConnection);
    m_active->disconnect(this);

    ISampleSource *doomed = m_active;
    m_active = nullptr;
    m_serial = nullptr;
    m_replay = nullptr;
    doomed->deleteLater();

    setRunning(false);
    setMode(Mode::Idle);
    setDataFlowing(false);
}

void Acquisition::attachSource(ISampleSource *src)
{
    src->setRing(&m_ring);
    src->setFilteredRing(&m_filteredRing);
    src->setEnvelopes(&m_envelopes);
    src->setHeartRates(&m_heartRates);
    src->applyFilterConfig(m_filter);
    src->moveToThread(m_ioThread);
    m_active = src;

    connect(src, &ISampleSource::infoReceived, this, &Acquisition::onInfo);
    connect(src, &ISampleSource::statsUpdated, this, &Acquisition::onStats);
    connect(src, &ISampleSource::textLine, &m_log, &LogModel::append);
    connect(src, &ISampleSource::errorOccurred, this, &Acquisition::onSourceError);
    connect(src, &ISampleSource::stopped, this, &Acquisition::onSourceStopped);
}

void Acquisition::connectSerial(const QString &portName, int baud)
{
    if (m_running) {
        return;
    }
    setPaused(false);
    if (portName.isEmpty()) {
        emit sourceError(tr("No serial port selected."));
        return;
    }

    teardownSource();
    m_stats = LinkStats{};
    m_lastParser = emg::ParserStats{};
    m_lastStatsMs = 0;
    reconfigureRing();

    m_serial = new SerialSource;
    m_serial->setPortName(portName);
    m_serial->setBaudRate(baud);
    m_serial->setForwardRaw(m_recording);
    attachSource(m_serial);

    connect(m_serial, &ISampleSource::started, this, [this, portName, baud] {
        m_startedMs = QDateTime::currentMSecsSinceEpoch();
        setDataFlowing(true);   // grace: opening the port can reset the board
        setRunning(true);
        setMode(Mode::Live);
        setStatus(tr("Streaming from %1 at %2 baud").arg(portName).arg(baud));
    });
    connect(m_serial, &ISampleSource::rawBytes, m_recorder, &Recorder::write);

    m_haveInfo = false;
    emit streamInfoChanged();

    QMetaObject::invokeMethod(m_serial, "start", Qt::QueuedConnection);
}

void Acquisition::openReplay(const QUrl &fileUrl)
{
    const QString path = fileUrl.isLocalFile() ? fileUrl.toLocalFile() : fileUrl.toString();
    if (path.isEmpty()) {
        return;
    }

    setPaused(false);
    teardownSource();
    m_stats = LinkStats{};
    m_lastParser = emg::ParserStats{};
    m_lastStatsMs = 0;
    reconfigureRing();

    m_replay = new ReplaySource;
    m_replay->setFilePath(path);
    m_replay->setForwardRaw(false);   // never re-record a replay by accident
    attachSource(m_replay);

    connect(m_replay, &ISampleSource::started, this, [this, path] {
        m_startedMs = QDateTime::currentMSecsSinceEpoch();
        setDataFlowing(true);
        setRunning(true);
        setMode(Mode::Replay);
        setStatus(tr("Replaying %1").arg(QFileInfo(path).fileName()));
    });

    m_haveInfo = false;
    emit streamInfoChanged();

    QMetaObject::invokeMethod(m_replay, "start", Qt::QueuedConnection);
}

void Acquisition::disconnectSource()
{
    teardownSource();
    setStatus(tr("Idle"));
}

void Acquisition::setReplaySpeed(double multiplier)
{
    if (!m_replay) {
        return;
    }
    QMetaObject::invokeMethod(m_replay, "setSpeed", Qt::QueuedConnection,
                              Q_ARG(double, multiplier));
}

// ---------------------------------------------------------------- recording

void Acquisition::startRecording(const QUrl &folderUrl, bool raw, bool csv)
{
    if (m_recording) {
        return;
    }

    QString dir = folderUrl.isLocalFile() ? folderUrl.toLocalFile() : folderUrl.toString();
    if (dir.isEmpty()) {
        dir = QDir::homePath();
    }
    QDir().mkpath(dir);

    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"));
    const QString base = QDir(dir).filePath(QStringLiteral("emg-%1").arg(stamp));

    m_recordedSamples = 0;
    emit recordingProgress();

    QMetaObject::invokeMethod(m_recorder, "startRecording", Qt::QueuedConnection,
                              Q_ARG(QString, base), Q_ARG(bool, raw), Q_ARG(bool, csv));

    if (m_serial) {
        m_serial->setForwardRaw(true);
    }
}

void Acquisition::stopRecording()
{
    if (m_serial) {
        m_serial->setForwardRaw(false);
    }
    QMetaObject::invokeMethod(m_recorder, "stopRecording", Qt::QueuedConnection);
}

// ------------------------------------------------------------------- events

void Acquisition::onInfo(const emg::InfoFrame &info)
{
    if (!info.isValid()) {
        return;
    }

    // Resizing the rings CLEARS them, so this must not fire on small drift: an
    // advertised rate that wanders by a few SPS would wipe the display over and
    // over, which looks exactly like the board resetting (B-027).
    //
    // The ring is a capacity in samples, so being 10 % out only changes how many
    // seconds it holds - harmless. A channel-count change genuinely does need a
    // rebuild, because the interleave stride changes.
    const int rateDelta = std::abs(info.sampleRateHz - m_sampleRate);
    const bool rateMovedMaterially = m_sampleRate <= 0 || rateDelta * 10 > m_sampleRate;
    const bool rebuild = rateMovedMaterially || info.nChActive != m_channelCount;

    m_sampleRate = info.sampleRateHz;
    m_channelCount = info.nChActive;
    m_gain = info.gain;
    m_vrefUv = info.vrefUv;
    m_chipId = info.chipId;
    m_fwVersion = info.fwVersion;
    m_haveInfo = true;

    if (rebuild) {
        reconfigureRing();
    }

    emit streamInfoChanged();
}

void Acquisition::onStats(const emg::ParserStats &s, quint64 overflows, quint64 lostSamples)
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const double dt = m_lastStatsMs > 0 ? (now - m_lastStatsMs) / 1000.0 : 0.0;
    const int nCh = std::max(1, m_channelCount);

    if (dt > 0.0) {
        m_stats.bytesPerSecond = double(s.bytesIn - m_lastParser.bytesIn) / dt;
        m_stats.framesPerSecond = double(s.dataFrames - m_lastParser.dataFrames) / dt;
        m_stats.measuredSps = double(s.samples - m_lastParser.samples) / dt / nCh;
    }

    m_stats.nominalSps = m_sampleRate;
    m_stats.dataFrames = s.dataFrames;
    m_stats.infoFrames = s.infoFrames;
    m_stats.crcErrors = s.crcErrors;
    m_stats.resyncs = s.resyncs;
    m_stats.droppedFrames = s.droppedFrames;
    m_stats.overflows = overflows;

    // The parser counts samples across ALL channels; the loss tracker counts
    // conversions. LinkStats::lostPercent() divides one by the other, so they
    // must be in the same unit or loss is understated N-fold on an N-channel
    // stream. (It was, before this rebuild - invisible on the single-channel
    // firmware that has been in use.)
    m_stats.totalSamples = s.samples / static_cast<quint64>(nCh);
    m_stats.lostSamples = lostSamples;

    // Flowing = DATA frames advanced since the last report, with a grace period
    // after connecting so the first second does not read as silence.
    const bool advanced = s.dataFrames > m_lastParser.dataFrames;
    const bool inGrace = m_startedMs > 0 && (now - m_startedMs) < kSilentAfterMs;
    setDataFlowing(advanced || inGrace);

    m_lastParser = s;
    m_lastStatsMs = now;

    emit statsChanged();
}

void Acquisition::onSourceError(const QString &message)
{
    m_log.append(QStringLiteral("<err> %1").arg(message));
    setStatus(message);
    emit sourceError(message);
}

void Acquisition::onSourceStopped()
{
    setRunning(false);
    setMode(Mode::Idle);
    setDataFlowing(false);
}

void Acquisition::clearLog()
{
    m_log.clear();
}
