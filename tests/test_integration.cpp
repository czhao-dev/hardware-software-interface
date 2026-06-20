#include <array>
#include <cstdio>
#include <string>

#include "riscv_sim/cpu.hpp"
#include "riscv_sim/loader.hpp"
#include "test_framework.hpp"

// RISCV_SIM_EXECUTABLE and EXAMPLES_DIR are injected by CMake as absolute
// paths to the built CLI binary and the examples/ directory.

using riscv_sim::cpu_from_binary;
using riscv_sim::Status;

namespace {

std::string run_cli(const std::string& program_path, const std::string& flags) {
    std::string command = "\"" + std::string(RISCV_SIM_EXECUTABLE) + "\" \"" + program_path + "\" " +
                           flags + " 2>&1";
    std::array<char, 256> buffer;
    std::string output;
    FILE* pipe = popen(command.c_str(), "r");
    REQUIRE(pipe != nullptr);
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        output += buffer.data();
    }
    pclose(pipe);
    return output;
}

bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

}  // namespace

TEST_CASE("arithmetic_example_program") {
    auto cpu = cpu_from_binary(std::string(EXAMPLES_DIR) + "/arithmetic/addi.bin");
    cpu.run(10);
    CHECK(cpu.status == Status::Halted);
    CHECK(cpu.regs.read(1) == 5);
    CHECK(cpu.regs.read(2) == 10);
    CHECK(cpu.regs.read(3) == 15);
}

TEST_CASE("branches_example_program") {
    auto cpu = cpu_from_binary(std::string(EXAMPLES_DIR) + "/branches/loop.bin");
    cpu.run(100);
    CHECK(cpu.status == Status::Halted);
    CHECK(cpu.regs.read(1) == 0);
    CHECK(cpu.regs.read(2) == 15);  // 5 + 4 + 3 + 2 + 1
}

TEST_CASE("memory_example_program") {
    auto cpu = cpu_from_binary(std::string(EXAMPLES_DIR) + "/memory/load_store.bin");
    cpu.run(10);
    CHECK(cpu.status == Status::Halted);
    CHECK(cpu.regs.read(1) == 100);
    CHECK(cpu.regs.read(2) == 64);
    CHECK(cpu.regs.read(3) == 100);
}

TEST_CASE("cli_trace_output_matches_readme_example") {
    std::string output = run_cli(std::string(EXAMPLES_DIR) + "/arithmetic/addi.bin", "--trace");
    CHECK(contains(output, "pc=0x00000000  instr=0x00500093  addi x1, x0, 5"));
    CHECK(contains(output, "pc=0x00000004  instr=0x00a00113  addi x2, x0, 10"));
    CHECK(contains(output, "pc=0x00000008  instr=0x002081b3  add x3, x1, x2"));
    CHECK(contains(output, "x3 = 0x0000000f"));
    CHECK(contains(output, "halted: ebreak"));
}

TEST_CASE("cli_dump_registers") {
    std::string output = run_cli(std::string(EXAMPLES_DIR) + "/memory/load_store.bin", "--dump-registers");
    CHECK(contains(output, "x1  = 0x00000064"));
    CHECK(contains(output, "x2  = 0x00000040"));
    CHECK(contains(output, "x3  = 0x00000064"));
}
