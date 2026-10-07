#include "SignalModel.h"

#include "Acquisition.h"
#include "core/SampleRing.h"

#include <QDateTime>
#include <QDir>
#include <QQmlEngine>
#include <QStandardPaths>
#include <QUrl>

#include <algorithm>
#include <cmath>

using emg::SignalProfile;
using emg::SignalType;
using emg::SpectrumEngine;

namespace {

// Typographic characters the spec uses. The source is UTF-8, as Qt 6 expects.
const QString kDash = QStringLiteral("—");          // em dash: "no value"
const QString kRange = QStringLiteral("–");         // en dash in ranges
const QString kPlusMinus = QStringLiteral("±");
const QString kMicroVolt = QStringLiteral("µV");
const QString kMiddot = QStringLiteral(" · ");
const QString kDelta = QStringLiteral("Δ");

constexpr double kWindowChoices[] = {2.5, 5.0, 10.0};
constexpr double kGainChoices[] = {0.5, 1.0, 2.0};

// Seconds of history the filtered ring may not be trusted for after the filter
// band changes: the reconfiguration is queued to another thread, so a few
// frames are still written with the old band after the GUI has asked.
constexpr double kBandChangeMarginSeconds = 0.05;

// How many 400 ms metric ticks of biceps RMS captureMvc() looks back over.
constexpr std::size_t kMvcLookbackTicks = 8;

double snap(double v, const double *choices, std::size_t n)
{
    double best = choices[0];
    for (std::size_t i = 1; i < n; ++i) {
        if (std::abs(choices[i] - v) < std::abs(best - v)) {
            best = choices[i];
        }
    }
    return best;
}

QString number(double v)
{
    return QString::number(v, 'g', 3);
}

QString recordFolder()
{
    QSettings st;
    const QString def = QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
                            .filePath(QStringLiteral("FES_Board/Record"));
    return st.value(QStringLiteral("record/folder"), def).toString();
}

} // namespace

SignalModel *SignalModel::create(QQmlEngine *, QJSEngine *)
{
    if (s_instance) {
        QQmlEngine::setObjectOwnership(s_instance, QQmlEngine::CppOwnership);
    }
    return s_instance;
}

SignalModel::SignalModel(Acquisition *acq, QObject *parent) : QObject(parent), m_acq(acq)
{
    loadSettings();
    applyFilter();
    configureEngines();
    rebuildRows();

    // Acquisition state feeds straight into what is displayed.
    connect(m_acq, &Acquisition::runningChanged, this, [this] {
        rebuildRows();
        emit statusChanged();
        emit presentationChanged();
        tickMetrics();
    });
    connect(m_acq, &Acquisition::dataFlowingChanged, this, &SignalModel::statusChanged);
    connect(m_acq, &Acquisition::streamInfoChanged, this, [this] {
        // The firmware sends INFO every second. Rebuilding the engines on each one
        // WIPES their averages, so the EEG and EMG cards - which read an engine
        // that is only refreshed every ~400 ms in the time domain - went blank at
        // every INFO. Only a material change to the analysis setup is worth that:
        // a different decimation factor, or a rate that has really moved. The
        // advertised rate wobbles by a few SPS before it locks, which moves the bin
        // width by a fraction of a percent and is not worth discarding the average.
        const double rate = deviceRate();
        const int m = SpectrumEngine::decimationFactor(rate, profile().nominalRateHz);
        if (m != m_decimation || m_engineRate <= 0.0 ||
            std::abs(rate - m_engineRate) > 0.02 * m_engineRate) {
            configureEngines();
        }
        rebuildRows();
        emit presentationChanged();
    });
    connect(m_acq, &Acquisition::pausedChanged, this, [this] {
        refreshFrameHead();   // a frozen view must show the frozen ring at once
        emit pausedChanged();
        emit statusChanged();
        emit presentationChanged();
    });
    connect(m_acq, &Acquisition::recordingChanged, this, [this] {
        if (m_acq->isRecording()) {
            m_recordClock.start();
            m_recordSeconds = 0;
            m_recordTimer.start();
        } else {
            m_recordTimer.stop();
            m_recordSeconds = 0;
        }
        emit recordingChanged();
        emit recordSecondsChanged();
        emit statusChanged();
    });
    connect(m_acq, &Acquisition::ringsReset, this, [this] {
        // The rings were rebuilt and are empty, so every stored sample index
        // now points at nothing.
        m_markers.clear();
        m_markerCounter = {0, 0, 0};
        m_validFrom = 0;
        m_frameHead = 0;
        for (auto &e : m_engines) {
            e.reset();
        }
        emit markersChanged();
    });

    m_recordTimer.setInterval(1000);
    connect(&m_recordTimer, &QTimer::timeout, this, [this] {
        m_recordSeconds = static_cast<int>(m_recordClock.elapsed() / 1000);
        emit recordSecondsChanged();
    });

    m_frameTimer.setInterval(16);
    m_frameTimer.setTimerType(Qt::PreciseTimer);
    connect(&m_frameTimer, &QTimer::timeout, this, [this] {
        refreshFrameHead();   // ONE head for every row this frame
        emit frame();
    });
    m_frameTimer.start();

    // About 12 Hz for the spectrum (spec section 6.2), 2.5 Hz for the metrics
    // (section 7: every 400 ms).
    m_spectrumTimer.setInterval(83);
    connect(&m_spectrumTimer, &QTimer::timeout, this, [this] { tickSpectra(false); });
    m_spectrumTimer.start();

    m_metricsTimer.setInterval(400);
    connect(&m_metricsTimer, &QTimer::timeout, this, &SignalModel::tickMetrics);
    m_metricsTimer.start();

    tickMetrics();
}

SignalModel::~SignalModel()
{
    saveSettings();
}

// ---------------------------------------------------------------- settings

void SignalModel::loadSettings()
{
    QSettings st;
    m_signal = static_cast<Signal>(std::clamp(st.value(QStringLiteral("view/signal"), 0).toInt(), 0, 2));
    m_window = snap(st.value(QStringLiteral("view/window"), 5.0).toDouble(), kWindowChoices, 3);
    m_gain = snap(st.value(QStringLiteral("view/gain"), 1.0).toDouble(), kGainChoices, 3);
    m_averaging = static_cast<Averaging>(std::clamp(st.value(QStringLiteral("view/averaging"), 1).toInt(), 0, 2));
    m_displayMode = static_cast<DisplayMode>(std::clamp(st.value(QStringLiteral("view/displayMode"), 0).toInt(), 0, 1));
    m_glow = st.value(QStringLiteral("view/glow"), true).toBool();
    m_spectrumStyle = static_cast<SpectrumStyle>(std::clamp(st.value(QStringLiteral("view/spectrumStyle"), 0).toInt(), 0, 1));

    // Same keys as the application this replaced, so the mains frequency the
    // user already chose carries over.
    const int mains = st.value(QStringLiteral("filter/mainsHz"), 50).toInt();
    m_mainsHz = (mains == 60) ? 60 : 50;
    m_notchEnabled = st.value(QStringLiteral("filter/notchEnabled"), true).toBool();

    m_patient = st.value(QStringLiteral("header/patient"), m_patient).toString();
    m_session = st.value(QStringLiteral("header/session"), m_session).toString();
    m_mvcUv = std::max(0.0, st.value(QStringLiteral("emg/mvcUv"), 0.0).toDouble());
}

void SignalModel::saveSettings() const
{
    QSettings st;
    st.setValue(QStringLiteral("view/signal"), int(m_signal));
    st.setValue(QStringLiteral("view/window"), m_window);
    st.setValue(QStringLiteral("view/gain"), m_gain);
    st.setValue(QStringLiteral("view/averaging"), int(m_averaging));
    st.setValue(QStringLiteral("view/displayMode"), int(m_displayMode));
    st.setValue(QStringLiteral("view/glow"), m_glow);
    st.setValue(QStringLiteral("view/spectrumStyle"), int(m_spectrumStyle));
    st.setValue(QStringLiteral("filter/mainsHz"), m_mainsHz);
    st.setValue(QStringLiteral("filter/notchEnabled"), m_notchEnabled);
    st.setValue(QStringLiteral("header/patient"), m_patient);
    st.setValue(QStringLiteral("header/session"), m_session);
    st.setValue(QStringLiteral("emg/mvcUv"), m_mvcUv);
}

// ---------------------------------------------------------------- setters

void SignalModel::setSignal(int s)
{
    const auto next = static_cast<Signal>(std::clamp(s, 0, 2));
    if (next == m_signal) {
        return;
    }
    m_signal = next;

    // Switching signal changes the filter band, and the ring still holds the
    // previous band's output. Showing ECG-filtered history under an EMG title is
    // wrong, so the display only trusts samples from now on...
    m_validFrom = m_acq->liveRing()->written() +
                  static_cast<std::uint64_t>(kBandChangeMarginSeconds * deviceRate());

    // ...and a frozen view of the old signal has nothing meaningful to show
    // under the new one, so switching resumes live.
    m_acq->setPaused(false);

    m_recentRms.clear();
    applyFilter();
    configureEngines();
    refreshFrameHead();
    rebuildRows();
    tickMetrics();

    saveSettings();
    emit signalChanged();
    emit presentationChanged();
    emit markersChanged();
}

void SignalModel::setDomain(int d)
{
    const auto next = (d == int(Frequency)) ? Frequency : Time;
    if (next == m_domain) {
        return;
    }
    m_domain = next;
    rebuildRows();
    emit domainChanged();
    emit presentationChanged();
    if (m_domain == Frequency) {
        tickSpectra(true);   // do not show an empty plot for the first 83 ms
    }
}

void SignalModel::setWindowSeconds(double s)
{
    const double next = snap(s, kWindowChoices, 3);
    if (next == m_window) {
        return;
    }
    m_window = next;
    saveSettings();
    emit windowChanged();
    emit presentationChanged();
}

void SignalModel::setGain(double g)
{
    const double next = snap(g, kGainChoices, 3);
    if (next == m_gain) {
        return;
    }
    m_gain = next;
    rebuildRows();
    saveSettings();
    emit gainChanged();
}

void SignalModel::setAveraging(int a)
{
    const auto next = static_cast<Averaging>(std::clamp(a, 0, 2));
    if (next == m_averaging) {
        return;
    }
    m_averaging = next;
    saveSettings();
    emit averagingChanged();
}

bool SignalModel::paused() const
{
    return m_acq->isPaused();
}

void SignalModel::setPaused(bool p)
{
    m_acq->setPaused(p);
}

void SignalModel::setDisplayMode(int m)
{
    const auto next = (m == int(Sweep)) ? Sweep : Scroll;
    if (next == m_displayMode) {
        return;
    }
    m_displayMode = next;
    saveSettings();
    emit displayModeChanged();
    emit presentationChanged();
}

void SignalModel::setGlow(bool g)
{
    if (g == m_glow) {
        return;
    }
    m_glow = g;
    saveSettings();
    emit glowChanged();
}

void SignalModel::setSpectrumStyle(int s)
{
    const auto next = (s == int(Bars)) ? Bars : Area;
    if (next == m_spectrumStyle) {
        return;
    }
    m_spectrumStyle = next;
    saveSettings();
    emit spectrumStyleChanged();
}

void SignalModel::setMainsHz(int hz)
{
    const int next = (hz == 60) ? 60 : 50;
    if (next == m_mainsHz) {
        return;
    }
    m_mainsHz = next;
    applyFilter();
    saveSettings();
    emit filterChanged();
    emit presentationChanged();
}

void SignalModel::setNotchEnabled(bool on)
{
    if (on == m_notchEnabled) {
        return;
    }
    m_notchEnabled = on;
    applyFilter();
    saveSettings();
    emit filterChanged();
    emit presentationChanged();
}

void SignalModel::cycleNotch()
{
    // 50 Hz -> 60 Hz -> off -> 50 Hz.
    if (m_notchEnabled && m_mainsHz == 50) {
        setMainsHz(60);
    } else if (m_notchEnabled && m_mainsHz == 60) {
        setNotchEnabled(false);
    } else {
        setMainsHz(50);
        setNotchEnabled(true);
    }
}

void SignalModel::setPatientLabel(const QString &s)
{
    if (s == m_patient) {
        return;
    }
    m_patient = s;
    saveSettings();
    emit headerChanged();
}

void SignalModel::setSessionLabel(const QString &s)
{
    if (s == m_session) {
        return;
    }
    m_session = s;
    saveSettings();
    emit headerChanged();
}

// ---------------------------------------------------------------- derived state

const SignalProfile &SignalModel::profile() const
{
    return emg::signalProfile(static_cast<SignalType>(m_signal));
}

int SignalModel::rowCountValue() const
{
    return profile().rowCount;
}

bool SignalModel::online() const
{
    return m_acq->isRunning();
}

emg::SampleRing *SignalModel::displayRing() const
{
    return m_acq->displayRing();
}

std::uint64_t SignalModel::head() const
{
    return m_acq->displayRing()->written();
}

void SignalModel::refreshFrameHead()
{
    m_frameHead = m_acq->displayRing()->written();
}

std::size_t SignalModel::availableSamples() const
{
    const std::uint64_t h = head();
    return h > m_validFrom ? static_cast<std::size_t>(h - m_validFrom) : 0;
}

double SignalModel::deviceRate() const
{
    if (m_acq->haveInfo() && m_acq->sampleRate() > 0) {
        return double(m_acq->sampleRate());
    }
    return profile().nominalRateHz;
}

std::size_t SignalModel::windowSamples() const
{
    return static_cast<std::size_t>(std::llround(m_window * deviceRate()));
}

bool SignalModel::rowHasData(int row) const
{
    return online() && m_acq->haveInfo() && row >= 0 && row < rowCountValue() &&
           row < m_acq->channelCount();
}

double SignalModel::halfRangeUv() const
{
    return profile().rangeUv / m_gain;
}

const SpectrumEngine &SignalModel::spectrum(int row) const
{
    return m_engines[static_cast<std::size_t>(std::clamp(row, 0, 3))];
}

QString SignalModel::formatRate(double hz)
{
    if (hz < 1000.0) {
        return QStringLiteral("%1 Hz").arg(std::llround(hz));
    }
    // 1130.4 -> "1.13 kHz", 2000 -> "2 kHz": two decimals, trailing zeros off.
    QString s = QString::number(hz / 1000.0, 'f', 2);
    while (s.endsWith(QLatin1Char('0'))) {
        s.chop(1);
    }
    if (s.endsWith(QLatin1Char('.'))) {
        s.chop(1);
    }
    return s + QStringLiteral(" kHz");
}

// ---------------------------------------------------------------- presentation

QString SignalModel::signalName() const
{
    return QString::fromLatin1(profile().name);
}

QString SignalModel::title() const
{
    return QString::fromLatin1(profile().title);
}

QString SignalModel::kicker() const
{
    const QString domain =
        (m_domain == Time ? QStringLiteral("Time domain") : QStringLiteral("Frequency domain")).toUpper();
    const double rate = (online() && m_acq->haveInfo()) ? deviceRate() : profile().nominalRateHz;
    return domain + kMiddot + signalName() + kMiddot + formatRate(rate) + QStringLiteral(" sampling");
}

QStringList SignalModel::filterTags() const
{
    const SignalProfile &p = profile();
    QStringList tags;
    tags << QStringLiteral("HP %1 Hz").arg(number(p.highpassHz));
    tags << QStringLiteral("LP %1 Hz").arg(number(p.lowpassHz));
    tags << (m_notchEnabled ? QStringLiteral("Notch %1 Hz").arg(m_mainsHz) : QStringLiteral("Notch off"));
    return tags;
}

QStringList SignalModel::timeTicks() const
{
    // Six ticks across the window: 0, W/5, ... W. That is 0.5 s steps for 2.5 s,
    // 1 s for 5 s and 2 s for 10 s, which is exactly the spec's table.
    QStringList ticks;
    for (int i = 0; i <= 5; ++i) {
        ticks << number(m_window * i / 5.0) + QStringLiteral(" s");
    }
    return ticks;
}

QString SignalModel::footerLeft() const
{
    if (m_domain == Frequency) {
        return QStringLiteral("Hz");
    }
    if (paused()) {
        return QStringLiteral("Frozen");
    }
    return m_displayMode == Sweep ? QStringLiteral("Sweep") : QStringLiteral("Scroll");
}

QString SignalModel::footerRight() const
{
    if (m_domain != Frequency) {
        return QString();
    }
    return QStringLiteral("FFT %1 pt · Hann · %2f %3 Hz")
        .arg(SpectrumEngine::kFftSize)
        .arg(kDelta)
        .arg(m_engines[0].binHz(), 0, 'f', 2);
}

QStringList SignalModel::railMeta() const
{
    QStringList meta;
    for (int i = 0; i < emg::kSignalTypeCount; ++i) {
        const SignalProfile &p = emg::signalProfile(static_cast<SignalType>(i));
        const double rate = (online() && m_acq->haveInfo()) ? deviceRate() : p.nominalRateHz;
        meta << QStringLiteral("%1 %2%3%4")
                    .arg(p.rowCount)
                    .arg(QString::fromLatin1(p.channelNoun), kMiddot, formatRate(rate));
    }
    return meta;
}

QString SignalModel::statusText() const
{
    if (!online()) {
        return QStringLiteral("Device offline");
    }

    QStringList parts;
    if (!m_acq->dataFlowing()) {
        parts << QStringLiteral("No data from device");
    }
    if (paused()) {
        parts << QStringLiteral("Display frozen");
    }
    if (recording()) {
        parts << QStringLiteral("Recording");
    }
    if (parts.isEmpty()) {
        parts << QStringLiteral("Live");
    }
    return parts.join(QStringLiteral(" · "));
}

bool SignalModel::recording() const
{
    return m_acq->isRecording();
}

void SignalModel::rebuildRows()
{
    const SignalProfile &p = profile();
    QVariantList rows;

    const double half = halfRangeUv();
    const bool mv = p.rangeUv >= 1000.0;
    const QString timeScale = kPlusMinus + number(mv ? half / 1000.0 : half) +
                              (mv ? QStringLiteral(" mV") : QStringLiteral(" ") + kMicroVolt);

    for (int i = 0; i < p.rowCount; ++i) {
        QVariantMap r;
        r.insert(QStringLiteral("name"), QString::fromLatin1(p.rows[std::size_t(i)].name));
        r.insert(QStringLiteral("location"), QString::fromLatin1(p.rows[std::size_t(i)].location));
        r.insert(QStringLiteral("hasData"), rowHasData(i));
        r.insert(QStringLiteral("scale"),
                 m_domain == Time ? timeScale : QStringLiteral("dB · rel"));
        rows << r;
    }

    if (rows != m_rows) {
        m_rows = rows;
        emit rowsChanged();
    }
}

// ---------------------------------------------------------------- filter + engines

void SignalModel::applyFilter()
{
    const SignalProfile &p = profile();
    // 250 ms: the window the spec gives for EMG RMS (section 7).
    m_acq->setFilter(p.highpassHz, p.lowpassHz, m_notchEnabled, double(m_mainsHz), 250.0);
}

void SignalModel::configureEngines()
{
    const SignalProfile &p = profile();
    m_decimation = SpectrumEngine::decimationFactor(deviceRate(), p.nominalRateHz);

    SpectrumEngine::Config c;
    c.sampleRateHz = deviceRate() / double(m_decimation);
    c.displayMaxHz = p.displayMaxHz;
    c.peakMinHz = p.peakMinHz;
    for (auto &e : m_engines) {
        e.configure(c);
    }

    m_engineRate = deviceRate();
    m_scratch.assign(SpectrumEngine::kFftSize * std::size_t(m_decimation), 0.0f);
    m_decimated.assign(SpectrumEngine::kFftSize, 0.0f);
}

int SignalModel::metricRow() const
{
    switch (m_signal) {
    case Eeg:
        return 2;   // O1 - the hero card is "DOMINANT FREQUENCY . O1"
    case Emg:
        return 0;   // Biceps
    case Ecg:
        break;
    }
    return -1;
}

void SignalModel::tickSpectra(bool force)
{
    if (!force && (!online() || m_acq->isPaused())) {
        return;
    }

    // In the time domain nothing is drawn from the spectrum, but the EEG and EMG
    // metric cards still need one channel's. Do just that, at a fifth of the rate.
    const bool freq = (m_domain == Frequency);
    if (!freq && !force && (++m_spectrumTick % 5) != 0) {
        return;
    }

    const std::size_t need = SpectrumEngine::kFftSize * std::size_t(m_decimation);
    if (availableSamples() < need) {
        return;   // still filling after a connect or a signal change
    }

    emg::SampleRing *ring = displayRing();
    const auto avg = static_cast<SpectrumEngine::Averaging>(m_averaging);
    bool any = false;

    for (int row = 0; row < rowCountValue(); ++row) {
        if (!freq && row != metricRow()) {
            continue;
        }
        if (row >= m_acq->channelCount() || (!force && !rowHasData(row))) {
            continue;
        }
        if (ring->readLatest(row, m_scratch.data(), need) < need) {
            continue;
        }
        emg::SpectrumEngine::decimateMean(m_scratch.data(), need, m_decimation, m_decimated.data());
        m_engines[std::size_t(row)].update(m_decimated.data(), avg);
        any = true;
    }

    if (any && freq) {
        emit spectraUpdated();
    }
}

// ---------------------------------------------------------------- metrics

double SignalModel::ecgHeartRate() const
{
    // The first lead with a discernible rate. The detector runs on the RAW
    // samples with its own 5-15 Hz band-pass, so this is independent of the
    // display filter.
    const int n = std::min(rowCountValue(), m_acq->channelCount());
    for (int ch = 0; ch < n; ++ch) {
        const double bpm = m_acq->heartRate(ch);
        if (bpm > 0.0) {
            return bpm;
        }
    }
    return 0.0;
}

void SignalModel::tickMetrics()
{
    QVariantMap m;
    const bool on = online();
    QStringList rail{kDash, kDash, kDash};

    switch (m_signal) {
    case Ecg: {
        const double hr = on ? ecgHeartRate() : 0.0;
        const bool have = hr > 0.0;
        m.insert(QStringLiteral("hr"), have ? QString::number(std::llround(hr)) : kDash);
        m.insert(QStringLiteral("rr"), have ? QString::number(std::llround(60000.0 / hr)) : kDash);
        // PR, QRS duration and QTc need waveform delineation (P, Q and T onsets),
        // which nothing here does. The spec says outright that these must come
        // from real measurement - so until they do, they say so.
        m.insert(QStringLiteral("pr"), kDash);
        m.insert(QStringLiteral("qrs"), kDash);
        m.insert(QStringLiteral("qtc"), kDash);
        m.insert(QStringLiteral("limits"),
                 QStringLiteral("Limits %1%2%3").arg(kHrLimitLow).arg(kRange).arg(kHrLimitHigh));

        // The spec's tag reads "Sinus rhythm". That is a rhythm DIAGNOSIS, and
        // nothing here classifies rhythm - printing it would present a guess as a
        // finding. What IS measured is the rate, so the tag reports that against
        // the stated limits.
        QString tag;
        bool accent = false;
        if (!on) {
            tag = QStringLiteral("No signal");
        } else if (!have) {
            tag = QStringLiteral("Measuring…");
        } else if (hr < kHrLimitLow) {
            tag = QStringLiteral("Below limit");
            accent = true;
        } else if (hr > kHrLimitHigh) {
            tag = QStringLiteral("Above limit");
            accent = true;
        } else {
            tag = QStringLiteral("Within limits");
            accent = true;
        }
        m.insert(QStringLiteral("tag"), tag);
        m.insert(QStringLiteral("tagAccent"), accent);
        if (have) {
            rail[0] = QStringLiteral("%1 bpm").arg(std::llround(hr));
        }
        break;
    }

    case Eeg: {
        // Everything on this card is O1 (row 2).
        const SpectrumEngine &o1 = m_engines[2];
        const bool have = on && rowHasData(2) && o1.valid() && o1.peakHz() > 0.0;
        m.insert(QStringLiteral("dominant"), have ? QString::number(o1.peakHz(), 'f', 1) : kDash);

        const double total = (on && o1.valid()) ? o1.bandPower(emg::kEegBands.front().loHz,
                                                                emg::kEegBands.back().hiHz)
                                                : 0.0;
        QVariantList bands;
        int domIdx = -1;
        double domPct = -1.0;
        for (std::size_t i = 0; i < emg::kEegBands.size(); ++i) {
            const auto &b = emg::kEegBands[i];
            const double pct = total > 0.0 ? 100.0 * o1.bandPower(b.loHz, b.hiHz) / total : 0.0;
            if (pct > domPct) {
                domPct = pct;
                domIdx = int(i);
            }
            QVariantMap bm;
            bm.insert(QStringLiteral("symbol"), QString::fromUtf8(b.symbol));
            bm.insert(QStringLiteral("name"), QString::fromLatin1(b.name));
            bm.insert(QStringLiteral("range"),
                      QStringLiteral("%1%2%3 Hz").arg(number(b.loHz), kRange, number(b.hiHz)));
            bm.insert(QStringLiteral("percent"), pct);
            bm.insert(QStringLiteral("percentText"),
                      total > 0.0 ? QStringLiteral("%1%").arg(std::llround(pct)) : kDash);
            bands << bm;
        }
        const bool haveBands = total > 0.0 && domIdx >= 0;
        for (int i = 0; i < bands.size(); ++i) {
            QVariantMap bm = bands[i].toMap();
            bm.insert(QStringLiteral("dominant"), haveBands && i == domIdx);
            bands[i] = bm;
        }
        m.insert(QStringLiteral("bands"), bands);
        m.insert(QStringLiteral("bandTag"),
                 haveBands ? QStringLiteral("%1 band").arg(QString::fromLatin1(emg::kEegBands[std::size_t(domIdx)].name))
                           : QStringLiteral("No signal"));
        m.insert(QStringLiteral("bandTagAccent"), haveBands);
        if (have) {
            rail[1] = QStringLiteral("%1 Hz").arg(o1.peakHz(), 0, 'f', 1);
        }
        break;
    }

    case Emg: {
        const bool have0 = on && rowHasData(0);
        const bool have1 = on && rowHasData(1);
        const double bicepsUv = have0 ? m_acq->envelope(0) : 0.0;
        const double bicepsMv = bicepsUv / 1000.0;
        const double tricepsMv = have1 ? m_acq->envelope(1) / 1000.0 : 0.0;

        m.insert(QStringLiteral("rms"), have0 ? QString::number(bicepsMv, 'f', 2) : kDash);
        m.insert(QStringLiteral("triceps"), have1 ? QString::number(tricepsMv, 'f', 2) : kDash);

        const bool contraction = have0 && bicepsMv > kContractionMv;
        m.insert(QStringLiteral("state"), have0 ? (contraction ? QStringLiteral("Contraction")
                                                               : QStringLiteral("Rest"))
                                                : QStringLiteral("No signal"));
        m.insert(QStringLiteral("stateAccent"), contraction);

        // %MVC needs a reference contraction. The mockup's value was static; a
        // real one has to be measured, so until a reference is captured this
        // shows nothing rather than a number computed against a made-up maximum.
        const bool mvcSet = m_mvcUv > 0.0;
        const double pct = (mvcSet && have0) ? 100.0 * bicepsUv / m_mvcUv : -1.0;
        m.insert(QStringLiteral("mvcSet"), mvcSet);
        m.insert(QStringLiteral("mvcText"), pct >= 0.0 ? QStringLiteral("%1").arg(std::llround(pct)) : kDash);
        m.insert(QStringLiteral("mvcFraction"), pct >= 0.0 ? std::min(1.0, pct / 100.0) : 0.0);

        // Median frequency from 20 Hz upward (spec section 7): below that is
        // motion artifact, and the profile's 20-450 Hz is the SENIAM band.
        const SignalProfile &emgProfile = emg::signalProfile(SignalType::Emg);
        const SpectrumEngine &bic = m_engines[0];
        const double med = (have0 && bic.valid() && bicepsMv >= kMedianMinMv)
                               ? bic.medianHz(emgProfile.peakMinHz, emgProfile.displayMaxHz)
                               : 0.0;
        m.insert(QStringLiteral("median"), med > 0.0 ? QString::number(std::llround(med)) : kDash);

        if (have0) {
            rail[2] = QStringLiteral("%1 mV").arg(bicepsMv, 0, 'f', 2);

            m_recentRms.push_back(bicepsUv);
            while (m_recentRms.size() > kMvcLookbackTicks) {
                m_recentRms.erase(m_recentRms.begin());
            }
        }
        break;
    }
    }

    m_metrics = m;
    m_railValues = rail;
    emit metricsChanged();
}

// ---------------------------------------------------------------- MVC

void SignalModel::captureMvc()
{
    if (m_signal != Emg || m_recentRms.empty()) {
        return;
    }
    m_mvcUv = *std::max_element(m_recentRms.begin(), m_recentRms.end());
    saveSettings();
    tickMetrics();
}

void SignalModel::clearMvc()
{
    m_mvcUv = 0.0;
    saveSettings();
    tickMetrics();
}

// ---------------------------------------------------------------- markers

void SignalModel::addMarker()
{
    if (!online()) {
        return;   // nothing to annotate
    }

    MarkerInfo mk;
    mk.signal = m_signal;
    mk.number = ++m_markerCounter[std::size_t(m_signal)];
    // Stamped against the DISPLAYED head: while frozen, the marker annotates the
    // newest sample on screen, which is what the person who pressed M was
    // looking at, not wherever the live stream has got to.
    //
    // The NEWEST SAMPLE is head - 1. The head itself is the next sample to arrive,
    // one position past the right edge, so a marker stamped there is never drawn -
    // and while frozen, nothing ever arrives to scroll it into view.
    const std::uint64_t h = head();
    mk.sampleIndex = static_cast<qint64>(h > 0 ? h - 1 : 0);
    mk.label = QStringLiteral("Marker");
    mk.time = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"));
    m_markers.push_back(mk);

    emit markersChanged();
}

QList<SignalModel::MarkerInfo> SignalModel::markersForView() const
{
    QList<MarkerInfo> out;
    for (const MarkerInfo &mk : m_markers) {
        if (mk.signal == m_signal) {
            out << mk;
        }
    }
    return out;
}

int SignalModel::markerCount() const
{
    return int(markersForView().size());
}

QVariantList SignalModel::recentMarkers() const
{
    const QList<MarkerInfo> all = markersForView();
    QVariantList out;
    // Latest first, five at most (spec section 7).
    for (int i = int(all.size()) - 1; i >= 0 && out.size() < 5; --i) {
        QVariantMap m;
        m.insert(QStringLiteral("tag"), QStringLiteral("M%1").arg(all[i].number));
        m.insert(QStringLiteral("label"), all[i].label);
        m.insert(QStringLiteral("time"), all[i].time);
        out << m;
    }
    return out;
}

// ---------------------------------------------------------------- recording

void SignalModel::toggleRecording()
{
    if (recording()) {
        m_acq->stopRecording();
        return;
    }
    if (!online()) {
        return;
    }
    // Freeze must not stop recording and recording must not need the display
    // live, so this touches only the acquisition side. Raw frames AND a CSV: the
    // raw file is byte-exact and replayable, the CSV is what analysis tools open.
    m_acq->startRecording(QUrl::fromLocalFile(recordFolder()), true, true);
}
