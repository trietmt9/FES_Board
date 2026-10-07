#pragma once

// Is what the user typed into the Wi-Fi dialog an address the viewer could connect to?
//
// Pure functions, no Qt Quick, so the rules are tested on their own. They accept what
// WIFI_DESIGN.md section 4 offers as the manual fallback: an IPv4 address, an IPv6
// address, or a host name such as `fes-nrf7002.local`.
//
// The traps this exists to avoid:
//   - QHostAddress accepts "192.168.4" as IPv4 shorthand (192.168.0.4), which is never
//     what someone typing an address means. Four dotted octets are required.
//   - A string of digits and dots that is not a valid IPv4 address ("192.168.4.300",
//     "1.2.3") is also a syntactically valid host NAME, so it would pass a name check
//     and then fail much later as "host not found". Anything numeric is judged as an
//     address only.

#include <QHostAddress>
#include <QRegularExpression>
#include <QString>

namespace wifiaddress {

constexpr int kDefaultPort = 5000;

inline bool isValidPort(int port) { return port >= 1 && port <= 65535; }

inline bool isValidHost(const QString &raw)
{
    const QString host = raw.trimmed();
    if (host.isEmpty() || host.size() > 253) {
        return false;
    }

    if (host.contains(QLatin1Char(':'))) {                    // IPv6
        QHostAddress a;
        return a.setAddress(host) && a.protocol() == QAbstractSocket::IPv6Protocol;
    }

    static const QRegularExpression numeric(QStringLiteral("^[0-9.]+$"));
    if (numeric.match(host).hasMatch()) {                     // must be a real IPv4
        static const QRegularExpression quad(
            QStringLiteral("^(0|[1-9][0-9]{0,2})(\\.(0|[1-9][0-9]{0,2})){3}$"));
        if (!quad.match(host).hasMatch()) {
            return false;
        }
        QHostAddress a;
        return a.setAddress(host) && a.protocol() == QAbstractSocket::IPv4Protocol;
    }

    // Host name: dot-separated labels of letters, digits and inner hyphens.
    static const QRegularExpression name(
        QStringLiteral("^([A-Za-z0-9]([A-Za-z0-9-]{0,61}[A-Za-z0-9])?)"
                       "(\\.[A-Za-z0-9]([A-Za-z0-9-]{0,61}[A-Za-z0-9])?)*$"));
    return name.match(host).hasMatch();
}

/// What is wrong with the pair, in words for the dialog; empty when it is usable OR
/// when there is nothing to judge yet (an empty address is "not ready", not "wrong").
inline QString problem(const QString &host, int port)
{
    if (!isValidPort(port)) {
        return QStringLiteral("Port must be between 1 and 65535.");
    }
    if (!host.trimmed().isEmpty() && !isValidHost(host)) {
        return QStringLiteral("That is not a valid address or host name.");
    }
    return {};
}

inline bool usable(const QString &host, int port) { return isValidHost(host) && isValidPort(port); }

} // namespace wifiaddress
