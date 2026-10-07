#pragma once

// Host-side mirror of firmware/proto/inc/emg_frame.h.
//
// The constants are duplicated rather than #included so that this application
// stays buildable without the firmware repo checked out alongside it. The
// duplication is not left to trust: tests/tst_frameparser.cpp includes BOTH
// headers and static_asserts every value here against the firmware's, so a
// change on either side fails to compile rather than corrupting a capture.
//
// Normative spec: software/ARCHITECTURE.md section 4.

#include <cstddef>
#include <cstdint>

namespace emg {

inline constexpr std::uint8_t kMagic0 = 0xAA;
inline constexpr std::uint8_t kMagic1 = 0x55;
inline constexpr std::uint8_t kVersion = 0x01;

enum class FrameType : std::uint8_t {
    Data = 0x01,
    Info = 0x02,
    Text = 0x03,
};

inline constexpr std::size_t kHeaderSize = 6;   // magic(2) type(1) ver(1) len(2)
inline constexpr std::size_t kCrcSize = 2;
inline constexpr std::size_t kOverhead = kHeaderSize + kCrcSize;

inline constexpr std::size_t kMaxChannels = 8;
inline constexpr std::size_t kMaxSamples = 64;
inline constexpr std::size_t kMaxPayload = 1024;

inline constexpr std::size_t kDataHeaderSize = 12;
inline constexpr std::size_t kBytesPerSample = 3;

inline constexpr std::uint8_t kFlagOverflow = 1u << 0;
inline constexpr std::uint8_t kFlagLeadOff = 1u << 1;

inline constexpr std::size_t kInfoPayloadSize = 22;
inline constexpr std::size_t kFwVersionLen = 8;

inline constexpr std::size_t kMaxFrameSize =
    kOverhead + kDataHeaderSize + kMaxChannels * kMaxSamples * kBytesPerSample;

// Positive full scale of the ADS1298's 24-bit output, i.e. 2^23. Matches
// ADS_FS_CODE in firmware/src/main.c.
inline constexpr double kFullScaleCode = 8388608.0;

} // namespace emg
