#include "riscv_sim/registers.hpp"
#include "riscv_sim/bits.hpp"

namespace riscv_sim {

const std::array<const char*, 32> kAbiNames = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6",
};

std::int32_t RegisterFile::read_signed(int index) const {
    return to_signed(regs_[index]);
}

void RegisterFile::write(int index, std::uint32_t value) {
    if (index == 0) {
        return;
    }
    regs_[index] = value;
}

}  // namespace riscv_sim
