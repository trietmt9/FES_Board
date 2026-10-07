#pragma once

// CRC-16/CCITT-FALSE: poly 0x1021, init 0xFFFF, no reflection, xorout 0x0000.
// Check value for "123456789" is 0x29B1.
//
// Implemented table-driven here while the firmware's emg_crc16() is bitwise.
// The divergence is intentional: tests/tst_frameparser.cpp checks the two
// against each other, which only proves something if they are independent
// implementations rather than the same code compiled twice.

#include <cstddef>
#include <cstdint>

namespace emg {

std::uint16_t crc16(const std::uint8_t *data, std::size_t len) noexcept;

} // namespace emg
