// The RV32I integer register file.
#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace riscv_sim {

extern const std::array<const char*, 32> kAbiNames;

// 32 general-purpose registers, x0-x31. x0 is hardwired to zero.
class RegisterFile {
public:
    RegisterFile() = default;

    // Read register `index` as an unsigned 32-bit value.
    std::uint32_t read(int index) const { return regs_[index]; }

    // Read register `index` as a signed 32-bit value.
    std::int32_t read_signed(int index) const;

    // Write `value` to register `index`. Writes to x0 are discarded.
    void write(int index, std::uint32_t value);

    // Return a copy of all 32 register values.
    std::array<std::uint32_t, 32> snapshot() const { return regs_; }

    std::string name(int index) const { return "x" + std::to_string(index); }

    std::string abi_name(int index) const { return kAbiNames[index]; }

private:
    std::array<std::uint32_t, 32> regs_{};
};

}  // namespace riscv_sim
