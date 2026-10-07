#include "SerialSource.h"

#include <QSerialPort>

SerialSource::SerialSource(QObject *parent) : ISampleSource(parent)
{
    m_parser.setDataHandler([this](const emg::DataFrame &f) { handleData(f); });
    m_parser.setInfoHandler([this](const emg::InfoFrame &i) { handleInfo(i); });
    m_parser.setTextHandler([this](const QString &s) { emit textLine(s); });
}

SerialSource::~SerialSource() = default;

void SerialSource::start()
{
    if (m_port) {
        return;
    }

    // Constructed here, not in the constructor, so the QSerialPort belongs to
    // the worker thread that will service its readyRead.
    m_port = new QSerialPort(this);
    m_port->setPortName(m_portName);
    m_port->setBaudRate(m_baudRate);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_port->open(QIODevice::ReadWrite)) {
        const QString why = m_port->errorString();
        delete m_port;
        m_port = nullptr;
        emit errorOccurred(tr("Cannot open %1: %2").arg(m_portName, why));
        return;
    }

    // Drop anything that arrived before we attached, so the parser starts on a
    // clean boundary instead of mid-frame.
    m_port->clear(QSerialPort::AllDirections);

    connect(m_port, &QSerialPort::readyRead, this, &SerialSource::onReadyRead);
    connect(m_port, &QSerialPort::errorOccurred, this, &SerialSource::onErrorOccurred);

    beginStream();

    emit started();
}

void SerialSource::beginStream()
{
    m_parser.reset();
    m_haveInfo = false;
    m_overflows = 0;
    m_loss.reset();
    m_haveMask = false;
    if (m_ring) {
        m_ring->clear();
    }
    if (m_filteredRing) {
        m_filteredRing->clear();
    }
    reconfigureFilters();
    m_uptime.start();

    if (!m_statsTimer) {
        m_statsTimer = new QTimer(this);
        m_statsTimer->setInterval(1000);
        connect(m_statsTimer, &QTimer::timeout, this, &SerialSource::publishStats);
    }
    m_statsTimer->start();
}

void SerialSource::stop()
{
    if (m_statsTimer) {
        m_statsTimer->stop();
    }
    if (!m_port) {
        return;
    }

    m_parser.flushText(); // do not swallow a final unterminated log line
    publishStats();

    m_port->close();
    m_port->deleteLater();
    m_port = nullptr;

    emit stopped();
}

void SerialSource::onReadyRead()
{
    const QByteArray chunk = m_port->readAll();
    if (chunk.isEmpty()) {
        return;
    }

    // Forward verbatim before parsing, so .emgraw is a byte-exact copy of the
    // link - including log text and anything the parser rejects.
    if (m_forwardRaw.loadRelaxed()) {
        emit rawBytes(chunk);
    }

    m_parser.feed(chunk);
}

void SerialSource::onErrorOccurred()
{
    if (!m_port) {
        return;
    }
    const QSerialPort::SerialPortError e = m_port->error();
    if (e == QSerialPort::NoError) {
        return;
    }

    emit errorOccurred(m_port->errorString());

    // A resource error means the device vanished (cable pulled, board reset).
    if (e == QSerialPort::ResourceError || e == QSerialPort::PermissionError) {
        stop();
    }
}

void SerialSource::handleInfo(const emg::InfoFrame &info)
{
    const bool changed = !m_haveInfo || info.sampleRateHz != m_info.sampleRateHz ||
                         info.gain != m_info.gain || info.vrefUv != m_info.vrefUv ||
                         info.nChActive != m_info.nChActive;

    m_info = info;
    m_haveInfo = true;

    if (info.isValid()) {
        m_uvPerCode = info.uvPerCode();

        // Filter corners are rate-dependent, so the coefficients are only
        // correct once the device has told us its actual rate.
        if (info.sampleRateHz != m_filterRate) {
            m_filterRate = info.sampleRateHz;
            // The loss tracker converts elapsed t_ms into an expected sample
            // count, so a rate change invalidates everything measured so far.
            m_loss.reset();
            reconfigureFilters();
        }
    }

    // INFO repeats once a second; only bother the GUI when something moved.
    if (changed) {
        emit infoReceived(info);
    }
}

void SerialSource::handleData(const emg::DataFrame &frame)
{
    if (!m_ring) {
        return;
    }
    if (frame.overflow()) {
        ++m_overflows;
    }
    noteDataFrame(frame);

    const std::size_t count = frame.codes.size();
    if (m_scratch.size() < count) {
        m_scratch.resize(count);
    }

    for (std::size_t i = 0; i < count; ++i) {
        m_scratch[i] = static_cast<float>(frame.codes[i] * m_uvPerCode);
    }

    // codes are already sample-major, which is what SampleRing::write expects.
    m_ring->write(m_scratch.data(), frame.nCh, frame.nSamp);

    // Filtering runs here, on the stream, so the IIR state is continuous. Doing
    // it per-repaint over a display window would restart the transient every
    // frame and break the envelope.
    if (m_filteredRing) {
        if (m_filtered.size() < count) {
            m_filtered.resize(count);
        }
        filterBlock(m_scratch.data(), m_filtered.data(), frame.nCh, frame.nSamp);
        m_filteredRing->write(m_filtered.data(), frame.nCh, frame.nSamp);
    }
}

void SerialSource::publishStats()
{
    emit statsUpdated(m_parser.stats(), m_overflows, m_loss.lost());
}
