// Bit-manipulation helpers shared across the simulator.
#pragma once

#include <cstdint>

namespace riscv_sim {

// Reinterpret an arbitrary 32-bit pattern as unsigned (no-op on uint32_t, kept
// for symmetry with to_signed and for converting from signed inputs).
inline std::uint32_t to_unsigned(std::int64_t value) {
    return static_cast<std::uint32_t>(value);
}

// Reinterpret an unsigned 32-bit value as a signed 32-bit int.
inline std::int32_t to_signed(std::uint32_t value) {
    return static_cast<std::int32_t>(value);
}

// Sign-extend `value` (an unsigned int with `bits` significant bits) to a
// full-width 32-bit signed result.
inline std::int32_t sign_extend(std::uint32_t value, int bits) {
    std::uint32_t mask = 1u << (bits - 1);
    value &= (bits == 32) ? 0xFFFFFFFFu : ((1u << bits) - 1u);
    return static_cast<std::int32_t>((value ^ mask) - mask);
}

}  // namespace riscv_sim
