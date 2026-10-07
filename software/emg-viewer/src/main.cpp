#include <QCommandLineParser>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QPalette>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>

#include "model/Acquisition.h"
#include "model/DeviceManager.h"
#include "model/SignalModel.h"

namespace {

// Register the bundled fonts. The spec asks for Inter at weights 400 and 500
// only, with tabular figures on every number; Qt 6.4 cannot switch OpenType
// features on, so the figures come from a derived family (tools/make_fonts.py).
void loadFonts()
{
    const char *files[] = {
        ":/fonts/Inter-Regular.otf",
        ":/fonts/Inter-Medium.otf",
        ":/fonts/InterTabular-Regular.otf",
        ":/fonts/InterTabular-Medium.otf",
        ":/fonts/Phosphor.ttf",
    };
    for (const char *f : files) {
        if (QFontDatabase::addApplicationFont(QString::fromLatin1(f)) < 0) {
            // Not fatal - Qt falls back to a system font - but the layout was
            // designed around Inter's metrics, so say so.
            qWarning("could not load bundled font %s", f);
        }
    }
}

// Qt Quick Controls' Basic style draws control text and selection from the
// application palette, not from anything QML sets, so an unset palette gives
// near-black text on the dark surface. Keep in step with qml/Theme.qml.
void applyPalette()
{
    QPalette p;
    const QColor bg(0x16, 0x18, 0x26), surface(0x23, 0x25, 0x32), text(0xE9, 0xE9, 0xED);
    const QColor muted(0x93, 0x97, 0xAB), accent(0x91, 0x84, 0xD9);

    p.setColor(QPalette::Window, bg);
    p.setColor(QPalette::WindowText, text);
    p.setColor(QPalette::Base, surface);
    p.setColor(QPalette::AlternateBase, bg);
    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Button, surface);
    p.setColor(QPalette::ButtonText, text);
    p.setColor(QPalette::Highlight, accent);
    p.setColor(QPalette::HighlightedText, bg);
    p.setColor(QPalette::PlaceholderText, muted);
    p.setColor(QPalette::ToolTipBase, surface);
    p.setColor(QPalette::ToolTipText, text);
    p.setColor(QPalette::Disabled, QPalette::Text, muted);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, muted);
    p.setColor(QPalette::Disabled, QPalette::WindowText, muted);
    QGuiApplication::setPalette(p);
}

} // namespace

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // The application name keys QSettings. It is deliberately the one the
    // previous viewer used, so the mains frequency and notch choice carry over.
    QGuiApplication::setApplicationName(QStringLiteral("EMG Viewer"));
    QGuiApplication::setOrganizationName(QStringLiteral("FES_Board"));
    QGuiApplication::setApplicationDisplayName(QStringLiteral("BioView Monitor"));
    QGuiApplication::setApplicationVersion(QStringLiteral(EMG_VIEWER_VERSION));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("BioView Monitor - live ECG / EEG / EMG display for the FES_Board."));
    parser.addHelpOption();
    parser.addVersionOption();

    auto opt = [](const char *names, const char *help, const char *value = nullptr,
                  const char *def = nullptr) {
        QStringList n = QString::fromLatin1(names).split(QLatin1Char(','));
        return QCommandLineOption(n, QString::fromLatin1(help),
                                  value ? QString::fromLatin1(value) : QString(),
                                  def ? QString::fromLatin1(def) : QString());
    };

    // ---- connect -----------------------------------------------------------
    const auto replayOpt = opt("r,replay", "Replay a recorded .emgraw capture.", "file");
    const auto replaySpeedOpt = opt("replay-speed", "Replay pacing, 1 = recorded speed.", "x", "1");
    const auto portOpt = opt("p,port", "Serial port to connect to, e.g. ttyACM0.", "port");
    const auto baudOpt = opt("b,baud", "Baud rate.", "rate", "921600");
    const auto wifiOpt = opt("wifi", "Connect to a board over Wi-Fi at host[:port] (port defaults to 5000).", "host[:port]");

    // ---- view (the spec names these settings but gives no control for them) --
    const auto signalOpt = opt("signal", "Signal type: ecg, eeg or emg.", "type");
    const auto domainOpt = opt("domain", "time or freq.", "domain");
    const auto windowOpt = opt("window", "Time window: 2.5, 5 or 10 seconds.", "s");
    const auto gainOpt = opt("gain", "Display gain: 0.5, 1 or 2.", "x");
    const auto avgOpt = opt("averaging", "Spectrum averaging: low, med or high.", "level");
    const auto modeOpt = opt("display-mode", "scroll or sweep.", "mode");
    const auto noGlowOpt = opt("no-glow", "Draw traces without the glow.");
    const auto barsOpt = opt("spectrum-bars", "Draw the spectrum as bars instead of a filled line.");
    const auto mainsOpt = opt("mains", "Mains frequency for the notch: 50 or 60.", "hz");
    const auto patientOpt = opt("patient", "Text for the header's patient label.", "text");
    const auto sessionOpt = opt("session", "Text for the header's session label.", "text");

    // ---- development ---------------------------------------------------------
    const auto openDevicesOpt = opt("open-devices", "Open the device dialog on start.");
    const auto previewOpt = opt("preview-devices",
                                "List the spec's three example networks in the device dialog, for "
                                "layout review only. They cannot connect.");
    const auto previewWifiOpt = opt("preview-wifi",
                                    "Put the Wi-Fi tab and the header button in one of the design's states "
                                    "(manual, searching, connecting, connected, nodata, error), for "
                                    "layout review only (add --open-devices to see the dialog). Nothing connects.", "phase");
    const auto diagOpt = opt("diagnostics", "Open the diagnostics drawer on start.");
    const auto freezeOpt = opt("freeze-after", "Freeze the display after <ms>.", "ms");
    const auto markerOpt = opt("marker-after", "Drop a marker after <ms>.", "ms");
    const auto sizeOpt = opt("size", "Initial window size, e.g. 1440x900.", "WxH");
    const auto grabOpt = opt("grab", "Save a screenshot after --grab-delay and exit.", "file");
    const auto grabDelayOpt = opt("grab-delay", "Milliseconds before --grab.", "ms", "3000");

    for (const auto &o : {replayOpt, replaySpeedOpt, portOpt, baudOpt, wifiOpt, signalOpt, domainOpt,
                          windowOpt, gainOpt, avgOpt, modeOpt, noGlowOpt, barsOpt, mainsOpt,
                          patientOpt, sessionOpt, openDevicesOpt, previewOpt, previewWifiOpt, diagOpt,
                          freezeOpt, markerOpt, sizeOpt, grabOpt, grabDelayOpt}) {
        parser.addOption(o);
    }
    parser.process(app);

    QQuickStyle::setStyle(QStringLiteral("Basic"));
    loadFonts();
    applyPalette();

    // ---- the three objects QML binds to ---------------------------------------
    Acquisition acquisition(nullptr);
    SignalModel signalModel(&acquisition);
    DeviceManager devices(&acquisition);
    Acquisition::setInstance(&acquisition);
    SignalModel::setInstance(&signalModel);
    DeviceManager::setInstance(&devices);

    // ---- command-line view settings, applied before the window exists ----------
    auto sigIdx = [](const QString &s) { return s == "eeg" ? 1 : (s == "emg" ? 2 : 0); };
    if (parser.isSet(signalOpt)) {
        signalModel.setSignal(sigIdx(parser.value(signalOpt).toLower()));
    }
    if (parser.isSet(domainOpt)) {
        signalModel.setDomain(parser.value(domainOpt).startsWith(QLatin1String("f"), Qt::CaseInsensitive)
                                  ? SignalModel::Frequency
                                  : SignalModel::Time);
    }
    if (parser.isSet(windowOpt)) {
        signalModel.setWindowSeconds(parser.value(windowOpt).toDouble());
    }
    if (parser.isSet(gainOpt)) {
        signalModel.setGain(parser.value(gainOpt).toDouble());
    }
    if (parser.isSet(avgOpt)) {
        const QString a = parser.value(avgOpt).toLower();
        signalModel.setAveraging(a.startsWith(QLatin1String("l")) ? SignalModel::Low
                                 : a.startsWith(QLatin1String("h")) ? SignalModel::High
                                                                    : SignalModel::Medium);
    }
    if (parser.isSet(modeOpt)) {
        signalModel.setDisplayMode(parser.value(modeOpt).startsWith(QLatin1String("sw"), Qt::CaseInsensitive)
                                       ? SignalModel::Sweep
                                       : SignalModel::Scroll);
    }
    if (parser.isSet(noGlowOpt)) {
        signalModel.setGlow(false);
    }
    if (parser.isSet(barsOpt)) {
        signalModel.setSpectrumStyle(SignalModel::Bars);
    }
    if (parser.isSet(mainsOpt)) {
        signalModel.setMainsHz(parser.value(mainsOpt).toInt());
    }
    if (parser.isSet(patientOpt)) {
        signalModel.setPatientLabel(parser.value(patientOpt));
    }
    if (parser.isSet(sessionOpt)) {
        signalModel.setSessionLabel(parser.value(sessionOpt));
    }
    if (parser.isSet(previewOpt)) {
        devices.setPreviewMode(true);
    }
    if (parser.isSet(previewWifiOpt) && !devices.setPreviewWifi(parser.value(previewWifiOpt))) {
        qCritical().noquote() << "unknown --preview-wifi phase:" << parser.value(previewWifiOpt);
        return 2;
    }

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    // Qt 6.5 added loadFromModule(); the 6.4 baseline loads the module's resource
    // URL directly. The prefix is pinned in CMakeLists.txt.
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/EmgViewer/Main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());

    if (window && parser.isSet(sizeOpt)) {
        const QStringList wh = parser.value(sizeOpt).split(QLatin1Char('x'));
        if (wh.size() == 2) {
            window->resize(wh[0].toInt(), wh[1].toInt());
        }
    }

    // ---- connect, once the UI exists so status and errors are visible ----------
    QTimer::singleShot(0, &app, [&] {
        if (parser.isSet(replayOpt)) {
            devices.openReplayFile(parser.value(replayOpt));
            acquisition.setReplaySpeed(parser.value(replaySpeedOpt).toDouble());
        } else if (parser.isSet(wifiOpt)) {
            // The dialog's own path again: set the address, then press Connect.
            const QString v = parser.value(wifiOpt);
            const int colon = v.lastIndexOf(QLatin1Char(':'));
            devices.setWifiHost(colon > 0 ? v.left(colon) : v);
            devices.setWifiPort(colon > 0 ? v.mid(colon + 1).toInt() : wifiaddress::kDefaultPort);
            devices.connectWifi();
        } else if (parser.isSet(portOpt)) {
            // Reuse the dialog's own path so there is exactly one way to connect.
            devices.scan();
            QTimer::singleShot(600, &app, [&] {
                const QString want = parser.value(portOpt);
                auto *m = qobject_cast<DeviceListModel *>(devices.devices());
                const int i = m ? m->indexOfName(want) : -1;
                if (i >= 0) {
                    devices.setSelectedIndex(i);
                    devices.connectSelected();
                } else {
                    qWarning("port %s not found", qPrintable(want));
                }
            });
        }
        if (window) {
            if (parser.isSet(openDevicesOpt)) {
                QMetaObject::invokeMethod(window, "openDevices");
            }
            if (parser.isSet(diagOpt)) {
                QMetaObject::invokeMethod(window, "openDiagnostics");
            }
        }
    });
    if (parser.isSet(freezeOpt)) {
        QTimer::singleShot(parser.value(freezeOpt).toInt(), &app, [&] { signalModel.setPaused(true); });
    }
    if (parser.isSet(markerOpt)) {
        QTimer::singleShot(parser.value(markerOpt).toInt(), &app, [&] { signalModel.addMarker(); });
    }

    if (parser.isSet(grabOpt)) {
        if (!window) {
            qWarning("--grab: root object is not a window");
            return 2;
        }
        const int delay = parser.value(grabDelayOpt).toInt();
        const QString path = parser.value(grabOpt);

        QTimer::singleShot(delay > 0 ? delay : 3000, &app, [window, path] {
            // grabWindow() blocks until the scene graph has produced a frame, so
            // wait for one rather than calling it cold - on a window that has
            // never rendered it simply never returns.
            auto *once = new QMetaObject::Connection;
            *once = QObject::connect(
                window, &QQuickWindow::frameSwapped, window,
                [window, path, once] {
                    QObject::disconnect(*once);
                    delete once;

                    const QImage image = window->grabWindow();
                    if (image.isNull() || !image.save(path)) {
                        qWarning("--grab: could not save %s", qPrintable(path));
                        QCoreApplication::exit(2);
                        return;
                    }
                    QCoreApplication::quit();
                },
                Qt::QueuedConnection);
            window->update();
        });
    }

    return app.exec();
}
