// SignalModel and DeviceManager - everything the window shows, with no window.
//
// Two kinds of test:
//
//  * Presentation: the strings the spec dictates (filter tags, tick labels, row
//    scales, the FFT footer). These are checked against the spec's own examples.
//
//  * Measurement: a synthetic capture with a KNOWN answer - 72 BPM, a 10.2 Hz alpha
//    rhythm, a 120 Hz tone of known RMS - is replayed through the real source,
//    filter and ring, and the number the UI would show is compared with the truth.
//    Nothing is mocked: a replay is the same decode path as a live board.

#include <QCoreApplication>
#include <QDir>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>

#include "core/SpectrumEngine.h"
#include "model/Acquisition.h"
#include "model/DeviceManager.h"
#include "model/SignalModel.h"

extern "C" {
#include "emg_frame.h"
}

#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr int kNSamp = 32;
constexpr double kUvPerCode = 2400000.0 / 6.0 / 8388608.0;

using Signal = std::function<double(int channel, double t)>;   // microvolts

QByteArray capture(int seconds, int rate, int nCh, const Signal &uv)
{
    QByteArray out;
    std::vector<std::uint8_t> frame(EMG_FRAME_MAX_SIZE);
    std::vector<std::int32_t> block(std::size_t(nCh) * kNSamp);

    emg_info info{};
    info.sample_rate_hz = std::uint16_t(rate);
    info.vref_uv = 2400000;
    info.gain = 6;
    info.chip_id = 0x92;
    info.n_ch_active = std::uint8_t(nCh);
    info.hr_mode = 1;
    std::memcpy(info.fw_version, "test", 4);

    // INFO once at the start and then once per second of recorded time, exactly as
    // the firmware does. A capture with a single INFO hides every bug that only
    // shows when INFO keeps arriving - which is how the engine-reset bug got past
    // the first version of this suite.
    int n = emg_frame_build_info(frame.data(), frame.size(), &info);
    out.append(reinterpret_cast<const char *>(frame.data()), n);

    const long blocks = long(seconds) * rate / kNSamp;
    const long blocksPerSecond = std::max<long>(1, rate / kNSamp);
    std::uint32_t seq = 0;
    for (long b = 0; b < blocks; ++b) {
        if (b > 0 && (b % blocksPerSecond) == 0) {
            info.uptime_s = std::uint32_t(b / blocksPerSecond);
            n = emg_frame_build_info(frame.data(), frame.size(), &info);
            out.append(reinterpret_cast<const char *>(frame.data()), n);
        }
        for (int s = 0; s < kNSamp; ++s) {
            const double t = double(b * kNSamp + s) / rate;
            for (int c = 0; c < nCh; ++c) {
                block[std::size_t(c) * kNSamp + s] = std::int32_t(uv(c, t) / kUvPerCode);
            }
        }
        const auto tMs = std::uint32_t(b * kNSamp * 1000 / rate);
        n = emg_frame_build_data(frame.data(), frame.size(), seq++, tMs, 0, block.data(),
                                 kNSamp, std::uint8_t(nCh), kNSamp, EMG_CH_MASK_CONTIGUOUS);
        out.append(reinterpret_cast<const char *>(frame.data()), n);
    }
    return out;
}

// P, Q, R, S, T as Gaussians relative to the R peak (the same shape emg_gen uses).
double ecgWave(double ph)
{
    static const double off[5] = {-0.160, -0.020, 0.0, 0.020, 0.300};
    static const double amp[5] = {0.15, -0.10, 1.0, -0.25, 0.30};
    static const double wid[5] = {0.025, 0.008, 0.010, 0.010, 0.050};
    double v = 0.0;
    for (int i = 0; i < 5; ++i) {
        const double d = (ph - off[i]) / wid[i];
        v += amp[i] * std::exp(-0.5 * d * d);
    }
    return v;
}

Signal ecg(double bpm, double mv = 1.0)
{
    return [=](int c, double t) {
        const double rr = 60.0 / bpm;
        double ph = std::fmod(t, rr);
        if (ph > rr / 2.0) {
            ph -= rr;
        }
        return 3000.0 + 400.0 * c + mv * 1000.0 * ecgWave(ph);   // DC offset, as on the real front end
    };
}

QString write(QTemporaryDir &dir, const QString &name, const QByteArray &data)
{
    const QString path = QDir(dir.path()).filePath(name);
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        return {};
    }
    f.write(data);
    return path;
}

} // namespace

class TstSignalModel : public QObject {
    Q_OBJECT

private:
    Acquisition *acq = nullptr;
    SignalModel *model = nullptr;
    QTemporaryDir dir;

    // Replay `path` fast and wait until the display has a full window of trusted
    // samples to read.
    void startReplay(const QString &path, double speed = 8.0)
    {
        acq->openReplay(QUrl::fromLocalFile(path));
        acq->setReplaySpeed(speed);
        QTRY_VERIFY_WITH_TIMEOUT(acq->isRunning() && acq->haveInfo(), 5000);
    }

    void waitForSamples(std::size_t n)
    {
        QTRY_VERIFY_WITH_TIMEOUT(model->availableSamples() >= n, 10000);
    }

private slots:
    void initTestCase();
    void init();
    void cleanup();

    // ---- presentation, against the spec's own text ------------------------------
    void filterTagsPerSignal();
    void notchTagFollowsTheMainsSetting();
    void kickerCarriesDomainSignalAndRate();
    void titlesAndRailMeta();
    void timeTicksMatchTheSpecTable();
    void rowsAndScaleLabels();
    void footerTextsMatchTheSpec();
    void windowAndGainSnapToTheirChoices();
    void settingsPersist();

    // ---- state -----------------------------------------------------------------------
    void offlineStatusAndNoRows();
    void statusReflectsFreezeAndLive();
    void signalSwitchDropsOldHistoryAndResumesLive();
    void markersAreNumberedPerSignal();
    void markersRefusedOffline();

    // ---- measurement ---------------------------------------------------------------------
    void ecgRateIsMeasuredNotAssumed();
    void slowHeartRateIsFlaggedAgainstTheLimits();
    void eegFindsTheAlphaPeakAndBandMix();
    void metricsSurviveTheDevicesPeriodicInfoFrames();
    void metricsAppearInTheTimeDomainWithoutForcing();
    void emgRmsAndMedianFromAKnownTone();
    void mvcReferenceScalesTheActivationBar();
    void medianFrequencyIsNotReportedForNoise();

    // ---- device manager ---------------------------------------------------------------------
    void replayReportsAsAConnectedDevice();
    void deliberateDisconnectIsNotALostConnection();
    void previewListIsTheSpecsThreeNetworksAndCannotConnect();
    void dialogTitleIsHonestAboutTheTransport();
};

void TstSignalModel::initTestCase()
{
    // A throwaway settings location: this suite must never read or write the
    // user's real viewer settings.
    QCoreApplication::setOrganizationName(QStringLiteral("FES_Board_Test"));
    QCoreApplication::setApplicationName(QStringLiteral("signalmodel-test"));
    QStandardPaths::setTestModeEnabled(true);
    QSettings().clear();

    qRegisterMetaType<emg::InfoFrame>("emg::InfoFrame");
    qRegisterMetaType<emg::ParserStats>("emg::ParserStats");
    qRegisterMetaType<emg::FilterConfig>("emg::FilterConfig");
    QVERIFY(dir.isValid());
}

void TstSignalModel::init()
{
    QSettings().clear();
    acq = new Acquisition(nullptr);
    model = new SignalModel(acq);
}

void TstSignalModel::cleanup()
{
    delete model;
    model = nullptr;
    delete acq;
    acq = nullptr;
}

// =============================================================== presentation

void TstSignalModel::filterTagsPerSignal()
{
    // Spec section 5.2.
    model->setSignal(SignalModel::Ecg);
    QCOMPARE(model->filterTags(), (QStringList{"HP 0.5 Hz", "LP 40 Hz", "Notch 50 Hz"}));
    model->setSignal(SignalModel::Eeg);
    QCOMPARE(model->filterTags(), (QStringList{"HP 0.5 Hz", "LP 45 Hz", "Notch 50 Hz"}));
    model->setSignal(SignalModel::Emg);
    QCOMPARE(model->filterTags(), (QStringList{"HP 20 Hz", "LP 450 Hz", "Notch 50 Hz"}));
}

void TstSignalModel::notchTagFollowsTheMainsSetting()
{
    // The tag shows the filter state truthfully - "never hide the filter state"
    // (spec section 10) - so it follows the setting, including "off".
    model->setSignal(SignalModel::Ecg);
    model->setMainsHz(60);
    QCOMPARE(model->filterTags().at(2), QStringLiteral("Notch 60 Hz"));
    model->setNotchEnabled(false);
    QCOMPARE(model->filterTags().at(2), QStringLiteral("Notch off"));

    // cycleNotch steps 50 -> 60 -> off -> 50.
    model->setNotchEnabled(true);
    model->setMainsHz(50);
    model->cycleNotch();
    QCOMPARE(model->mainsHz(), 60);
    QVERIFY(model->notchEnabled());
    model->cycleNotch();
    QVERIFY(!model->notchEnabled());
    model->cycleNotch();
    QCOMPARE(model->mainsHz(), 50);
    QVERIFY(model->notchEnabled());

    // Anything but 60 is 50: there is no third mains frequency.
    model->setMainsHz(55);
    QCOMPARE(model->mainsHz(), 50);
}

void TstSignalModel::kickerCarriesDomainSignalAndRate()
{
    // The spec's own example, character for character.
    model->setSignal(SignalModel::Ecg);
    QCOMPARE(model->kicker(), QStringLiteral("TIME DOMAIN · ECG · 250 Hz sampling"));

    model->setDomain(SignalModel::Frequency);
    QCOMPARE(model->kicker(), QStringLiteral("FREQUENCY DOMAIN · ECG · 250 Hz sampling"));

    model->setSignal(SignalModel::Emg);
    QCOMPARE(model->kicker(), QStringLiteral("FREQUENCY DOMAIN · EMG · 1 kHz sampling"));
}

void TstSignalModel::titlesAndRailMeta()
{
    model->setSignal(SignalModel::Ecg);
    QCOMPARE(model->title(), QStringLiteral("Electrocardiogram"));
    model->setSignal(SignalModel::Eeg);
    QCOMPARE(model->title(), QStringLiteral("Electroencephalogram"));
    model->setSignal(SignalModel::Emg);
    QCOMPARE(model->title(), QStringLiteral("Electromyogram"));

    // Spec section 4, offline (so the spec's nominal rates, not a device's).
    QCOMPARE(model->railMeta(),
             (QStringList{QStringLiteral("3 leads · 250 Hz"),
                          QStringLiteral("4 channels · 256 Hz"),
                          QStringLiteral("2 channels · 1 kHz")}));
}

void TstSignalModel::timeTicksMatchTheSpecTable()
{
    // Section 5.4: 2.5 s -> 0.5 s steps, 5 s -> 1 s, 10 s -> 2 s.
    model->setWindowSeconds(2.5);
    QCOMPARE(model->timeTicks(), (QStringList{"0 s", "0.5 s", "1 s", "1.5 s", "2 s", "2.5 s"}));
    model->setWindowSeconds(5.0);
    QCOMPARE(model->timeTicks(), (QStringList{"0 s", "1 s", "2 s", "3 s", "4 s", "5 s"}));
    model->setWindowSeconds(10.0);
    QCOMPARE(model->timeTicks(), (QStringList{"0 s", "2 s", "4 s", "6 s", "8 s", "10 s"}));
}

void TstSignalModel::rowsAndScaleLabels()
{
    auto scales = [&] {
        QStringList s;
        for (const QVariant &r : model->rows()) {
            s << r.toMap().value("scale").toString();
        }
        return s;
    };
    auto names = [&] {
        QStringList s;
        for (const QVariant &r : model->rows()) {
            s << r.toMap().value("name").toString() + "/" + r.toMap().value("location").toString();
        }
        return s;
    };

    // Rows, section 5.3.
    model->setSignal(SignalModel::Ecg);
    QCOMPARE(names(), (QStringList{"I/Limb lead", "II/Limb lead", "V1/Precordial"}));
    model->setSignal(SignalModel::Eeg);
    QCOMPARE(names(), (QStringList{"Fp1/Frontal", "C3/Central", "O1/Occipital", "O2/Occipital"}));
    model->setSignal(SignalModel::Emg);
    QCOMPARE(names(), (QStringList{"Biceps/Right arm", "Triceps/Right arm"}));

    // Scale = RANGE / gain, section 5.3: ECG and EMG 1.6 mV, EEG 70 uV.
    const QString pm = QStringLiteral("±");
    model->setSignal(SignalModel::Ecg);
    model->setGain(1.0);
    QCOMPARE(scales().first(), pm + QStringLiteral("1.6 mV"));
    model->setGain(2.0);
    QCOMPARE(scales().first(), pm + QStringLiteral("0.8 mV"));
    model->setGain(0.5);
    QCOMPARE(scales().first(), pm + QStringLiteral("3.2 mV"));

    model->setSignal(SignalModel::Eeg);
    model->setGain(1.0);
    QCOMPARE(scales().first(), pm + QStringLiteral("70 µV"));
    model->setGain(2.0);
    QCOMPARE(scales().first(), pm + QStringLiteral("35 µV"));
    model->setGain(0.5);
    QCOMPARE(scales().first(), pm + QStringLiteral("140 µV"));

    // Frequency mode: "dB . rel".
    model->setDomain(SignalModel::Frequency);
    QCOMPARE(scales().first(), QStringLiteral("dB · rel"));

    // And the half-range the plot actually uses is the same number.
    model->setDomain(SignalModel::Time);
    model->setGain(2.0);
    QCOMPARE(model->halfRangeUv(), 35.0);
}

void TstSignalModel::footerTextsMatchTheSpec()
{
    // Section 5.4. Left: Scroll / Sweep / Frozen in time mode, "Hz" in frequency.
    QCOMPARE(model->footerLeft(), QStringLiteral("Scroll"));
    model->setDisplayMode(SignalModel::Sweep);
    QCOMPARE(model->footerLeft(), QStringLiteral("Sweep"));
    acq->setPaused(true);
    QCOMPARE(model->footerLeft(), QStringLiteral("Frozen"));
    acq->setPaused(false);

    model->setDomain(SignalModel::Frequency);
    QCOMPARE(model->footerLeft(), QStringLiteral("Hz"));

    // Right, frequency mode: the three df values the spec prints.
    model->setSignal(SignalModel::Ecg);
    QCOMPARE(model->footerRight(), QStringLiteral("FFT 512 pt · Hann · Δf 0.49 Hz"));
    model->setSignal(SignalModel::Eeg);
    QCOMPARE(model->footerRight(), QStringLiteral("FFT 512 pt · Hann · Δf 0.50 Hz"));
    model->setSignal(SignalModel::Emg);
    QCOMPARE(model->footerRight(), QStringLiteral("FFT 512 pt · Hann · Δf 1.95 Hz"));

    model->setDomain(SignalModel::Time);
    QVERIFY(model->footerRight().isEmpty());
}

void TstSignalModel::windowAndGainSnapToTheirChoices()
{
    // The spec offers three windows and three gains; anything else snaps to the
    // nearest, so a stray value from a settings file cannot put the UI in a state
    // its controls cannot represent.
    model->setWindowSeconds(4.0);
    QCOMPARE(model->windowSeconds(), 5.0);
    model->setWindowSeconds(100.0);
    QCOMPARE(model->windowSeconds(), 10.0);
    model->setWindowSeconds(0.1);
    QCOMPARE(model->windowSeconds(), 2.5);

    model->setGain(1.4);
    QCOMPARE(model->gain(), 1.0);
    model->setGain(9.0);
    QCOMPARE(model->gain(), 2.0);
    model->setGain(0.0);
    QCOMPARE(model->gain(), 0.5);
}

void TstSignalModel::settingsPersist()
{
    model->setSignal(SignalModel::Emg);
    model->setWindowSeconds(10.0);
    model->setGain(2.0);
    model->setMainsHz(60);
    model->setGlow(false);
    model->setPatientLabel(QStringLiteral("Patient 7"));
    delete model;

    // A new model over the same settings comes back as it was left. This is also
    // what keeps the user's mains frequency across the viewer rebuild.
    model = new SignalModel(acq);
    QCOMPARE(model->signal(), int(SignalModel::Emg));
    QCOMPARE(model->windowSeconds(), 10.0);
    QCOMPARE(model->gain(), 2.0);
    QCOMPARE(model->mainsHz(), 60);
    QVERIFY(!model->glow());
    QCOMPARE(model->patientLabel(), QStringLiteral("Patient 7"));
}

// =================================================================== state

void TstSignalModel::offlineStatusAndNoRows()
{
    QVERIFY(!model->online());
    QCOMPARE(model->statusText(), QStringLiteral("Device offline"));
    for (int i = 0; i < model->rowCountValue(); ++i) {
        QVERIFY2(!model->rowHasData(i), "an offline row must not claim to have a trace");
    }
    // No heart rate offline: a dash, never a zero (a zero reads as bradycardia).
    model->tickMetrics();
    QCOMPARE(model->metrics().value("hr").toString(), QStringLiteral("—"));
    QCOMPARE(model->metrics().value("tag").toString(), QStringLiteral("No signal"));
}

void TstSignalModel::statusReflectsFreezeAndLive()
{
    QTemporaryDir d;
    const QString path = write(d, "s.emgraw", capture(20, 1000, 2, ecg(72)));
    startReplay(path, 1.0);
    if (QTest::currentTestFailed()) {
        return;
    }       // real time: stays running while we look

    QCOMPARE(model->statusText(), QStringLiteral("Live"));
    model->setPaused(true);
    QCOMPARE(model->statusText(), QStringLiteral("Display frozen"));
    model->setPaused(false);
    QCOMPARE(model->statusText(), QStringLiteral("Live"));
}

void TstSignalModel::signalSwitchDropsOldHistoryAndResumesLive()
{
    QTemporaryDir d;
    const QString path = write(d, "s.emgraw", capture(90, 1000, 4, ecg(72)));
    startReplay(path);
    if (QTest::currentTestFailed()) {
        return;
    }
    waitForSamples(3000);
    const std::size_t before = model->availableSamples();
    QVERIFY(before >= 3000);

    // Switching changes the filter band, and the ring still holds the previous
    // band's output; the display must not show it under the new title.
    model->setPaused(true);
    model->setSignal(SignalModel::Emg);

    QVERIFY2(!model->paused(), "switching signal resumes live");
    QVERIFY2(model->availableSamples() < before / 2,
             qPrintable(QStringLiteral("history kept: %1 samples").arg(model->availableSamples())));
}

void TstSignalModel::markersAreNumberedPerSignal()
{
    QTemporaryDir d;
    const QString path = write(d, "s.emgraw", capture(30, 1000, 4, ecg(72)));
    startReplay(path, 1.0);
    if (QTest::currentTestFailed()) {
        return;
    }

    model->setSignal(SignalModel::Ecg);
    model->addMarker();
    model->addMarker();
    model->setSignal(SignalModel::Emg);
    model->addMarker();
    model->setSignal(SignalModel::Ecg);
    model->addMarker();

    // ECG has M1, M2, M3 - numbered within the view, so each reads from M1.
    QCOMPARE(model->markerCount(), 3);
    const QVariantList recent = model->recentMarkers();
    QCOMPARE(recent.size(), 3);
    QCOMPARE(recent.at(0).toMap().value("tag").toString(), QStringLiteral("M3"));   // latest first
    QCOMPARE(recent.at(2).toMap().value("tag").toString(), QStringLiteral("M1"));

    model->setSignal(SignalModel::Emg);
    QCOMPARE(model->markerCount(), 1);
    QCOMPARE(model->recentMarkers().first().toMap().value("tag").toString(), QStringLiteral("M1"));

    // At most five are listed.
    model->setSignal(SignalModel::Ecg);
    for (int i = 0; i < 5; ++i) {
        model->addMarker();
    }
    QCOMPARE(model->markerCount(), 8);
    QCOMPARE(model->recentMarkers().size(), 5);

    // A marker is stamped with the ring position, so it can be drawn at its time.
    QVERIFY(model->markersForView().last().sampleIndex > 0);
}

void TstSignalModel::markersRefusedOffline()
{
    model->addMarker();
    QCOMPARE(model->markerCount(), 0);   // nothing to annotate
}

// ============================================================ measurement

void TstSignalModel::ecgRateIsMeasuredNotAssumed()
{
    QTemporaryDir d;
    const QString path = write(d, "e72.emgraw", capture(90, 1000, 3, ecg(72)));
    startReplay(path);
    if (QTest::currentTestFailed()) {
        return;
    }
    model->setSignal(SignalModel::Ecg);

    // Wait for the detector to lock - a median of recent beats needs several.
    QTRY_VERIFY_WITH_TIMEOUT((model->tickMetrics(),
                              model->metrics().value("hr").toString() != QStringLiteral("—")),
                             10000);
    const int hr = model->metrics().value("hr").toString().toInt();
    QVERIFY2(std::abs(hr - 72) <= 1, qPrintable(QStringLiteral("read %1 BPM from a 72 BPM source").arg(hr)));

    // RR is 60000 / HR, as the spec says: 833 ms.
    const int rr = model->metrics().value("rr").toString().toInt();
    QVERIFY2(std::abs(rr - 833) <= 14, qPrintable(QString::number(rr)));

    // In range, so the tag is a rate verdict - NOT the spec's "Sinus rhythm", which
    // is a diagnosis nothing here makes.
    QCOMPARE(model->metrics().value("tag").toString(), QStringLiteral("Within limits"));
    QVERIFY(model->metrics().value("tagAccent").toBool());
    QCOMPARE(model->railValues().at(0), QStringLiteral("%1 bpm").arg(hr));

    // The measurements nothing makes are dashes, not numbers.
    for (const char *k : {"pr", "qrs", "qtc"}) {
        QCOMPARE(model->metrics().value(k).toString(), QStringLiteral("—"));
    }
    QCOMPARE(model->metrics().value("limits").toString(), QStringLiteral("Limits 50–120"));
}

void TstSignalModel::slowHeartRateIsFlaggedAgainstTheLimits()
{
    QTemporaryDir d;
    const QString path = write(d, "e30.emgraw", capture(120, 1000, 3, ecg(30)));
    startReplay(path);
    if (QTest::currentTestFailed()) {
        return;
    }
    model->setSignal(SignalModel::Ecg);

    QTRY_VERIFY_WITH_TIMEOUT((model->tickMetrics(),
                              model->metrics().value("hr").toString() != QStringLiteral("—")),
                             15000);
    const int hr = model->metrics().value("hr").toString().toInt();
    QVERIFY2(std::abs(hr - 30) <= 1, qPrintable(QString::number(hr)));
    QCOMPARE(model->metrics().value("tag").toString(), QStringLiteral("Below limit"));
}

void TstSignalModel::eegFindsTheAlphaPeakAndBandMix()
{
    // 10.2 Hz alpha on O1 (channel 2), with weaker theta and beta underneath.
    auto eeg = [](int c, double t) {
        const double a = (c == 2) ? 40.0 : 8.0;
        return 2500.0 + a * std::sin(2 * kPi * 10.2 * t) + 6.0 * std::sin(2 * kPi * 6.0 * t + c) +
               4.0 * std::sin(2 * kPi * 21.0 * t + c);
    };
    QTemporaryDir d;
    const QString path = write(d, "eeg.emgraw", capture(90, 1000, 4, eeg));
    startReplay(path);
    if (QTest::currentTestFailed()) {
        return;
    }
    model->setSignal(SignalModel::Eeg);
    waitForSamples(3500);                    // 512 samples x decimation 4, plus margin

    QTRY_VERIFY_WITH_TIMEOUT((model->tickSpectra(true), model->tickMetrics(),
                              model->metrics().value("dominant").toString() != QStringLiteral("—")),
                             10000);

    const double dom = model->metrics().value("dominant").toString().toDouble();
    QVERIFY2(std::abs(dom - 10.2) < 0.2, qPrintable(QString::number(dom)));
    QCOMPARE(model->metrics().value("bandTag").toString(), QStringLiteral("Alpha band"));
    QCOMPARE(model->railValues().at(1), QStringLiteral("%1 Hz").arg(dom, 0, 'f', 1));

    // Five bands, percentages that make a whole, and alpha the dominant one.
    const QVariantList bands = model->metrics().value("bands").toList();
    QCOMPARE(bands.size(), 5);
    double total = 0.0;
    int dominant = -1;
    for (int i = 0; i < bands.size(); ++i) {
        total += bands.at(i).toMap().value("percent").toDouble();
        if (bands.at(i).toMap().value("dominant").toBool()) {
            dominant = i;
        }
    }
    QVERIFY2(std::abs(total - 100.0) < 0.01, qPrintable(QString::number(total)));
    QCOMPARE(dominant, 2);   // delta, theta, ALPHA, beta, gamma
    QVERIFY(bands.at(2).toMap().value("percent").toDouble() > 80.0);
}

// The firmware sends INFO every second, and each one used to rebuild the spectrum
// engines - wiping them. In the frequency view they refresh every 83 ms so nobody
// noticed; in the time view the EEG and EMG cards read from an engine that is only
// updated every ~400 ms, and they went blank at every INFO.
void TstSignalModel::metricsSurviveTheDevicesPeriodicInfoFrames()
{
    auto eeg = [](int c, double t) {
        return 2500.0 + ((c == 2) ? 40.0 : 8.0) * std::sin(2 * kPi * 10.2 * t);
    };
    QTemporaryDir d;
    const QString path = write(d, "eeg.emgraw", capture(60, 1000, 4, eeg));
    startReplay(path, 4.0);
    if (QTest::currentTestFailed()) {
        return;
    }
    model->setSignal(SignalModel::Eeg);
    model->setDomain(SignalModel::Frequency);        // engines refresh constantly here

    QTRY_VERIFY_WITH_TIMEOUT((model->tickSpectra(false), model->spectrum(2).valid()), 15000);

    // Now let several INFO frames pass (one per second of recording, 4x speed) and
    // confirm the engine is NEVER observed invalid again.
    const std::uint64_t startInfo = acq->stats().infoFrames;
    int observations = 0;
    while (acq->stats().infoFrames < startInfo + 3 && acq->isRunning()) {
        QTest::qWait(30);
        QVERIFY2(model->spectrum(2).valid(), "an INFO frame reset the spectrum engine");
        ++observations;
    }
    QVERIFY2(observations > 5, "never saw any further INFO frames");
}

// The normal, un-forced path: the timer's call, in the time domain. Forcing it, as
// most of this suite does for determinism, skips exactly the gating that was wrong.
void TstSignalModel::metricsAppearInTheTimeDomainWithoutForcing()
{
    auto eeg = [](int c, double t) {
        return 2500.0 + ((c == 2) ? 40.0 : 8.0) * std::sin(2 * kPi * 10.2 * t);
    };
    QTemporaryDir d;
    const QString path = write(d, "eeg.emgraw", capture(90, 1000, 4, eeg));
    startReplay(path, 4.0);
    if (QTest::currentTestFailed()) {
        return;
    }
    model->setSignal(SignalModel::Eeg);
    QCOMPARE(model->domain(), int(SignalModel::Time));

    // Only the model's own timers run - no force, no help.
    QTRY_VERIFY_WITH_TIMEOUT(model->metrics().value("dominant").toString() != QStringLiteral("\u2014"),
                             15000);
    const double dom = model->metrics().value("dominant").toString().toDouble();
    QVERIFY2(std::abs(dom - 10.2) < 0.2, qPrintable(QString::number(dom)));

    // And it STAYS - sampled across a few INFO frames, never blank again.
    int blanks = 0;
    for (int i = 0; i < 40; ++i) {
        QTest::qWait(50);
        if (model->metrics().value("dominant").toString() == QStringLiteral("\u2014")) {
            ++blanks;
        }
    }
    QVERIFY2(blanks == 0, qPrintable(QStringLiteral("blank in %1 of 40 samples").arg(blanks)));
}

void TstSignalModel::emgRmsAndMedianFromAKnownTone()
{
    // A 120 Hz tone: RMS = A / sqrt(2). Biceps A = 500 uV -> 354 uV -> "0.35 mV".
    // Triceps A = 50 uV -> 35 uV -> "0.04 mV". 120 Hz sits clear of the 50 Hz
    // notch and inside the 20-450 Hz band, so the filter should leave it alone.
    auto emg = [](int c, double t) {
        const double a = (c == 0) ? 500.0 : 50.0;
        return 1500.0 + a * std::sin(2 * kPi * 120.0 * t);
    };
    QTemporaryDir d;
    const QString path = write(d, "emg.emgraw", capture(90, 2000, 2, emg));
    startReplay(path);
    if (QTest::currentTestFailed()) {
        return;
    }
    model->setSignal(SignalModel::Emg);
    waitForSamples(2000);

    QTRY_VERIFY_WITH_TIMEOUT((model->tickSpectra(true), model->tickMetrics(),
                              model->metrics().value("rms").toString() != QStringLiteral("—") &&
                                  model->metrics().value("median").toString() != QStringLiteral("—")),
                             10000);

    const double rms = model->metrics().value("rms").toString().toDouble();
    QVERIFY2(std::abs(rms - 0.354) < 0.02, qPrintable(QStringLiteral("biceps RMS %1 mV").arg(rms)));
    const double tri = model->metrics().value("triceps").toString().toDouble();
    QVERIFY2(std::abs(tri - 0.035) < 0.01, qPrintable(QStringLiteral("triceps RMS %1 mV").arg(tri)));

    // 0.35 mV is above the 0.12 mV threshold: a contraction. The weak channel is not.
    QCOMPARE(model->metrics().value("state").toString(), QStringLiteral("Contraction"));
    QVERIFY(model->metrics().value("stateAccent").toBool());

    const double median = model->metrics().value("median").toString().toDouble();
    QVERIFY2(std::abs(median - 120.0) < 6.0, qPrintable(QStringLiteral("median %1 Hz").arg(median)));
    QCOMPARE(model->railValues().at(2), QStringLiteral("%1 mV").arg(rms, 0, 'f', 2));
}

void TstSignalModel::mvcReferenceScalesTheActivationBar()
{
    auto emg = [](int, double t) { return 1000.0 + 400.0 * std::sin(2 * kPi * 100.0 * t); };
    QTemporaryDir d;
    const QString path = write(d, "emg.emgraw", capture(90, 2000, 2, emg));
    startReplay(path);
    if (QTest::currentTestFailed()) {
        return;
    }
    model->setSignal(SignalModel::Emg);

    // No reference yet: a dash and an empty bar, not a number against a guess.
    QTRY_VERIFY_WITH_TIMEOUT((model->tickMetrics(),
                              model->metrics().value("rms").toString() != QStringLiteral("—")),
                             10000);
    QVERIFY(!model->metrics().value("mvcSet").toBool());
    QCOMPARE(model->metrics().value("mvcText").toString(), QStringLiteral("—"));
    QCOMPARE(model->metrics().value("mvcFraction").toDouble(), 0.0);

    // Capture the current level as 100 %. The same steady tone is then ~100 %.
    QTest::qWait(900);                       // let a few metric samples accumulate
    for (int i = 0; i < 3; ++i) {
        model->tickMetrics();
    }
    model->captureMvc();
    QVERIFY(model->metrics().value("mvcSet").toBool());
    const int pct = model->metrics().value("mvcText").toString().toInt();
    QVERIFY2(pct >= 90 && pct <= 110, qPrintable(QString::number(pct)));

    model->clearMvc();
    QVERIFY(!model->metrics().value("mvcSet").toBool());
}

// Median frequency is the fatigue index of a CONTRACTING muscle. Computed on an
// idle channel it is the median of the amplifier's noise floor - a confident number
// about nothing, which is the same mistake Spectrum.h documents for ECG ("a number
// about the noise, presented as though it were about the heart").
void TstSignalModel::medianFrequencyIsNotReportedForNoise()
{
    // 5 uV of 120 Hz on both channels: RMS 3.5 uV, far below any real contraction.
    auto idle = [](int, double t) { return 1500.0 + 5.0 * std::sin(2 * kPi * 120.0 * t); };
    QTemporaryDir d;
    const QString path = write(d, "idle.emgraw", capture(90, 2000, 2, idle));
    startReplay(path);
    if (QTest::currentTestFailed()) {
        return;
    }
    model->setSignal(SignalModel::Emg);
    waitForSamples(2000);

    QTRY_VERIFY_WITH_TIMEOUT((model->tickSpectra(true), model->tickMetrics(),
                              model->metrics().value("rms").toString() != QStringLiteral("\u2014")),
                             10000);
    QVERIFY(model->spectrum(0).valid());             // the engine HAS a median...
    QCOMPARE(model->metrics().value("state").toString(), QStringLiteral("Rest"));
    // ...and the card refuses to show it.
    QCOMPARE(model->metrics().value("median").toString(), QStringLiteral("\u2014"));
}

// ======================================================== device manager

void TstSignalModel::replayReportsAsAConnectedDevice()
{
    DeviceManager dev(acq);
    QCOMPARE(dev.state(), DeviceManager::Off);
    QCOMPARE(dev.label(), QStringLiteral("Connect device"));
    QCOMPARE(dev.iconName(), QStringLiteral("wifi-slash"));

    QTemporaryDir d;
    const QString path = write(d, "short.emgraw", capture(4, 1000, 2, ecg(72)));
    dev.openReplayFile(path);
    acq->setReplaySpeed(1.0);
    QTRY_COMPARE_WITH_TIMEOUT(dev.state(), DeviceManager::Connected, 5000);
    QCOMPARE(dev.label(), QStringLiteral("short.emgraw"));
    // A replay is not a Wi-Fi link, so it must not get a Wi-Fi icon.
    QCOMPARE(dev.iconName(), QStringLiteral("pulse"));

    // Running to the end reads as "finished", not as a dropped connection.
    QTRY_COMPARE_WITH_TIMEOUT(dev.state(), DeviceManager::Off, 15000);
    QCOMPARE(dev.statusLine(), QStringLiteral("Replay finished."));
}

void TstSignalModel::deliberateDisconnectIsNotALostConnection()
{
    DeviceManager dev(acq);
    QTemporaryDir d;
    const QString path = write(d, "long.emgraw", capture(60, 1000, 2, ecg(72)));
    dev.openReplayFile(path);
    acq->setReplaySpeed(1.0);
    QTRY_COMPARE_WITH_TIMEOUT(dev.state(), DeviceManager::Connected, 5000);

    dev.disconnectDevice();
    QCOMPARE(dev.state(), DeviceManager::Off);
    QVERIFY2(!dev.statusLine().contains(QStringLiteral("lost"), Qt::CaseInsensitive),
             qPrintable(dev.statusLine()));
    QCOMPARE(dev.label(), QStringLiteral("Connect device"));
}

void TstSignalModel::previewListIsTheSpecsThreeNetworksAndCannotConnect()
{
    DeviceManager dev(acq);
    dev.setPreviewMode(true);

    auto *m = qobject_cast<DeviceListModel *>(dev.devices());
    QVERIFY(m);
    QCOMPARE(m->count(), 3);
    QCOMPARE(m->at(0)->name, QStringLiteral("BioAmp-8CH-3F2A"));
    QCOMPARE(m->at(0)->meta, QStringLiteral("Amplifier · WPA2 · 5 GHz"));
    QCOMPARE(m->at(1)->name, QStringLiteral("BioAmp-8CH-71C0"));
    QCOMPARE(m->at(2)->name, QStringLiteral("Ward4-Telemetry"));
    QCOMPARE(dev.statusLine(), QStringLiteral("3 devices nearby"));

    // Locked networks need a passkey, and the field only shows for them.
    QVERIFY(dev.needsPasskey());
    dev.connectSelected();
    QVERIFY2(dev.statusLine().contains(QStringLiteral("passkey")), qPrintable(dev.statusLine()));
    QCOMPARE(dev.state(), DeviceManager::Off);

    // Even with a passkey these are preview rows: they say so rather than "connect".
    dev.setPasskey(QStringLiteral("x"));
    dev.connectSelected();
    QVERIFY2(dev.statusLine().contains(QStringLiteral("not available")), qPrintable(dev.statusLine()));
    QCOMPARE(dev.state(), DeviceManager::Off);

    // A passkey typed for one network must not follow the selection to another.
    dev.setSelectedIndex(1);
    QVERIFY(dev.passkey().isEmpty());
}

void TstSignalModel::dialogTitleIsHonestAboutTheTransport()
{
    // "over Wi-Fi" is true only when Wi-Fi devices are listed. Over a serial port
    // it would be wrong on the one screen that says what you are connecting to.
    DeviceManager dev(acq);
    QCOMPARE(dev.dialogTitle(), QStringLiteral("Connect amplifier"));
    dev.setPreviewMode(true);
    QCOMPARE(dev.dialogTitle(), QStringLiteral("Connect amplifier over Wi-Fi"));
}

QTEST_GUILESS_MAIN(TstSignalModel)
#include "tst_signalmodel.moc"
