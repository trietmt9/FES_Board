#pragma once

// Session recording, on its own thread so a slow filesystem can never stall
// acquisition.
//
// Two formats, independently selectable:
//   .emgraw - the verbatim byte stream, including log text and anything the
//             parser rejected. Lossless, and the format ReplaySource consumes.
//   .csv    - t_s,ch1_uv,...  for analysis in Python/MATLAB.
//
// The recorder is handed raw bytes and runs its own FrameParser to produce the
// CSV. That costs one extra decode of a 12.6 kB/s stream and buys a single
// narrow interface (bytes in) plus a guarantee that .emgraw is byte-exact.

#include "core/FrameParser.h"

#include <QFile>
#include <QObject>
#include <QTextStream>

class Recorder : public QObject {
    Q_OBJECT

public:
    explicit Recorder(QObject *parent = nullptr);
    ~Recorder() override;

public slots:
    // basePath without extension; .emgraw / .csv are appended as enabled.
    void startRecording(const QString &basePath, bool raw, bool csv);
    void stopRecording();
    void write(const QByteArray &bytes);

signals:
    void recordingStarted(const QString &rawPath, const QString &csvPath);
    void recordingStopped(qint64 bytesWritten, qint64 samplesWritten);
    void errorOccurred(const QString &message);
    void progress(qint64 bytesWritten, qint64 samplesWritten);

private:
    void handleData(const emg::DataFrame &frame);
    void handleInfo(const emg::InfoFrame &info);
    void closeFiles();

    QFile m_rawFile;
    QFile m_csvFile;
    QTextStream m_csv;

    emg::FrameParser m_parser;
    double m_uvPerCode = 2400000.0 / 6.0 / 8388608.0;
    int m_sampleRate = 1000;

    bool m_active = false;
    bool m_csvHeaderWritten = false;
    qint64 m_bytes = 0;
    qint64 m_samples = 0;   // samples per channel
    quint64 m_sampleIndex = 0;
};
