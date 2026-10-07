#pragma once

// The same framed stream as SerialSource, read from a TCP socket (the board's
// nRF7002 listens, this PC connects - WIFI_DESIGN.md mode A). Parsing, filtering
// and statistics are inherited; only the transport differs.

#include "io/SerialSource.h"

QT_BEGIN_NAMESPACE
class QTcpSocket;
QT_END_NAMESPACE

class TcpClientSource : public SerialSource {
    Q_OBJECT

public:
    explicit TcpClientSource(QObject *parent = nullptr) : SerialSource(parent) {}

    // Call before start().
    void setTarget(const QString &host, quint16 port)
    {
        m_host = host;
        m_tcpPort = port;
    }

public slots:
    /// Asynchronous: started() is emitted once the connection is up, not before.
    void start() override;
    void stop() override;

private:
    void onData();
    void onError();

    QTcpSocket *m_sock = nullptr;
    bool m_up = false;   // connected() seen; the socket state is already Unconnected inside disconnected()
    QString m_host;
    quint16 m_tcpPort = 5000;
};
