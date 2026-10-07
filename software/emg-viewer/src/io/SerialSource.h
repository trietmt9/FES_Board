#pragma once

// Live acquisition from the board over a serial link.
//
// Lives on its own QThread. On readyRead it parses, converts to microvolts and
// writes into the SampleRing - all without touching QML or any GUI object.
//
// Works unchanged for the planned USB CDC ACM transport: the frame format does
// not change and the host still sees a /dev/ttyACM* character device.

#include "io/ISampleSource.h"

#include <QElapsedTimer>
#include <QTimer>

#include <vector>

QT_BEGIN_NAMESPACE
class QSerialPort;
QT_END_NAMESPACE

class SerialSource : public ISampleSource {
    Q_OBJECT

public:
    explicit SerialSource(QObject *parent = nullptr);
    ~SerialSource() override;

    // Call before start(); both are read on the worker thread at start().
    void setPortName(const QString &name) { m_portName = name; }
    void setBaudRate(int baud) { m_baudRate = baud; }

    void setForwardRaw(bool on) { m_forwardRaw.storeRelaxed(on ? 1 : 0); }

public slots:
    void start() override;
    void stop() override;

protected slots:
    void publishStats();

protected:
    /// Reset the parser, loss tracker and rings and start the 1 s stats timer.
    /// Shared with TcpClientSource, which only swaps the transport.
    void beginStream();
    void handleData(const emg::DataFrame &frame);
    void handleInfo(const emg::InfoFrame &info);

    QTimer *m_statsTimer = nullptr;
    QElapsedTimer m_uptime;

    emg::FrameParser m_parser;
    emg::InfoFrame m_info;
    bool m_haveInfo = false;

    // Scale factor from the INFO frame. Until one arrives we fall back to the
    // firmware's compiled-in constants so a capture is never silently unscaled;
    // ARCHITECTURE.md 2.4 explains why we prefer the wire value.
    double m_uvPerCode = 2400000.0 / 6.0 / 8388608.0;

    QAtomicInt m_forwardRaw{0};
    quint64 m_overflows = 0;
    std::vector<float> m_scratch;   // raw microvolts
    std::vector<float> m_filtered;  // band-passed microvolts

private slots:
    void onReadyRead();
    void onErrorOccurred();

private:
    QSerialPort *m_port = nullptr;
    QString m_portName;
    int m_baudRate = 921600;
};
