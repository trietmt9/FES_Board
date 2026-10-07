#pragma once

// Playback of a recorded .emgraw capture.
//
// Feeds the *same* FrameParser as the live link, so replaying a capture
// exercises the real decode path rather than a parallel implementation of it -
// and so the whole UI can be developed and demonstrated with no board attached.
//
// Pacing follows the DATA frames' embedded t_ms rather than wall-clock chunk
// sizes, so playback runs at true recorded speed regardless of how the file was
// buffered.

#include "io/ISampleSource.h"

#include <QElapsedTimer>
#include <QFile>
#include <QTimer>

#include <vector>

class ReplaySource : public ISampleSource {
    Q_OBJECT

public:
    explicit ReplaySource(QObject *parent = nullptr);
    ~ReplaySource() override;

    void setFilePath(const QString &path) { m_path = path; }
    void setForwardRaw(bool on) { m_forwardRaw = on; }

public slots:
    void start() override;
    void stop() override;

    void pause();
    void resume();
    void setSpeed(double multiplier);
    void seekToStart();

signals:
    void progressChanged(double fraction);
    void finished();
    void pausedChanged(bool paused);

private slots:
    void pump();

private:
    void handleData(const emg::DataFrame &frame);
    void handleInfo(const emg::InfoFrame &info);
    void rewind();

    QFile m_file;
    QString m_path;
    QTimer *m_timer = nullptr;

    emg::FrameParser m_parser;
    emg::InfoFrame m_info;
    double m_uvPerCode = 2400000.0 / 6.0 / 8388608.0;

    double m_speed = 1.0;
    bool m_paused = false;
    bool m_forwardRaw = false;

    // Set by handleData when a block's timestamp runs past the budget for this
    // pump() call; pump() then stops feeding until the next tick.
    bool m_blockBudgetSpent = false;
    quint32 m_firstTMs = 0;
    quint32 m_lastTMs = 0;
    bool m_haveFirstTMs = false;
    double m_playedMs = 0.0;

    QElapsedTimer m_statsClock;
    quint64 m_overflows = 0;
    std::vector<float> m_scratch;   // raw microvolts
    std::vector<float> m_filtered;  // band-passed microvolts
};
