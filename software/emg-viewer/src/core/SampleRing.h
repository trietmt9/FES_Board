#pragma once

// Lock-free single-producer / single-consumer sample store.
//
// The IO thread writes decoded microvolt samples here; the GUI thread reads the
// most recent window 60 times a second. Neither ever blocks the other, which is
// the point: a slow repaint must degrade into dropped *video* frames, never
// dropped samples.
//
// Channels advance in lockstep (every DATA frame carries the same number of
// samples for each channel), so one shared monotonic write counter indexes all
// of them. The counter is monotonic rather than wrapped so the reader can tell
// how far it has fallen behind.
//
// Tearing contract: a reader that falls more than `capacity` samples behind can
// observe a torn window. At 1000 SPS with a 10 s capacity that requires the GUI
// thread to stall for ten seconds, which would be a far larger problem than the
// torn pixels. Readers should still clamp their request to `capacity`.

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace emg {

class SampleRing {
public:
    // Not thread-safe: call before the producer starts.
    void configure(int channels, std::size_t capacity)
    {
        m_channels = channels > 0 ? channels : 1;
        m_capacity = capacity > 0 ? capacity : 1;
        m_data.assign(static_cast<std::size_t>(m_channels) * m_capacity, 0.0f);
        m_written.store(0, std::memory_order_release);
    }

    void clear()
    {
        std::fill(m_data.begin(), m_data.end(), 0.0f);
        m_written.store(0, std::memory_order_release);
    }

    int channels() const { return m_channels; }
    std::size_t capacity() const { return m_capacity; }

    // Total samples per channel ever written. Monotonic.
    std::uint64_t written() const { return m_written.load(std::memory_order_acquire); }

    /**
     * Producer. @p interleaved is sample-major (sample * nCh + channel), which
     * is the order the wire delivers, so no transpose is needed here.
     */
    void write(const float *interleaved, int nCh, int nSamp)
    {
        if (m_data.empty() || nSamp <= 0) {
            return;
        }
        const int n = nCh < m_channels ? nCh : m_channels;
        std::uint64_t w = m_written.load(std::memory_order_relaxed);

        for (int s = 0; s < nSamp; ++s) {
            const std::size_t slot = static_cast<std::size_t>((w + s) % m_capacity);
            for (int c = 0; c < n; ++c) {
                m_data[static_cast<std::size_t>(c) * m_capacity + slot] =
                    interleaved[static_cast<std::size_t>(s) * nCh + c];
            }
        }

        // Release: publish the samples before the index that advertises them.
        m_written.store(w + static_cast<std::uint64_t>(nSamp),
                        std::memory_order_release);
    }

    /**
     * Copy this ring's whole contents into @p dst, for a display freeze.
     *
     * Snapshotting rather than just remembering a read position matters: the
     * producer keeps running while the display is frozen, so within one ring
     * capacity (10 s here) it would overwrite the very samples being looked at.
     *
     * The producer may write during the copy, so the newest sample or two can
     * tear. That is invisible in a frozen trace and not worth a lock on the
     * acquisition path to avoid.
     */
    void snapshotInto(SampleRing &dst) const
    {
        dst.m_channels = m_channels;
        dst.m_capacity = m_capacity;
        dst.m_data = m_data;
        dst.m_written.store(m_written.load(std::memory_order_acquire),
                            std::memory_order_release);
    }

    /**
     * Consumer. Copies the most recent @p count samples of @p channel into
     * @p out, oldest first.
     *
     * @return number of samples actually copied (less than @p count only while
     *         the ring is still filling after a reset).
     */
    std::size_t readLatest(int channel, float *out, std::size_t count) const
    {
        if (m_data.empty() || channel < 0 || channel >= m_channels ||
            out == nullptr || count == 0) {
            return 0;
        }
        if (count > m_capacity) {
            count = m_capacity;
        }

        const std::uint64_t w = m_written.load(std::memory_order_acquire);
        const std::size_t have = w < count ? static_cast<std::size_t>(w) : count;
        if (have == 0) {
            return 0;
        }

        const std::uint64_t start = w - have;
        const float *base = m_data.data() + static_cast<std::size_t>(channel) * m_capacity;

        for (std::size_t i = 0; i < have; ++i) {
            out[i] = base[static_cast<std::size_t>((start + i) % m_capacity)];
        }
        return have;
    }

    /**
     * Consumer. Like readLatest(), but the window ENDS at sample index @p end
     * (exclusive) instead of at whatever the write counter happens to be now.
     *
     * A renderer needs this. It decides where each sample goes on screen from a
     * head index, then reads the samples; if those two reads of the write
     * counter differ - and at 1 kSPS the producer advances a frame's worth of
     * samples in 16 ms - the whole trace is drawn shifted by the difference.
     * Capture the head once, then read against it.
     *
     * @p end is clamped to the write counter, and the window is shortened rather
     * than read from slots the producer has already lapped.
     * @return samples copied, oldest first, ending at the (clamped) @p end.
     */
    std::size_t readEndingAt(int channel, std::uint64_t end, float *out,
                             std::size_t count) const
    {
        if (m_data.empty() || channel < 0 || channel >= m_channels || out == nullptr ||
            count == 0) {
            return 0;
        }

        const std::uint64_t w = m_written.load(std::memory_order_acquire);
        if (end > w) {
            end = w;
        }
        if (count > m_capacity) {
            count = m_capacity;
        }

        // Oldest sample still intact in the ring.
        const std::uint64_t oldest = w > m_capacity ? w - m_capacity : 0;
        if (end <= oldest) {
            return 0;
        }
        std::size_t have = count;
        if (end - have < oldest) {
            have = static_cast<std::size_t>(end - oldest);
        }
        if (have > end) {
            have = static_cast<std::size_t>(end);
        }
        if (have == 0) {
            return 0;
        }

        const std::uint64_t start = end - have;
        const float *base = m_data.data() + static_cast<std::size_t>(channel) * m_capacity;
        for (std::size_t i = 0; i < have; ++i) {
            out[i] = base[static_cast<std::size_t>((start + i) % m_capacity)];
        }
        return have;
    }

private:
    std::vector<float> m_data; // channel-major: [channel][slot]
    int m_channels = 0;
    std::size_t m_capacity = 0;
    std::atomic<std::uint64_t> m_written{0};
};

} // namespace emg
