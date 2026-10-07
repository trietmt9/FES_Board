#include "Recorder.h"

Recorder::Recorder(QObject *parent) : QObject(parent)
{
    m_parser.setDataHandler([this](const emg::DataFrame &f) { handleData(f); });
    m_parser.setInfoHandler([this](const emg::InfoFrame &i) { handleInfo(i); });
    // Log text is preserved in .emgraw verbatim; it has no column in the CSV.
}

Recorder::~Recorder()
{
    closeFiles();
}

void Recorder::startRecording(const QString &basePath, bool raw, bool csv)
{
    if (m_active) {
        stopRecording();
    }
    if (!raw && !csv) {
        emit errorOccurred(tr("Select at least one recording format."));
        return;
    }

    m_parser.reset();
    m_bytes = 0;
    m_samples = 0;
    m_sampleIndex = 0;
    m_csvHeaderWritten = false;

    QString rawPath;
    QString csvPath;

    if (raw) {
        rawPath = basePath + QStringLiteral(".emgraw");
        m_rawFile.setFileName(rawPath);
        if (!m_rawFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            emit errorOccurred(tr("Cannot write %1: %2").arg(rawPath, m_rawFile.errorString()));
            return;
        }
    }

    if (csv) {
        csvPath = basePath + QStringLiteral(".csv");
        m_csvFile.setFileName(csvPath);
        if (!m_csvFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            emit errorOccurred(tr("Cannot write %1: %2").arg(csvPath, m_csvFile.errorString()));
            m_rawFile.close();
            return;
        }
        m_csv.setDevice(&m_csvFile);
    }

    m_active = true;
    emit recordingStarted(rawPath, csvPath);
}

void Recorder::stopRecording()
{
    if (!m_active) {
        return;
    }
    m_active = false;
    closeFiles();
    emit recordingStopped(m_bytes, m_samples);
}

void Recorder::closeFiles()
{
    if (m_csvFile.isOpen()) {
        m_csv.flush();
        m_csv.setDevice(nullptr);
        m_csvFile.close();
    }
    if (m_rawFile.isOpen()) {
        m_rawFile.flush();
        m_rawFile.close();
    }
}

void Recorder::write(const QByteArray &bytes)
{
    if (!m_active || bytes.isEmpty()) {
        return;
    }

    if (m_rawFile.isOpen()) {
        const qint64 n = m_rawFile.write(bytes);
        if (n < 0) {
            emit errorOccurred(tr("Write failed: %1").arg(m_rawFile.errorString()));
            stopRecording();
            return;
        }
        m_bytes += n;
    } else {
        m_bytes += bytes.size();
    }

    // Only decode when there is a CSV to fill.
    if (m_csvFile.isOpen()) {
        m_parser.feed(bytes);
    }

    emit progress(m_bytes, m_samples);
}

void Recorder::handleInfo(const emg::InfoFrame &info)
{
    if (!info.isValid()) {
        return;
    }
    m_uvPerCode = info.uvPerCode();
    m_sampleRate = info.sampleRateHz;
}

void Recorder::handleData(const emg::DataFrame &frame)
{
    if (!m_csvFile.isOpen()) {
        return;
    }

    if (!m_csvHeaderWritten) {
        m_csv << "t_s";
        for (int c = 0; c < frame.nCh; ++c) {
            m_csv << ",ch" << (c + 1) << "_uv";
        }
        m_csv << '\n';
        m_csvHeaderWritten = true;
    }

    const double dt = m_sampleRate > 0 ? 1.0 / m_sampleRate : 0.001;

    for (int s = 0; s < frame.nSamp; ++s) {
        m_csv << QString::number(m_sampleIndex * dt, 'f', 6);
        for (int c = 0; c < frame.nCh; ++c) {
            m_csv << ',' << QString::number(frame.code(s, c) * m_uvPerCode, 'f', 3);
        }
        m_csv << '\n';
        ++m_sampleIndex;
    }

    m_samples += frame.nSamp;
}
