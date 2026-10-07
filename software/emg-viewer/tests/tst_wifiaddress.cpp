// The Wi-Fi tab's logic: which addresses it accepts, what it remembers, and that
// Connect does not pretend to connect.
//
// The tab exists before the Wi-Fi link does (WIFI_DESIGN.md), so the part that IS real
// is tested hard: validation and persistence. The part that is not - connecting - is
// tested for honesty: it must not report Connecting or Connected for something that
// cannot happen.

#include <QCoreApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QTest>

#include "model/Acquisition.h"
#include "model/DeviceManager.h"
#include "model/WifiAddress.h"

using namespace wifiaddress;

class TstWifiAddress : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void init() { QSettings().clear(); }

    // ---- validation: pure
    void acceptsDottedQuads_data();
    void acceptsDottedQuads();
    void rejectsNumericStringsThatAreNotAddresses_data();
    void rejectsNumericStringsThatAreNotAddresses();
    void acceptsHostNames();
    void rejectsMalformedHostNames_data();
    void rejectsMalformedHostNames();
    void acceptsIpv6();
    void portRange();
    void problemTextSaysNothingUntilThereIsSomethingToJudge();

    // ---- the dialog's model
    void defaultsAreEmptyAddressAndPort5000();
    void aUsableAddressIsRememberedAcrossInstances();
    void halfTypedTextIsNotRemembered();
    void connectingNeverClaimsToConnect();
    void connectRefusesAnInvalidAddressWithTheReason();
    void transportIsRememberedAndRetitlesTheDialog();
    void everyPreviewPhaseIsDistinctOnTheHeaderButton();
    void unknownPreviewPhaseIsRejected();
    void leavingAPreviewRestoresTheOffButton();
};

void TstWifiAddress::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("FES_Board_Test"));
    QCoreApplication::setApplicationName(QStringLiteral("wifiaddress-test"));
    QStandardPaths::setTestModeEnabled(true);
    QSettings().clear();
    qRegisterMetaType<emg::InfoFrame>("emg::InfoFrame");
    qRegisterMetaType<emg::ParserStats>("emg::ParserStats");
    qRegisterMetaType<emg::FilterConfig>("emg::FilterConfig");
}

// ---------------------------------------------------------------- validation

void TstWifiAddress::acceptsDottedQuads_data()
{
    QTest::addColumn<QString>("host");
    QTest::newRow("typical") << "192.168.4.37";
    QTest::newRow("zeros") << "0.0.0.0";
    QTest::newRow("max") << "255.255.255.255";
    QTest::newRow("loopback") << "127.0.0.1";
    QTest::newRow("padded with spaces") << "  10.0.0.5  ";
}

void TstWifiAddress::acceptsDottedQuads()
{
    QFETCH(QString, host);
    QVERIFY(isValidHost(host));
}

// "192.168.4.300" and "1.2.3" are also well-formed host NAMES, so a name check alone
// would wave them through and they would fail minutes later as "host not found".
// QHostAddress also reads "1.2.3" as the shorthand 1.2.0.3, which nobody means.
void TstWifiAddress::rejectsNumericStringsThatAreNotAddresses_data()
{
    QTest::addColumn<QString>("host");
    QTest::newRow("octet over 255") << "192.168.4.300";
    QTest::newRow("three octets") << "1.2.3";
    QTest::newRow("shorthand") << "192.168.4";
    QTest::newRow("five octets") << "1.2.3.4.5";
    QTest::newRow("leading zero octet") << "192.168.04.37";
    QTest::newRow("a bare number") << "12345";
    QTest::newRow("trailing dot") << "192.168.4.37.";
    QTest::newRow("empty octet") << "192..4.37";
}

void TstWifiAddress::rejectsNumericStringsThatAreNotAddresses()
{
    QFETCH(QString, host);
    QVERIFY2(!isValidHost(host), qPrintable(host));
}

void TstWifiAddress::acceptsHostNames()
{
    QVERIFY(isValidHost(QStringLiteral("fes-nrf7002.local")));
    QVERIFY(isValidHost(QStringLiteral("nrf7002")));
    QVERIFY(isValidHost(QStringLiteral("Lab-Amp-3.example.org")));
    QVERIFY(isValidHost(QStringLiteral("a")));
}

void TstWifiAddress::rejectsMalformedHostNames_data()
{
    QTest::addColumn<QString>("host");
    QTest::newRow("empty") << "";
    QTest::newRow("spaces only") << "   ";
    QTest::newRow("inner space") << "fes nrf";
    QTest::newRow("leading hyphen") << "-fes.local";
    QTest::newRow("trailing hyphen") << "fes-.local";
    QTest::newRow("underscore") << "fes_nrf.local";
    QTest::newRow("empty label") << "fes..local";
    QTest::newRow("leading dot") << ".local";
    QTest::newRow("a url") << "http://192.168.4.37";
    QTest::newRow("with a path") << "fes.local/stream";
    QTest::newRow("label over 63") << QString(64, QLatin1Char('a')) + ".local";
    QTest::newRow("name over 253") << (QString(60, QLatin1Char('a')) + ".").repeated(5);
}

void TstWifiAddress::rejectsMalformedHostNames()
{
    QFETCH(QString, host);
    QVERIFY2(!isValidHost(host), qPrintable(host));
}

void TstWifiAddress::acceptsIpv6()
{
    QVERIFY(isValidHost(QStringLiteral("fe80::1")));
    QVERIFY(isValidHost(QStringLiteral("2001:db8::7334")));
    QVERIFY(!isValidHost(QStringLiteral("fe80:::1")));
    QVERIFY(!isValidHost(QStringLiteral("not:an:address")));
}

void TstWifiAddress::portRange()
{
    QVERIFY(isValidPort(1));
    QVERIFY(isValidPort(5000));
    QVERIFY(isValidPort(65535));
    QVERIFY(!isValidPort(0));
    QVERIFY(!isValidPort(-1));
    QVERIFY(!isValidPort(65536));
}

// An empty address is "not ready", not "wrong": scolding someone for not having typed
// yet is noise, and it would put an error on screen the moment the tab opens.
void TstWifiAddress::problemTextSaysNothingUntilThereIsSomethingToJudge()
{
    QVERIFY(problem(QString(), 5000).isEmpty());
    QVERIFY(problem(QStringLiteral("   "), 5000).isEmpty());
    QVERIFY(problem(QStringLiteral("192.168.4.37"), 5000).isEmpty());
    QVERIFY(!problem(QStringLiteral("192.168.4.300"), 5000).isEmpty());
    QVERIFY(!problem(QStringLiteral("192.168.4.37"), 0).isEmpty());
    QVERIFY(!usable(QString(), 5000));
    QVERIFY(usable(QStringLiteral("fes.local"), 5000));
}

// ----------------------------------------------------------------- the model

void TstWifiAddress::defaultsAreEmptyAddressAndPort5000()
{
    Acquisition acq(nullptr);
    DeviceManager dev(&acq);
    QCOMPARE(dev.wifiHost(), QString());
    QCOMPARE(dev.wifiPort(), 5000);
    QCOMPARE(dev.transport(), 0);
    QVERIFY(!dev.canConnectWifi());          // nothing typed: nothing to connect to
    QVERIFY(dev.wifiProblem().isEmpty());    // ...and nothing to complain about either
    QCOMPARE(dev.wifiPhase(), QStringLiteral("manual"));
}

void TstWifiAddress::aUsableAddressIsRememberedAcrossInstances()
{
    {
        Acquisition acq(nullptr);
        DeviceManager dev(&acq);
        dev.setWifiHost(QStringLiteral("192.168.4.37"));
        dev.setWifiPort(5001);
        QVERIFY(dev.canConnectWifi());
    }
    Acquisition acq(nullptr);
    DeviceManager again(&acq);
    QCOMPARE(again.wifiHost(), QStringLiteral("192.168.4.37"));
    QCOMPARE(again.wifiPort(), 5001);
}

void TstWifiAddress::halfTypedTextIsNotRemembered()
{
    {
        Acquisition acq(nullptr);
        DeviceManager dev(&acq);
        dev.setWifiHost(QStringLiteral("10.0.0.5"));
        dev.setWifiHost(QStringLiteral("10.0.0."));          // mid-edit
        dev.setWifiPort(0);                                   // field cleared
        QVERIFY(!dev.canConnectWifi());
        QVERIFY(!dev.wifiProblem().isEmpty());
    }
    // The last USABLE values survive, not the fragments.
    Acquisition acq(nullptr);
    DeviceManager again(&acq);
    QCOMPARE(again.wifiHost(), QStringLiteral("10.0.0.5"));
    QCOMPARE(again.wifiPort(), 5000);
}

// The link does not exist yet. Connect must say so; showing "Connecting..." would be a
// lie that leaves the user watching a spinner for something that cannot happen.
void TstWifiAddress::connectingNeverClaimsToConnect()
{
    Acquisition acq(nullptr);
    DeviceManager dev(&acq);
    dev.setWifiHost(QStringLiteral("192.168.4.37"));
    dev.connectWifi();

    QCOMPARE(dev.state(), DeviceManager::Off);
    QCOMPARE(dev.label(), QStringLiteral("Connect device"));
    QCOMPARE(dev.iconName(), QStringLiteral("wifi-slash"));
    QVERIFY(!dev.noData());
    QVERIFY2(dev.wifiStatusLine().contains(QStringLiteral("not built yet")),
             qPrintable(dev.wifiStatusLine()));
}

void TstWifiAddress::connectRefusesAnInvalidAddressWithTheReason()
{
    Acquisition acq(nullptr);
    DeviceManager dev(&acq);

    dev.connectWifi();                                        // nothing typed
    QVERIFY2(dev.wifiStatusLine().contains(QStringLiteral("address")), qPrintable(dev.wifiStatusLine()));

    dev.setWifiHost(QStringLiteral("192.168.4.300"));
    dev.connectWifi();
    QVERIFY2(dev.wifiStatusLine().contains(QStringLiteral("not a valid")), qPrintable(dev.wifiStatusLine()));
    QCOMPARE(dev.state(), DeviceManager::Off);

    // Editing clears the stale complaint rather than leaving it under a fixed value.
    dev.setWifiHost(QStringLiteral("192.168.4.30"));
    QVERIFY2(!dev.wifiStatusLine().contains(QStringLiteral("not a valid")), qPrintable(dev.wifiStatusLine()));
}

void TstWifiAddress::transportIsRememberedAndRetitlesTheDialog()
{
    {
        Acquisition acq(nullptr);
        DeviceManager dev(&acq);
        QCOMPARE(dev.dialogTitle(), QStringLiteral("Connect amplifier"));
        dev.setTransport(1);
        QCOMPARE(dev.dialogTitle(), QStringLiteral("Connect amplifier over Wi-Fi"));
        dev.setTransport(7);                                  // anything else means USB
        QCOMPARE(dev.transport(), 0);
        dev.setTransport(1);
    }
    Acquisition acq(nullptr);
    DeviceManager again(&acq);
    QCOMPARE(again.transport(), 1);
}

// ------------------------------------------------------------------ previews

// Each design state has to be recognisable on the header button alone, since the
// spec's palette has no warning colour to tell "no data" from "connected" by.
void TstWifiAddress::everyPreviewPhaseIsDistinctOnTheHeaderButton()
{
    struct Row { const char *phase; const char *label; const char *icon; DeviceManager::State state; bool noData; };
    const Row rows[] = {
        {"manual", "Connect device", "wifi-slash", DeviceManager::Off, false},
        {"searching", "Connect device", "wifi-slash", DeviceManager::Off, false},
        {"connecting", "Connecting…", "wifi-medium", DeviceManager::Connecting, false},
        {"connected", "FES-nRF7002", "wifi-high", DeviceManager::Connected, false},
        {"nodata", "FES-nRF7002 · no data", "wifi-medium", DeviceManager::Connected, true},
        {"error", "Connect device", "wifi-slash", DeviceManager::Off, false},
    };
    for (const Row &r : rows) {
        Acquisition acq(nullptr);
        DeviceManager dev(&acq);
        QVERIFY2(dev.setPreviewWifi(QString::fromLatin1(r.phase)), r.phase);
        QCOMPARE(dev.label(), QString::fromUtf8(r.label));
        QCOMPARE(dev.iconName(), QString::fromLatin1(r.icon));
        QCOMPARE(dev.state(), r.state);
        QCOMPARE(dev.noData(), r.noData);
    }

    // "no data" must differ from "connected" in BOTH label and icon.
    Acquisition a1(nullptr), a2(nullptr);
    DeviceManager ok(&a1), silent(&a2);
    ok.setPreviewWifi(QStringLiteral("connected"));
    silent.setPreviewWifi(QStringLiteral("nodata"));
    QVERIFY(ok.label() != silent.label());
    QVERIFY(ok.iconName() != silent.iconName());

    // The error preview carries the real-looking cause (WIFI_DESIGN.md section 5).
    Acquisition a3(nullptr);
    DeviceManager bad(&a3);
    bad.setPreviewWifi(QStringLiteral("error"));
    QVERIFY(bad.wifiStatusLine().contains(QStringLiteral("refused")));
}

void TstWifiAddress::unknownPreviewPhaseIsRejected()
{
    Acquisition acq(nullptr);
    DeviceManager dev(&acq);
    QVERIFY(!dev.setPreviewWifi(QStringLiteral("bogus")));
    QCOMPARE(dev.state(), DeviceManager::Off);
    QCOMPARE(dev.wifiPhase(), QStringLiteral("manual"));
}

// A preview must not leave a ghost behind: example data is never saved, and
// Disconnect returns to the real, empty state.
void TstWifiAddress::leavingAPreviewRestoresTheOffButton()
{
    Acquisition acq(nullptr);
    DeviceManager dev(&acq);
    dev.setPreviewWifi(QStringLiteral("nodata"));
    QVERIFY(dev.canDisconnect());
    dev.disconnectDevice();
    QCOMPARE(dev.state(), DeviceManager::Off);
    QCOMPARE(dev.label(), QStringLiteral("Connect device"));
    QVERIFY(!dev.noData());
    QCOMPARE(dev.wifiPhase(), QStringLiteral("manual"));

    // And the example address 192.168.4.37 was never written to the settings.
    QVERIFY(QSettings().value(QStringLiteral("wifi/host")).toString().isEmpty());
}

QTEST_MAIN(TstWifiAddress)
#include "tst_wifiaddress.moc"
