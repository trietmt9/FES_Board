#include "DeviceManager.h"

#include "Acquisition.h"

#include <QFileInfo>
#include <QQmlEngine>
#include <QSettings>
#include <QSerialPortInfo>
#include <QUrl>

#include <algorithm>

// ====================================================================== model

int DeviceListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : int(m_entries.size());
}

QHash<int, QByteArray> DeviceListModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {MetaRole, "meta"},
        {KindRole, "kind"},
        {LevelRole, "level"},
        {StatusRole, "status"},
        {NeedsPasskeyRole, "needsPasskey"},
        {IconRole, "iconName"},
    };
}

QString DeviceListModel::statusFor(const Entry &e) const
{
    if (!m_connected.isEmpty() && e.name == m_connected) {
        return QStringLiteral("Connected");
    }
    if (!m_connecting.isEmpty() && e.name == m_connecting) {
        return QStringLiteral("Connecting…");
    }
    if (e.busy) {
        return QStringLiteral("In use");
    }
    if (e.kind == Wifi && e.needsPasskey) {
        return QStringLiteral("Locked");
    }
    return QString();
}

QVariant DeviceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= int(m_entries.size())) {
        return {};
    }
    const Entry &e = m_entries[std::size_t(index.row())];
    switch (role) {
    case NameRole:
        return e.name;
    case MetaRole:
        return e.meta;
    case KindRole:
        return int(e.kind);
    case LevelRole:
        return e.level;
    case StatusRole:
        return statusFor(e);
    case NeedsPasskeyRole:
        return e.needsPasskey;
    case IconRole:
        // A wired link is not a Wi-Fi signal, so it does not get a Wi-Fi icon.
        if (e.kind == Wifi) {
            return e.level >= 3 ? QStringLiteral("wifi-high")
                                : (e.level == 2 ? QStringLiteral("wifi-medium")
                                                : QStringLiteral("wifi-low"));
        }
        return QStringLiteral("pulse");
    default:
        break;
    }
    return {};
}

void DeviceListModel::setEntries(std::vector<Entry> entries)
{
    beginResetModel();
    m_entries = std::move(entries);
    endResetModel();
}

const DeviceListModel::Entry *DeviceListModel::at(int row) const
{
    if (row < 0 || row >= int(m_entries.size())) {
        return nullptr;
    }
    return &m_entries[std::size_t(row)];
}

int DeviceListModel::indexOfName(const QString &name) const
{
    for (std::size_t i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].name == name) {
            return int(i);
        }
    }
    return -1;
}

void DeviceListModel::setConnectedName(const QString &name)
{
    if (m_connected == name) {
        return;
    }
    m_connected = name;
    if (!m_entries.empty()) {
        emit dataChanged(index(0), index(int(m_entries.size()) - 1), {StatusRole});
    }
}

void DeviceListModel::setConnectingName(const QString &name)
{
    if (m_connecting == name) {
        return;
    }
    m_connecting = name;
    if (!m_entries.empty()) {
        emit dataChanged(index(0), index(int(m_entries.size()) - 1), {StatusRole});
    }
}

// ================================================================== manager

DeviceManager *DeviceManager::create(QQmlEngine *, QJSEngine *)
{
    if (s_instance) {
        QQmlEngine::setObjectOwnership(s_instance, QQmlEngine::CppOwnership);
    }
    return s_instance;
}

DeviceManager::DeviceManager(Acquisition *acq, QObject *parent) : QObject(parent), m_acq(acq)
{
    connect(m_acq, &Acquisition::runningChanged, this, &DeviceManager::onRunningChanged);
    connect(m_acq, &Acquisition::sourceError, this, &DeviceManager::onSourceError);

    {
        QSettings s;
        m_wifiHost = s.value(QStringLiteral("wifi/host")).toString();
        const int port = s.value(QStringLiteral("wifi/port"), wifiaddress::kDefaultPort).toInt();
        m_wifiPort = wifiaddress::isValidPort(port) ? port : wifiaddress::kDefaultPort;
        m_transport = s.value(QStringLiteral("device/transport"), 0).toInt() == 1 ? 1 : 0;
    }

    m_rescanTimer.setInterval(3000);
    connect(&m_rescanTimer, &QTimer::timeout, this, &DeviceManager::enumerate);

    // A connection that neither opens nor errors must not leave the dialog on
    // "Connecting..." forever.
    m_connectTimeout.setSingleShot(true);
    m_connectTimeout.setInterval(5000);
    connect(&m_connectTimeout, &QTimer::timeout, this, [this] {
        if (m_state == Connecting) {
            m_acq->disconnectSource();
            m_model.setConnectingName(QString());
            setError(tr("No response from %1.").arg(m_pendingName));
            setState(Off);
        }
    });

    enumerate();
}

// ------------------------------------------------------------ presentation

QString DeviceManager::label() const
{
    switch (m_state) {
    case Connected:
        // The spec's palette has no warning colour, so "connected but silent" is said
        // in words, not coloured.
        return m_noData ? m_connectedName + QStringLiteral(" · no data") : m_connectedName;
    case Connecting:
        return QStringLiteral("Connecting…");
    case Off:
        break;
    }
    return QStringLiteral("Connect device");
}

QString DeviceManager::iconName() const
{
    switch (m_state) {
    case Connected: {
        if (!m_previewWifi.isEmpty()) {
            return m_noData ? QStringLiteral("wifi-medium") : QStringLiteral("wifi-high");
        }
        const auto *e = m_model.at(m_model.indexOfName(m_connectedName));
        // Replays and wired links are not Wi-Fi.
        return (e && e->kind == DeviceListModel::Wifi) ? QStringLiteral("wifi-high")
                                                       : QStringLiteral("pulse");
    }
    case Connecting:
        return QStringLiteral("wifi-medium");
    case Off:
        break;
    }
    return QStringLiteral("wifi-slash");
}

QString DeviceManager::dialogTitle() const
{
    if (m_transport == 1) {
        return QStringLiteral("Connect amplifier over Wi-Fi");
    }
    for (int i = 0; i < m_model.count(); ++i) {
        if (m_model.at(i)->kind == DeviceListModel::Wifi) {
            return QStringLiteral("Connect amplifier over Wi-Fi");
        }
    }
    return QStringLiteral("Connect amplifier");
}

QString DeviceManager::statusLine() const
{
    if (m_scanning) {
        return QStringLiteral("Scanning for devices…");
    }
    if (!m_error.isEmpty()) {
        return m_error;
    }
    const int n = m_model.count();
    if (n == 0) {
        return QStringLiteral("No devices nearby");
    }
    return n == 1 ? QStringLiteral("1 device nearby") : QStringLiteral("%1 devices nearby").arg(n);
}

QString DeviceManager::connectLabel() const
{
    if (m_state == Connecting) {
        return QStringLiteral("Connecting…");
    }
    const auto *e = m_model.at(m_selected);
    if (m_state == Connected && e && e->name == m_connectedName) {
        return QStringLiteral("Connected");
    }
    return QStringLiteral("Connect");
}

bool DeviceManager::canConnect() const
{
    if (m_state == Connecting) {
        return false;
    }
    const auto *e = m_model.at(m_selected);
    if (!e) {
        return false;
    }
    return !(m_state == Connected && e->name == m_connectedName);
}

bool DeviceManager::needsPasskey() const
{
    // Shown "when the selected network isn't the connected one" (spec section 8).
    const auto *e = m_model.at(m_selected);
    return e && e->needsPasskey && !(m_state == Connected && e->name == m_connectedName);
}

// ---------------------------------------------------------------- selection

void DeviceManager::setSelectedIndex(int i)
{
    if (i == m_selected || i < -1 || i >= m_model.count()) {
        return;
    }
    m_selected = i;
    m_passkey.clear();   // a passkey typed for one device must not follow to another
    emit selectionChanged();
}

void DeviceManager::setPasskey(const QString &p)
{
    if (p == m_passkey) {
        return;
    }
    m_passkey = p;
    if (!m_error.isEmpty()) {
        m_error.clear();
        emit statusLineChanged();
    }
    emit selectionChanged();
}

void DeviceManager::reselect(const QString &preferName)
{
    int idx = preferName.isEmpty() ? -1 : m_model.indexOfName(preferName);
    if (idx < 0 && !m_connectedName.isEmpty()) {
        idx = m_model.indexOfName(m_connectedName);
    }
    if (idx < 0 && m_model.count() > 0) {
        idx = 0;
    }
    if (idx != m_selected) {
        m_selected = idx;
        m_passkey.clear();
    }
    emit selectionChanged();
}

// ------------------------------------------------------------------ Wi-Fi tab

void DeviceManager::setTransport(int t)
{
    t = (t == 1) ? 1 : 0;
    if (t == m_transport) {
        return;
    }
    m_transport = t;
    QSettings().setValue(QStringLiteral("device/transport"), t);
    emit transportChanged();
    emit statusLineChanged();          // the title depends on it
}

void DeviceManager::setWifiHost(const QString &h)
{
    if (h == m_wifiHost) {
        return;
    }
    m_wifiHost = h;
    setError(QString());
    // Remembered only once it is a usable address: half-typed text is not a "last
    // used address" worth offering on the next run.
    if (wifiaddress::isValidHost(h)) {
        QSettings().setValue(QStringLiteral("wifi/host"), h.trimmed());
    }
    emit wifiChanged();
}

void DeviceManager::setWifiPort(int p)
{
    if (p == m_wifiPort) {
        return;
    }
    m_wifiPort = p;
    setError(QString());
    if (wifiaddress::isValidPort(p)) {
        QSettings().setValue(QStringLiteral("wifi/port"), p);
    }
    emit wifiChanged();
}

QString DeviceManager::wifiProblem() const { return wifiaddress::problem(m_wifiHost, m_wifiPort); }

bool DeviceManager::canConnectWifi() const
{
    return m_state != Connecting && m_previewWifi.isEmpty() && wifiaddress::usable(m_wifiHost, m_wifiPort);
}

QString DeviceManager::wifiPhase() const
{
    return m_previewWifi.isEmpty() ? QStringLiteral("manual") : m_previewWifi;
}

QString DeviceManager::wifiDeviceName() const
{
    const QString p = m_previewWifi;
    return (p == QLatin1String("connecting") || p == QLatin1String("connected") || p == QLatin1String("nodata"))
               ? m_pendingName : QString();
}

QString DeviceManager::wifiDeviceMeta() const
{
    return wifiDeviceName().isEmpty() ? QString()
                                      : QStringLiteral("%1 · port %2").arg(m_wifiHost).arg(m_wifiPort);
}

QString DeviceManager::wifiDeviceStatus() const
{
    const QString p = m_previewWifi;
    if (p == QLatin1String("connecting")) {
        return QStringLiteral("Connecting…");
    }
    if (p == QLatin1String("connected")) {
        return QStringLiteral("Connected");
    }
    if (p == QLatin1String("nodata")) {
        return QStringLiteral("No data");
    }
    return {};
}

QString DeviceManager::wifiStatusLine() const
{
    const QString p = m_previewWifi;
    if (p == QLatin1String("searching")) {
        return QStringLiteral("Searching for devices…");
    }
    if (p == QLatin1String("connecting")) {
        return QStringLiteral("Connecting to %1:%2…").arg(m_wifiHost).arg(m_wifiPort);
    }
    if (p == QLatin1String("connected")) {
        return QStringLiteral("Streaming from %1").arg(m_wifiHost);
    }
    if (p == QLatin1String("nodata")) {
        return QStringLiteral("Connected, but no data is arriving.");
    }
    if (!m_error.isEmpty()) {
        return m_error;
    }
    // The honest default. There is no discovery yet, so the user supplies the address.
    return QStringLiteral("Enter the amplifier's address. This PC must be on the same network.");
}

void DeviceManager::connectWifi()
{
    if (m_state == Connecting) {
        return;
    }
    const QString why = wifiProblem();
    if (!why.isEmpty() || !wifiaddress::usable(m_wifiHost, m_wifiPort)) {
        setError(why.isEmpty() ? tr("Enter the amplifier's address.") : why);
        return;
    }
    // The address is real and is remembered (setWifiHost); the link is not built yet.
    // Say so, instead of showing "Connecting..." for something that cannot happen.
    setError(tr("The Wi-Fi link is not built yet - connect over USB for now."));
}

bool DeviceManager::setPreviewWifi(const QString &phase)
{
    static const QStringList known{QStringLiteral("manual"), QStringLiteral("searching"),
                                   QStringLiteral("connecting"), QStringLiteral("connected"),
                                   QStringLiteral("nodata"), QStringLiteral("error")};
    if (!known.contains(phase)) {
        return false;
    }
    m_transport = 1;                    // a preview of the Wi-Fi view opens on it
    m_previewWifi = phase == QLatin1String("manual") ? QString() : phase;
    m_noData = (phase == QLatin1String("nodata"));
    m_wifiHost = QStringLiteral("192.168.4.37");      // example only, never saved
    m_wifiPort = wifiaddress::kDefaultPort;
    m_error.clear();

    const bool link = phase == QLatin1String("connected") || phase == QLatin1String("nodata");
    m_pendingName = QStringLiteral("FES-nRF7002");
    m_connectedName = link ? m_pendingName : QString();
    m_scanning = (phase == QLatin1String("searching"));
    if (phase == QLatin1String("error")) {
        m_error = tr("Connection refused - nothing is listening on %1:%2.")
                      .arg(m_wifiHost).arg(m_wifiPort);
    }
    m_state = (phase == QLatin1String("connecting")) ? Connecting : (link ? Connected : Off);

    emit transportChanged();
    emit stateChanged();
    emit selectionChanged();
    emit scanningChanged();
    emit statusLineChanged();
    emit wifiChanged();
    return true;
}

// ------------------------------------------------------------------ scanning

void DeviceManager::enumerate()
{
    const QString keep = m_model.at(m_selected) ? m_model.at(m_selected)->name : QString();
    std::vector<DeviceListModel::Entry> entries;

    if (m_preview) {
        // Layout review only - see setPreviewMode().
        auto net = [](const char *name, const char *meta, int level) {
            DeviceListModel::Entry e;
            e.name = QString::fromUtf8(name);
            e.meta = QString::fromUtf8(meta);
            e.kind = DeviceListModel::Wifi;
            e.level = level;
            e.needsPasskey = true;
            return e;
        };
        entries.push_back(net("BioAmp-8CH-3F2A", "Amplifier · WPA2 · 5 GHz", 3));
        entries.push_back(net("BioAmp-8CH-71C0", "Amplifier · WPA2 · 2.4 GHz", 2));
        entries.push_back(net("Ward4-Telemetry", "Hospital network · WPA2-Enterprise", 1));
    } else {
        std::vector<DeviceListModel::Entry> all, usb;
        for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
            DeviceListModel::Entry e;
            e.name = info.portName();
            e.port = info.portName();
            e.kind = DeviceListModel::Serial;

            const QString desc = info.description().isEmpty() ? QStringLiteral("Serial port")
                                                              : info.description();
            e.meta = desc + QStringLiteral(" · %1 baud").arg(kBaud);

            // QSerialPortInfo::isBusy() is gone in Qt 6, and the only other way to
            // learn that another program holds a port is to open it. So "busy" is
            // not guessed here: a port that cannot be opened reports the real
            // reason (permission denied, resource busy) when connecting.
            all.push_back(e);
            if (info.hasVendorIdentifier()) {
                usb.push_back(e);
            }
        }
        // The amplifier arrives through a USB bridge, so ports with a USB vendor
        // ID are the candidates. The built-in ttyS0..ttyS31 legacy ports are
        // noise on a dozen machines - but if nothing has a vendor ID (a virtual
        // port, an unusual adapter) show everything rather than an empty list.
        entries = usb.empty() ? std::move(all) : std::move(usb);
        std::sort(entries.begin(), entries.end(),
                  [](const auto &a, const auto &b) { return a.name < b.name; });
    }

    m_model.setEntries(std::move(entries));
    reselect(keep);
    emit statusLineChanged();
}

void DeviceManager::scan()
{
    if (m_scanning || !m_previewWifi.isEmpty()) {   // a --preview-wifi state is held as shown
        return;
    }
    m_scanning = true;
    m_error.clear();
    emit scanningChanged();
    emit statusLineChanged();

    // The enumeration itself takes a few milliseconds. The state is held for a
    // moment so a press of Rescan is visibly acknowledged rather than flickering.
    QTimer::singleShot(0, this, [this] {
        enumerate();
        QTimer::singleShot(400, this, [this] {
            m_scanning = false;
            emit scanningChanged();
            emit statusLineChanged();
        });
    });
}

void DeviceManager::setWatching(bool on)
{
    if (on == m_watching) {
        return;
    }
    m_watching = on;
    if (on) {
        m_rescanTimer.start();
    } else {
        m_rescanTimer.stop();
    }
    emit watchingChanged();
}

void DeviceManager::setPreviewMode(bool on)
{
    m_preview = on;
    enumerate();
}

// -------------------------------------------------------------- connecting

void DeviceManager::setState(State s)
{
    if (m_state == s) {
        return;
    }
    m_state = s;
    emit stateChanged();
    emit selectionChanged();
    emit statusLineChanged();
}

void DeviceManager::setError(const QString &msg)
{
    m_error = msg;
    emit statusLineChanged();
}

void DeviceManager::connectSelected()
{
    const auto *e = m_model.at(m_selected);
    if (!e || m_state == Connecting) {
        return;
    }
    if (m_state == Connected && e->name == m_connectedName) {
        return;
    }
    if (e->needsPasskey && m_passkey.isEmpty()) {
        setError(tr("Enter the passkey printed on the amplifier label."));
        return;
    }
    if (e->busy) {
        setError(tr("%1 is in use by another program.").arg(e->name));
        return;
    }
    if (m_preview || e->kind == DeviceListModel::Wifi) {
        setError(tr("Wi-Fi is not available yet - the amplifier streams over its serial link."));
        return;
    }

    setError(QString());
    const QString name = e->name;
    const QString port = e->port;

    if (m_state == Connected) {
        // Switching device: drop the current one first, without it reading as
        // an unexpected loss.
        m_connectedName.clear();
        m_model.setConnectedName(QString());
        setState(Off);
        m_acq->disconnectSource();
    }

    m_pendingName = name;
    m_model.setConnectingName(name);
    setState(Connecting);
    m_connectTimeout.start();
    m_acq->connectSerial(port, kBaud);
}

void DeviceManager::disconnectDevice()
{
    m_connectTimeout.stop();
    m_connectedName.clear();
    m_pendingName.clear();
    m_model.setConnectedName(QString());
    m_model.setConnectingName(QString());

    if (!m_previewWifi.isEmpty()) {              // leaving a --preview-wifi state
        m_previewWifi.clear();
        m_noData = false;
        emit wifiChanged();
    }

    // State first: tearing the source down fires runningChanged(false), and with
    // the state still Connected that would be reported as a lost connection.
    setState(Off);
    m_acq->disconnectSource();
}

void DeviceManager::openReplayFile(const QString &path)
{
    m_pendingName = QFileInfo(path).fileName();
    m_isReplay = true;
    m_acq->openReplay(QUrl::fromLocalFile(path));
}

void DeviceManager::onRunningChanged()
{
    if (m_acq->isRunning()) {
        m_connectTimeout.stop();
        m_connectedName = m_pendingName.isEmpty() ? tr("Device") : m_pendingName;
        m_model.setConnectingName(QString());
        m_model.setConnectedName(m_connectedName);
        setState(Connected);
        reselect(m_connectedName);
        return;
    }

    // Stopped. If we asked for it, disconnectDevice() already moved to Off.
    if (m_state == Connected) {
        const bool replay = m_isReplay;
        m_connectedName.clear();
        m_model.setConnectedName(QString());
        setError(replay ? tr("Replay finished.") : tr("Connection lost."));
        m_isReplay = false;
        setState(Off);
    }
}

void DeviceManager::onSourceError(const QString &message)
{
    // Only a failed connection attempt is this class's business. A recorder
    // error while streaming must not drop the device.
    if (m_state != Connecting) {
        return;
    }
    m_connectTimeout.stop();
    m_model.setConnectingName(QString());
    setError(message);
    setState(Off);
}
