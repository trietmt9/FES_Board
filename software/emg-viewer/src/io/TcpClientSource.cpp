#include "TcpClientSource.h"

#include <QTcpSocket>

void TcpClientSource::start()
{
    if (m_sock) {
        return;
    }

    // Created here so the socket belongs to the worker thread that services it.
    m_sock = new QTcpSocket(this);
    m_sock->setSocketOption(QAbstractSocket::LowDelayOption, 1);   // no Nagle

    connect(m_sock, &QTcpSocket::connected, this, [this] {
        m_up = true;
        beginStream();
        emit started();
    });
    connect(m_sock, &QTcpSocket::readyRead, this, &TcpClientSource::onData);
    connect(m_sock, &QTcpSocket::errorOccurred, this, &TcpClientSource::onError);
    connect(m_sock, &QTcpSocket::disconnected, this, &TcpClientSource::stop);

    m_sock->connectToHost(m_host, m_tcpPort);
}

void TcpClientSource::stop()
{
    if (m_statsTimer) {
        m_statsTimer->stop();
    }
    if (!m_sock) {
        return;
    }

    const bool wasUp = m_up;
    m_up = false;
    m_sock->disconnect(this);          // closing must not re-enter stop()
    if (wasUp) {
        m_parser.flushText();
        publishStats();
    }
    m_sock->abort();
    m_sock->deleteLater();
    m_sock = nullptr;

    if (wasUp) {
        emit stopped();
    }
}

void TcpClientSource::onData()
{
    const QByteArray chunk = m_sock->readAll();
    if (chunk.isEmpty()) {
        return;
    }
    // Verbatim before parsing, as SerialSource does, so recordings stay byte-exact.
    if (m_forwardRaw.loadRelaxed()) {
        emit rawBytes(chunk);
    }
    m_parser.feed(chunk);
}

void TcpClientSource::onError()
{
    // The peer closing is reported by disconnected(); only say why a connect failed.
    if (m_sock && m_sock->error() != QAbstractSocket::RemoteHostClosedError) {
        emit errorOccurred(tr("Cannot connect to %1:%2 - %3")
                               .arg(m_host).arg(m_tcpPort).arg(m_sock->errorString()));
    }
}
