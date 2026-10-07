#include "Crc16.h"

#include <array>

namespace emg {
namespace {

// Nibble-wise table: 16 entries instead of 256, four times less cache pressure
// than a byte table and still 8x fewer iterations than the bitwise form. At
// 12.6 kB/s the CRC is nowhere near a bottleneck; this is just tidy.
constexpr std::array<std::uint16_t, 16> kTable = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
};

} // namespace

std::uint16_t crc16(const std::uint8_t *data, std::size_t len) noexcept
{
    std::uint16_t crc = 0xFFFF;
    if (data == nullptr) {
        return crc;
    }

    for (std::size_t i = 0; i < len; ++i) {
        crc = static_cast<std::uint16_t>((crc << 4) ^
                                         kTable[((crc >> 12) ^ (data[i] >> 4)) & 0x0F]);
        crc = static_cast<std::uint16_t>((crc << 4) ^
                                         kTable[((crc >> 12) ^ (data[i] & 0x0F)) & 0x0F]);
    }
    return crc;
}

} // namespace emg
