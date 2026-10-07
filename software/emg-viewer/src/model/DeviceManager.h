#pragma once

// Finding, connecting to and dropping the amplifier - the spec's DeviceManager
// (Biosignal Monitor - Qt Spec.md, sections 3 and 8).
//
// The spec describes a Wi-Fi device list. The only transport the firmware
// actually speaks today is a serial link (921600 baud, framed binary); the Wi-Fi
// firmware in WifiFirmware/ is still a scan-only learning app and defines no data
// protocol. So the list shows REAL serial ports, and nothing here invents a
// network device. The list model is transport-agnostic - each entry carries its
// kind, signal level and whether it needs a passkey - so a Wi-Fi provider can add
// entries later with no change to the dialog.
//
// States are the spec's:  Off -> Connecting -> Connected.

#include "WifiAddress.h"

#include <QAbstractListModel>
#include <QObject>
#include <type_traits>
#include <QString>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

#include <vector>

class Acquisition;
QT_BEGIN_NAMESPACE
class QQmlEngine;
class QJSEngine;
QT_END_NAMESPACE

/// One row of the device dialog.
class DeviceListModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Kind { Serial = 0, Wifi = 1 };

    struct Entry {
        QString name;          ///< "ttyACM0", "BioAmp-8CH-3F2A"
        QString meta;          ///< "STM32 STLink . 921600 baud"
        Kind kind = Serial;
        int level = 3;         ///< 1..3, Wi-Fi signal strength; ignored for serial
        QString port;          ///< serial port name; empty for Wi-Fi
        bool needsPasskey = false;
        bool busy = false;     ///< held by another program
    };

    enum Role {
        NameRole = Qt::UserRole + 1,
        MetaRole,
        KindRole,
        LevelRole,
        StatusRole,
        NeedsPasskeyRole,
        IconRole,
    };

    explicit DeviceListModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setEntries(std::vector<Entry> entries);
    const Entry *at(int row) const;
    int indexOfName(const QString &name) const;
    int count() const { return int(m_entries.size()); }

    /// Which entry is connected / connecting, by name; "" for none.
    void setConnectedName(const QString &name);
    void setConnectingName(const QString &name);

private:
    QString statusFor(const Entry &e) const;

    std::vector<Entry> m_entries;
    QString m_connected;
    QString m_connecting;
};

class DeviceManager : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString label READ label NOTIFY stateChanged)
    Q_PROPERTY(QString iconName READ iconName NOTIFY stateChanged)
    Q_PROPERTY(QString connectedName READ connectedName NOTIFY stateChanged)

    /// The spec's title is "Connect amplifier over Wi-Fi". That is only true when
    /// the list actually holds Wi-Fi devices; over a serial port it would be
    /// wrong on the one screen that tells the user what they are connecting to.
    Q_PROPERTY(QString dialogTitle READ dialogTitle NOTIFY statusLineChanged)

    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(QAbstractItemModel *devices READ devices CONSTANT)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectionChanged)
    Q_PROPERTY(QString passkey READ passkey WRITE setPasskey NOTIFY selectionChanged)

    Q_PROPERTY(bool needsPasskey READ needsPasskey NOTIFY selectionChanged)
    Q_PROPERTY(QString statusLine READ statusLine NOTIFY statusLineChanged)
    Q_PROPERTY(QString connectLabel READ connectLabel NOTIFY selectionChanged)
    Q_PROPERTY(bool canConnect READ canConnect NOTIFY selectionChanged)
    Q_PROPERTY(bool canDisconnect READ canDisconnect NOTIFY stateChanged)

    // ---- Wi-Fi tab (WIFI_DESIGN.md section 4) --------------------------------------------
    // The address is validated and remembered; Connect opens a TCP connection to it.
    // Discovery (mDNS) is not built, so the user types the address.

    /// 0 = USB / serial list, 1 = Wi-Fi. Remembered between runs.
    Q_PROPERTY(int transport READ transport WRITE setTransport NOTIFY transportChanged)
    Q_PROPERTY(QString wifiHost READ wifiHost WRITE setWifiHost NOTIFY wifiChanged)
    Q_PROPERTY(int wifiPort READ wifiPort WRITE setWifiPort NOTIFY wifiChanged)
    /// Empty when the address is fine or not yet typed; otherwise what is wrong with it.
    Q_PROPERTY(QString wifiProblem READ wifiProblem NOTIFY wifiChanged)
    Q_PROPERTY(bool canConnectWifi READ canConnectWifi NOTIFY wifiChanged)
    /// manual | searching | connecting | connected | nodata | error. A real run
    /// produces manual, connecting, connected, nodata and error; "searching" is only
    /// for --preview-wifi.
    Q_PROPERTY(QString wifiPhase READ wifiPhase NOTIFY wifiChanged)
    /// The Wi-Fi card on the Wi-Fi tab: name, "address . port", status text. Empty when
    /// no Wi-Fi device is connected or connecting.
    Q_PROPERTY(QString wifiDeviceName READ wifiDeviceName NOTIFY wifiChanged)
    Q_PROPERTY(QString wifiDeviceMeta READ wifiDeviceMeta NOTIFY wifiChanged)
    Q_PROPERTY(QString wifiDeviceStatus READ wifiDeviceStatus NOTIFY wifiChanged)
    Q_PROPERTY(QString wifiStatusLine READ wifiStatusLine NOTIFY wifiChanged)
    /// Connected, but nothing valid has arrived for a while (WIFI_DESIGN.md section 5).
    Q_PROPERTY(bool noData READ noData NOTIFY stateChanged)

    /// While true the list rescans every few seconds, so a board plugged in with
    /// the dialog open appears without pressing Rescan.
    Q_PROPERTY(bool watching READ watching WRITE setWatching NOTIFY watchingChanged)

public:
    enum State { Off = 0, Connecting = 1, Connected = 2 };
    Q_ENUM(State)

    /// 921600 is what the firmware streams at.
    static constexpr int kBaud = 921600;

    explicit DeviceManager(Acquisition *acq, QObject *parent = nullptr);

    static DeviceManager *create(QQmlEngine *, QJSEngine *);
    static void setInstance(DeviceManager *m) { s_instance = m; }

    State state() const { return m_state; }
    QString label() const;
    QString iconName() const;
    QString connectedName() const { return m_connectedName; }
    QString dialogTitle() const;
    bool scanning() const { return m_scanning; }
    QAbstractItemModel *devices() { return &m_model; }
    int selectedIndex() const { return m_selected; }
    void setSelectedIndex(int i);
    QString passkey() const { return m_passkey; }
    void setPasskey(const QString &p);
    bool needsPasskey() const;
    QString statusLine() const;
    QString connectLabel() const;
    bool canConnect() const;
    bool canDisconnect() const { return m_state == Connected; }
    bool watching() const { return m_watching; }
    void setWatching(bool on);

    /// For --preview-devices: list the spec's three example networks instead of
    /// real ports, purely so the dialog's passkey field and status badges can be
    /// reviewed. They cannot connect.
    void setPreviewMode(bool on);

    int transport() const { return m_transport; }
    void setTransport(int t);
    QString wifiHost() const { return m_wifiHost; }
    void setWifiHost(const QString &h);
    int wifiPort() const { return m_wifiPort; }
    void setWifiPort(int p);
    QString wifiProblem() const;
    bool canConnectWifi() const;
    QString wifiPhase() const;
    QString wifiDeviceName() const;
    QString wifiDeviceMeta() const;
    QString wifiDeviceStatus() const;
    QString wifiStatusLine() const;
    bool noData() const { return m_noData; }

    /// --preview-wifi <phase>: put the Wi-Fi view and the header button into one of
    /// the design's states, for layout review only. Nothing is connected. Returns
    /// false for an unknown phase.
    bool setPreviewWifi(const QString &phase);

    /// Command-line replay: report as a connected device named after the file.
    void openReplayFile(const QString &path);

public slots:
    void scan();
    Q_INVOKABLE void connectSelected();
    /// Connect to wifiHost:wifiPort over TCP (the board's nRF7002 is the server).
    Q_INVOKABLE void connectWifi();
    Q_INVOKABLE void disconnectDevice();

signals:
    void stateChanged();
    void scanningChanged();
    void selectionChanged();
    void statusLineChanged();
    void watchingChanged();
    void transportChanged();
    void wifiChanged();

private slots:
    void onRunningChanged();
    void onSourceError(const QString &message);

private:
    void enumerate();
    void setState(State s);
    void setError(const QString &msg);
    /// manual | connecting | connected | nodata, or the --preview-wifi phase.
    QString effectivePhase() const;
    void reselect(const QString &preferName);

    Acquisition *m_acq;
    DeviceListModel m_model;

    State m_state = Off;
    QString m_connectedName;
    QString m_pendingName;
    QString m_error;
    QString m_passkey;
    int m_selected = -1;

    int m_transport = 0;
    QString m_wifiHost;
    int m_wifiPort = wifiaddress::kDefaultPort;
    QString m_previewWifi;        // "" = real; else the --preview-wifi phase
    bool m_noData = false;
    bool m_viaWifi = false;       // the current/pending link is the Wi-Fi tab's

    bool m_scanning = false;
    bool m_watching = false;
    bool m_preview = false;
    bool m_isReplay = false;     // so end-of-file reads as "finished", not "lost"
    QTimer m_rescanTimer;
    QTimer m_connectTimeout;

    static inline DeviceManager *s_instance = nullptr;
};

// QML singletons must come from create(), which returns the instance main() owns.
// A default constructor would make the QML engine build its own (see Acquisition).
static_assert(!std::is_default_constructible_v<DeviceManager>,
              "a default constructor makes QML ignore DeviceManager::create()");
