#pragma once

// View state and live metrics for the whole window - the spec's "SignalModel"
// (Biosignal Monitor - Qt Spec.md, section 10).
//
// Everything the UI shows is produced HERE, as finished strings and numbers, and
// QML only lays it out. That is a deliberate split: the interesting logic (which
// filter tags to show, what "2.5 s" does to the tick labels, how the axis moves
// with gain) is then testable with no display, and QML stays free of formatting
// code that would otherwise be duplicated across components.
//
// Threading: GUI thread only. The sample rings it reads are single-producer /
// single-consumer, with the source thread as producer and this as consumer.

#include "core/SignalProfile.h"
#include "core/SpectrumEngine.h"

#include <QElapsedTimer>
#include <QObject>
#include <type_traits>
#include <QSettings>
#include <QStringList>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

#include <array>
#include <cstdint>
#include <vector>

class Acquisition;
QT_BEGIN_NAMESPACE
class QQmlEngine;
class QJSEngine;
QT_END_NAMESPACE

namespace emg { class SampleRing; }

class SignalModel : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // ---- what is being viewed (writable) -----------------------------------
    Q_PROPERTY(int signal READ signal WRITE setSignal NOTIFY signalChanged)
    Q_PROPERTY(int domain READ domain WRITE setDomain NOTIFY domainChanged)
    Q_PROPERTY(double windowSeconds READ windowSeconds WRITE setWindowSeconds NOTIFY windowChanged)
    Q_PROPERTY(double gain READ gain WRITE setGain NOTIFY gainChanged)
    Q_PROPERTY(int averaging READ averaging WRITE setAveraging NOTIFY averagingChanged)
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)

    // Settings the spec names but gives no control for (sections 6.1, 6.2): kept
    // as persisted properties, settable from the command line.
    Q_PROPERTY(int displayMode READ displayMode WRITE setDisplayMode NOTIFY displayModeChanged)
    Q_PROPERTY(bool glow READ glow WRITE setGlow NOTIFY glowChanged)
    Q_PROPERTY(int spectrumStyle READ spectrumStyle WRITE setSpectrumStyle NOTIFY spectrumStyleChanged)

    // Mains. The spec's tags read "Notch 50 Hz"; 60 Hz countries need 60.
    Q_PROPERTY(int mainsHz READ mainsHz WRITE setMainsHz NOTIFY filterChanged)
    Q_PROPERTY(bool notchEnabled READ notchEnabled WRITE setNotchEnabled NOTIFY filterChanged)

    // Header context text. The mockup's "Patient 0042-118 / Session 03" is a
    // placeholder; there is no patient database, so these are plain settings.
    Q_PROPERTY(QString patientLabel READ patientLabel WRITE setPatientLabel NOTIFY headerChanged)
    Q_PROPERTY(QString sessionLabel READ sessionLabel WRITE setSessionLabel NOTIFY headerChanged)

    // ---- presentation, derived ---------------------------------------------
    Q_PROPERTY(QString signalName READ signalName NOTIFY signalChanged)
    Q_PROPERTY(QString title READ title NOTIFY signalChanged)
    Q_PROPERTY(QString kicker READ kicker NOTIFY presentationChanged)
    Q_PROPERTY(QStringList filterTags READ filterTags NOTIFY presentationChanged)
    Q_PROPERTY(QVariantList rows READ rows NOTIFY rowsChanged)
    Q_PROPERTY(QStringList timeTicks READ timeTicks NOTIFY windowChanged)
    Q_PROPERTY(QString footerLeft READ footerLeft NOTIFY presentationChanged)
    Q_PROPERTY(QString footerRight READ footerRight NOTIFY presentationChanged)
    Q_PROPERTY(QStringList railMeta READ railMeta NOTIFY presentationChanged)
    Q_PROPERTY(QStringList railValues READ railValues NOTIFY metricsChanged)

    // ---- device and recording state ----------------------------------------
    Q_PROPERTY(bool online READ online NOTIFY statusChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
    Q_PROPERTY(bool recording READ recording NOTIFY recordingChanged)
    Q_PROPERTY(int recordSeconds READ recordSeconds NOTIFY recordSecondsChanged)
    Q_PROPERTY(bool canRecord READ online NOTIFY statusChanged)

    // ---- metrics (refreshed every 400 ms) ----------------------------------
    Q_PROPERTY(QVariantMap metrics READ metrics NOTIFY metricsChanged)

    // ---- markers ------------------------------------------------------------
    Q_PROPERTY(QVariantList recentMarkers READ recentMarkers NOTIFY markersChanged)
    Q_PROPERTY(int markerCount READ markerCount NOTIFY markersChanged)

public:
    enum Signal { Ecg = 0, Eeg = 1, Emg = 2 };
    Q_ENUM(Signal)
    enum Domain { Time = 0, Frequency = 1 };
    Q_ENUM(Domain)
    enum DisplayMode { Scroll = 0, Sweep = 1 };
    Q_ENUM(DisplayMode)
    enum SpectrumStyle { Area = 0, Bars = 1 };
    Q_ENUM(SpectrumStyle)
    enum Averaging { Low = 0, Medium = 1, High = 2 };
    Q_ENUM(Averaging)

    /// Heart-rate limits shown on the ECG card (spec section 7: "Limits 50-120").
    static constexpr int kHrLimitLow = 50;
    static constexpr int kHrLimitHigh = 120;

    /// EMG: RMS above this is "Contraction" (spec section 7).
    static constexpr double kContractionMv = 0.12;

    /// EMG: below this RMS the median frequency is not reported. It is a property
    /// of the muscle signal; on an idle channel it is the median of the noise
    /// floor. 30 uV is several times the input-referred noise of this front end
    /// (a few uV RMS) and well under a weak contraction. This number is a choice
    /// made here, not taken from the spec, which is silent on it.
    static constexpr double kMedianMinMv = 0.03;

    explicit SignalModel(Acquisition *acq, QObject *parent = nullptr);
    ~SignalModel() override;

    static SignalModel *create(QQmlEngine *, QJSEngine *);
    static void setInstance(SignalModel *m) { s_instance = m; }

    // ---- selection ----------------------------------------------------------
    int signal() const { return int(m_signal); }
    void setSignal(int s);
    int domain() const { return int(m_domain); }
    void setDomain(int d);
    double windowSeconds() const { return m_window; }
    void setWindowSeconds(double s);
    double gain() const { return m_gain; }
    void setGain(double g);
    int averaging() const { return int(m_averaging); }
    void setAveraging(int a);
    bool paused() const;
    void setPaused(bool p);
    int displayMode() const { return int(m_displayMode); }
    void setDisplayMode(int m);
    bool glow() const { return m_glow; }
    void setGlow(bool g);
    int spectrumStyle() const { return int(m_spectrumStyle); }
    void setSpectrumStyle(int s);
    int mainsHz() const { return m_mainsHz; }
    void setMainsHz(int hz);
    bool notchEnabled() const { return m_notchEnabled; }
    void setNotchEnabled(bool on);
    QString patientLabel() const { return m_patient; }
    void setPatientLabel(const QString &s);
    QString sessionLabel() const { return m_session; }
    void setSessionLabel(const QString &s);

    // ---- presentation -------------------------------------------------------
    QString signalName() const;
    QString title() const;
    QString kicker() const;
    QStringList filterTags() const;
    QVariantList rows() const { return m_rows; }
    QStringList timeTicks() const;
    QString footerLeft() const;
    QString footerRight() const;
    QStringList railMeta() const;
    QStringList railValues() const { return m_railValues; }

    bool online() const;
    /// Online and not frozen: the only state in which a trace is moving.
    bool live() const { return online() && !paused(); }
    QString statusText() const;
    bool recording() const;
    int recordSeconds() const { return m_recordSeconds; }
    QVariantMap metrics() const { return m_metrics; }
    QVariantList recentMarkers() const;
    int markerCount() const;

    // ---- actions (QML and shortcuts) ---------------------------------------
    Q_INVOKABLE void selectSignal(int s) { setSignal(s); }
    Q_INVOKABLE void toggleDomain() { setDomain(m_domain == Time ? Frequency : Time); }
    Q_INVOKABLE void togglePause() { setPaused(!paused()); }
    Q_INVOKABLE void addMarker();
    Q_INVOKABLE void toggleRecording();
    /// Step the notch through 50 Hz, 60 Hz, off.
    Q_INVOKABLE void cycleNotch();
    /// Take the biceps RMS seen over the last few seconds as 100 % MVC.
    Q_INVOKABLE void captureMvc();
    Q_INVOKABLE void clearMvc();

    // ---- for the render items (GUI thread) ----------------------------------
    struct MarkerInfo {
        int number;            ///< "M3" - numbered per signal, so each view reads M1, M2, ...
        Signal signal;
        qint64 sampleIndex;    ///< ring write counter at the moment it was placed
        QString label;
        QString time;          ///< HH:MM:SS
    };

    Acquisition *acquisition() const { return m_acq; }
    emg::SampleRing *displayRing() const;
    double deviceRate() const;
    /// Samples spanned by the current window at the device rate.
    std::size_t windowSamples() const;
    /// True if row @p row has a trace behind it (device online and streaming
    /// at least that many channels).
    bool rowHasData(int row) const;
    int rowCountValue() const;
    const emg::SignalProfile &profile() const;
    double halfRangeUv() const;       ///< vertical half-height at the current gain
    QList<MarkerInfo> markersForView() const;
    const emg::SpectrumEngine &spectrum(int row) const;
    /// Samples the display ring holds right now (frozen when paused).
    std::uint64_t head() const;

    /// The head as of the last frame tick - the one every row is drawn against.
    ///
    /// Each row reading head() at its own paint time puts the rows against
    /// heads a few milliseconds apart (the producer never stops), so leads that
    /// carry the same beat come out staggered by tens of milliseconds - the one
    /// thing an ECG is read for. One captured value per frame makes them agree.
    std::uint64_t displayHead() const { return m_frameHead; }

    /// Ring index before which samples belong to a previous filter band.
    std::uint64_t validFrom() const { return m_validFrom; }
    /// How many of those are trustworthy for the CURRENT signal type. After a
    /// signal switch the ring still holds the previous band's output; only
    /// samples from the switch onward are shown. See setSignal().
    std::size_t availableSamples() const;

public slots:
    /// Recompute the spectra and metrics now. The timers call these; tests call
    /// them directly so nothing has to wait on wall-clock time.
    void tickSpectra(bool force = false);
    void tickMetrics();

signals:
    void signalChanged();
    void domainChanged();
    void windowChanged();
    void gainChanged();
    void averagingChanged();
    void pausedChanged();
    void displayModeChanged();
    void glowChanged();
    void spectrumStyleChanged();
    void filterChanged();
    void headerChanged();
    void presentationChanged();
    void rowsChanged();
    void statusChanged();
    void recordingChanged();
    void recordSecondsChanged();
    void metricsChanged();
    void markersChanged();
    /// Fresh spectra are available - the spectrum items repaint on this.
    void spectraUpdated();
    /// About 60 Hz. The trace items repaint on this one shared tick (spec
    /// section 10: "repaint driven by a 60 Hz QTimer").
    void frame();

private:
    void loadSettings();
    void saveSettings() const;
    void applyFilter();
    void configureEngines();
    void refreshFrameHead();
    void rebuildRows();
    void rebuildPresentation();
    int metricRow() const;
    double ecgHeartRate() const;
    static QString formatRate(double hz);

    Acquisition *m_acq;

    Signal m_signal = Ecg;
    Domain m_domain = Time;
    double m_window = 5.0;
    double m_gain = 1.0;
    Averaging m_averaging = Medium;
    DisplayMode m_displayMode = Scroll;
    bool m_glow = true;
    SpectrumStyle m_spectrumStyle = Area;
    int m_mainsHz = 50;
    bool m_notchEnabled = true;
    QString m_patient = QStringLiteral("Patient —");
    QString m_session = QStringLiteral("Session —");

    std::array<emg::SpectrumEngine, 4> m_engines;
    int m_decimation = 1;
    std::vector<float> m_scratch;
    std::vector<float> m_decimated;
    int m_spectrumTick = 0;
    /// Device rate the engines were last configured for.
    double m_engineRate = 0.0;

    /// Ring index before which samples belong to a previous filter band.
    std::uint64_t m_validFrom = 0;

    /// head() as captured at the last frame tick; see displayHead().
    std::uint64_t m_frameHead = 0;

    QVariantList m_rows;
    QVariantMap m_metrics;
    QStringList m_railValues{QStringLiteral("—"), QStringLiteral("—"), QStringLiteral("—")};

    std::vector<MarkerInfo> m_markers;
    std::array<int, 3> m_markerCounter{0, 0, 0};

    // EMG MVC reference, microvolts RMS; 0 = not set. Persisted.
    double m_mvcUv = 0.0;
    std::vector<double> m_recentRms;   // last few seconds of biceps RMS, for captureMvc()

    QTimer m_frameTimer;
    QTimer m_spectrumTimer;
    QTimer m_metricsTimer;
    QTimer m_recordTimer;
    QElapsedTimer m_recordClock;
    int m_recordSeconds = 0;

    static inline SignalModel *s_instance = nullptr;
};

// QML singletons must come from create(), which returns the instance main() owns.
// A default constructor would make the QML engine build its own (see Acquisition).
static_assert(!std::is_default_constructible_v<SignalModel>,
              "a default constructor makes QML ignore SignalModel::create()");
