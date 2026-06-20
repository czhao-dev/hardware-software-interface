#include "riscv_sim/registers.hpp"
#include "test_framework.hpp"

using namespace riscv_sim;

TEST_CASE("initial_state_is_zero") {
    RegisterFile regs;
    auto snap = regs.snapshot();
    for (std::uint32_t v : snap) {
        CHECK(v == 0);
    }
}

TEST_CASE("x0_is_hardwired_to_zero") {
    RegisterFile regs;
    regs.write(0, 0xDEADBEEF);
    CHECK(regs.read(0) == 0);
}

TEST_CASE("write_and_read_round_trip") {
    RegisterFile regs;
    regs.write(5, 42);
    CHECK(regs.read(5) == 42);
}

TEST_CASE("negative_values_wrap_to_unsigned_32_bit") {
    RegisterFile regs;
    regs.write(1, static_cast<std::uint32_t>(-1));
    CHECK(regs.read(1) == 0xFFFFFFFFu);
    CHECK(regs.read_signed(1) == -1);
}

TEST_CASE("abi_names") {
    RegisterFile regs;
    CHECK(regs.abi_name(0) == "zero");
    CHECK(regs.abi_name(2) == "sp");
    CHECK(regs.abi_name(10) == "a0");
}
