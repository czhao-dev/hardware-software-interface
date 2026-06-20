#include <vector>

#include "riscv_sim/cpu.hpp"
#include "riscv_sim/encoder.hpp"
#include "riscv_sim/errors.hpp"
#include "riscv_sim/loader.hpp"
#include "test_framework.hpp"

namespace e = riscv_sim::encoder;
using riscv_sim::CPU;
using riscv_sim::load_words;
using riscv_sim::MemoryAccessError;
using riscv_sim::Status;

namespace {
CPU cpu_with(const std::vector<std::uint32_t>& words, std::uint32_t base_address = 0) {
    return CPU(load_words(words, base_address), base_address);
}
}  // namespace

TEST_CASE("addi_and_add") {
    auto cpu = cpu_with({e::addi(1, 0, 5), e::addi(2, 0, 10), e::add(3, 1, 2)});
    cpu.step();
    cpu.step();
    cpu.step();
    CHECK(cpu.regs.read(1) == 5);
    CHECK(cpu.regs.read(2) == 10);
    CHECK(cpu.regs.read(3) == 15);
    CHECK(cpu.pc == 12);
}

TEST_CASE("sub_underflow_wraps_to_unsigned") {
    auto cpu = cpu_with({e::addi(1, 0, 1), e::addi(2, 0, 2), e::sub(3, 1, 2)});
    cpu.run(3);
    CHECK(cpu.regs.read(3) == 0xFFFFFFFFu);
    CHECK(cpu.regs.read_signed(3) == -1);
}

TEST_CASE("logical_r_type_and") {
    auto cpu = cpu_with({e::addi(1, 0, 0b1100), e::addi(2, 0, 0b1010), e::and_(3, 1, 2)});
    cpu.run(3);
    CHECK(cpu.regs.read(3) == 0b1000u);
}

TEST_CASE("logical_r_type_or") {
    auto cpu = cpu_with({e::addi(1, 0, 0b1100), e::addi(2, 0, 0b1010), e::or_(3, 1, 2)});
    cpu.run(3);
    CHECK(cpu.regs.read(3) == 0b1110u);
}

TEST_CASE("logical_r_type_xor") {
    auto cpu = cpu_with({e::addi(1, 0, 0b1100), e::addi(2, 0, 0b1010), e::xor_(3, 1, 2)});
    cpu.run(3);
    CHECK(cpu.regs.read(3) == 0b0110u);
}

TEST_CASE("shifts") {
    auto cpu = cpu_with({e::addi(1, 0, 1), e::slli(2, 1, 4), e::srli(3, 2, 2)});
    cpu.run(3);
    CHECK(cpu.regs.read(2) == 0x10u);
    CHECK(cpu.regs.read(3) == 0x4u);
}

TEST_CASE("sra_preserves_sign") {
    auto cpu = cpu_with({e::addi(1, 0, -8), e::srai(2, 1, 1)});
    cpu.run(2);
    CHECK(cpu.regs.read_signed(2) == -4);
}

TEST_CASE("slt_signed_vs_sltu_unsigned") {
    auto cpu = cpu_with({e::addi(1, 0, -1), e::addi(2, 0, 1), e::slt(3, 1, 2), e::sltu(4, 1, 2)});
    cpu.run(4);
    CHECK(cpu.regs.read(3) == 1);  // -1 < 1 (signed)
    CHECK(cpu.regs.read(4) == 0);  // 0xFFFFFFFF is not < 1 (unsigned)
}

TEST_CASE("store_and_load_word") {
    auto cpu = cpu_with({e::addi(1, 0, 100), e::addi(2, 0, 64), e::sw(2, 1, 0), e::lw(3, 2, 0)});
    cpu.run(4);
    CHECK(cpu.regs.read(3) == 100);
}

TEST_CASE("load_byte_sign_extension") {
    auto cpu =
        cpu_with({e::addi(1, 0, -1), e::addi(2, 0, 64), e::sb(2, 1, 0), e::lb(3, 2, 0), e::lbu(4, 2, 0)});
    cpu.run(5);
    CHECK(cpu.regs.read_signed(3) == -1);
    CHECK(cpu.regs.read(4) == 0xFFu);
}

TEST_CASE("invalid_memory_access_raises") {
    // lui sets x1 to a huge address that is well beyond the default 1 MiB memory.
    auto cpu = cpu_with({e::lui(1, 0x7FFFF000), e::lw(2, 1, 0)});
    cpu.step();
    CHECK_THROWS_AS(cpu.step(), MemoryAccessError);
    CHECK(cpu.status == Status::Error);
}

TEST_CASE("branch_taken_and_not_taken") {
    // bne(x1, x0, +8) skips an addi when x1 != 0.
    auto cpu = cpu_with({e::addi(1, 0, 1), e::bne(1, 0, 8), e::addi(2, 0, 99), e::addi(3, 0, 42)});
    cpu.run(3);
    CHECK(cpu.regs.read(2) == 0);  // skipped
    CHECK(cpu.regs.read(3) == 42);
}

TEST_CASE("loop_sums_one_through_five") {
    std::vector<std::uint32_t> words = {
        e::addi(1, 0, 5),
        e::addi(2, 0, 0),
        e::add(2, 2, 1),
        e::addi(1, 1, -1),
        e::bne(1, 0, -8),
        e::ebreak(),
    };
    auto cpu = cpu_with(words);
    cpu.run(100);
    CHECK(cpu.status == Status::Halted);
    CHECK(cpu.regs.read(2) == 15);
    CHECK(cpu.regs.read(1) == 0);
}

TEST_CASE("jal_and_jalr") {
    // jal x1, +8 jumps over one addi and links return address in x1.
    auto cpu = cpu_with({e::jal(1, 8), e::addi(2, 0, 99), e::addi(3, 0, 42)});
    cpu.run(2);
    CHECK(cpu.regs.read(1) == 4);
    CHECK(cpu.regs.read(2) == 0);
    CHECK(cpu.regs.read(3) == 42);
}

TEST_CASE("lui_and_auipc") {
    auto cpu = cpu_with({e::lui(1, 0x12345000), e::auipc(2, 0x1000)}, 0x100);
    cpu.run(2);
    CHECK(cpu.regs.read(1) == 0x12345000u);
    CHECK(cpu.regs.read(2) == 0x100u + 0x1000u + 4u);
}

TEST_CASE("ecall_halts_with_exit_code_from_a0") {
    auto cpu = cpu_with({e::addi(10, 0, 7), e::ecall()});
    cpu.run(2);
    CHECK(cpu.status == Status::Halted);
    CHECK(cpu.halt_reason == "ecall");
    CHECK(cpu.exit_code == 7);
}

TEST_CASE("x0_writes_are_discarded") {
    auto cpu = cpu_with({e::addi(0, 0, 123)});
    cpu.run(1);
    CHECK(cpu.regs.read(0) == 0);
}
