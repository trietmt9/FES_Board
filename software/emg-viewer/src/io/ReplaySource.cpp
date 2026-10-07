#include "ReplaySource.h"

namespace {
// How often we top up the parser. 20 ms is well under the 32 ms block period,
// so playback stays smooth without the timer itself becoming the pacer.
constexpr int kTickMs = 20;
// One DATA frame is 404 B, worth 32 ms of recorded time; keep chunks near
// that so pacing granularity is a frame rather than a large buffer.
constexpr qint64 kChunkBytes = 512;
} // namespace

ReplaySource::ReplaySource(QObject *parent) : ISampleSource(parent)
{
    m_parser.setDataHandler([this](const emg::DataFrame &f) { handleData(f); });
    m_parser.setInfoHandler([this](const emg::InfoFrame &i) { handleInfo(i); });
    m_parser.setTextHandler([this](const QString &s) { emit textLine(s); });
}

ReplaySource::~ReplaySource() = default;

void ReplaySource::start()
{
    if (m_file.isOpen()) {
        stop();
    }

    m_file.setFileName(m_path);
    if (!m_file.open(QIODevice::ReadOnly)) {
        emit errorOccurred(tr("Cannot open %1: %2").arg(m_path, m_file.errorString()));
        return;
    }

    rewind();

    if (!m_timer) {
        m_timer = new QTimer(this);
        m_timer->setInterval(kTickMs);
        connect(m_timer, &QTimer::timeout, this, &ReplaySource::pump);
    }
    m_paused = false;
    m_timer->start();

    emit started();
    emit pausedChanged(false);
}

void ReplaySource::rewind()
{
    m_file.seek(0);
    m_parser.reset();
    m_overflows = 0;
    m_loss.reset();
    m_haveMask = false;
    m_haveFirstTMs = false;
    m_firstTMs = 0;
    m_lastTMs = 0;
    m_playedMs = 0.0;
    m_blockBudgetSpent = false;
    m_statsClock.start();
    if (m_ring) {
        m_ring->clear();
    }
    if (m_filteredRing) {
        m_filteredRing->clear();
    }
    reconfigureFilters();
}

void ReplaySource::stop()
{
    if (m_timer) {
        m_timer->stop();
    }
    if (m_file.isOpen()) {
        m_parser.flushText();
        emit statsUpdated(m_parser.stats(), m_overflows, m_loss.lost());
        m_file.close();
    }
    emit stopped();
}

void ReplaySource::pause()
{
    if (m_paused || !m_timer) {
        return;
    }
    m_paused = true;
    m_timer->stop();
    emit pausedChanged(true);
}

void ReplaySource::resume()
{
    if (!m_paused || !m_timer || !m_file.isOpen()) {
        return;
    }
    m_paused = false;
    m_timer->start();
    emit pausedChanged(false);
}

void ReplaySource::setSpeed(double multiplier)
{
    m_speed = qBound(0.1, multiplier, 20.0);
}

void ReplaySource::seekToStart()
{
    if (!m_file.isOpen()) {
        return;
    }
    rewind();
    emit progressChanged(0.0);
}

void ReplaySource::pump()
{
    if (!m_file.isOpen()) {
        return;
    }

    // Budget of recorded milliseconds to release this tick.
    m_playedMs += kTickMs * m_speed;
    m_blockBudgetSpent = false;

    while (!m_blockBudgetSpent) {
        // Check the budget BEFORE reading, not after. Checking afterwards
        // guarantees at least one whole chunk per tick, and a chunk holds far
        // more recorded time than a tick is worth - which played a 30 s capture
        // back in about 9 s.
        if (m_haveFirstTMs &&
            static_cast<double>(m_lastTMs - m_firstTMs) >= m_playedMs) {
            break;
        }

        const QByteArray chunk = m_file.read(kChunkBytes);
        if (chunk.isEmpty()) {
            m_parser.flushText();
            emit statsUpdated(m_parser.stats(), m_overflows, m_loss.lost());
            emit progressChanged(1.0);
            m_timer->stop();
            m_file.close();
            emit finished();
            emit stopped();
            return;
        }

        if (m_forwardRaw) {
            emit rawBytes(chunk);
        }
        m_parser.feed(chunk);
    }

    const qint64 size = m_file.size();
    if (size > 0) {
        emit progressChanged(static_cast<double>(m_file.pos()) / size);
    }

    // Throttle to ~1 Hz, matching SerialSource. Emitting every 20 ms tick makes
    // the derived rates meaningless: most ticks decode zero or one frame, so the
    // deltas the controller divides by dt round to nothing.
    if (!m_statsClock.isValid() || m_statsClock.elapsed() >= 1000) {
        m_statsClock.restart();
        emit statsUpdated(m_parser.stats(), m_overflows, m_loss.lost());
    }
}

void ReplaySource::handleInfo(const emg::InfoFrame &info)
{
    m_info = info;
    if (info.isValid()) {
        m_uvPerCode = info.uvPerCode();

        // Filter corners are rate-dependent; only correct once the capture has
        // told us its actual rate.
        if (info.sampleRateHz != m_filterRate) {
            m_filterRate = info.sampleRateHz;
            // The loss tracker converts elapsed t_ms into an expected sample
            // count, so a rate change invalidates everything measured so far.
            m_loss.reset();
            reconfigureFilters();
        }
    }
    emit infoReceived(info);
}

void ReplaySource::handleData(const emg::DataFrame &frame)
{
    if (!m_haveFirstTMs) {
        m_firstTMs = frame.tMs;
        m_haveFirstTMs = true;
    }
    m_lastTMs = frame.tMs;

    if (frame.overflow()) {
        ++m_overflows;
    }
    noteDataFrame(frame);

    if (m_ring) {
        const std::size_t count = frame.codes.size();
        if (m_scratch.size() < count) {
            m_scratch.resize(count);
        }
        for (std::size_t i = 0; i < count; ++i) {
            m_scratch[i] = static_cast<float>(frame.codes[i] * m_uvPerCode);
        }
        m_ring->write(m_scratch.data(), frame.nCh, frame.nSamp);

        if (m_filteredRing) {
            if (m_filtered.size() < count) {
                m_filtered.resize(count);
            }
            filterBlock(m_scratch.data(), m_filtered.data(), frame.nCh, frame.nSamp);
            m_filteredRing->write(m_filtered.data(), frame.nCh, frame.nSamp);
        }
    }

    // Once we have released as much recorded time as this tick allows, tell
    // pump() to stop feeding. Using the frame timestamps rather than byte
    // counts keeps playback at true recorded speed.
    const double elapsed = static_cast<double>(m_lastTMs - m_firstTMs);
    if (elapsed >= m_playedMs) {
        m_blockBudgetSpent = true;
    }
}
