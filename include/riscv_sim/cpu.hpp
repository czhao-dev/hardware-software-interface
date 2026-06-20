// The CPU execution engine: applies decoded instructions to machine state.
#pragma once

#include <functional>
#include <optional>
#include <string>

#include "riscv_sim/decoder.hpp"
#include "riscv_sim/memory.hpp"
#include "riscv_sim/registers.hpp"

namespace riscv_sim {

enum class Status { Running, Halted, Error };

// RV32I CPU state and execution engine.
class CPU {
public:
    explicit CPU(Memory memory = Memory(), std::uint32_t pc = 0)
        : memory(std::move(memory)), pc(pc) {}

    // Fetch, decode, and execute a single instruction.
    //
    // Returns the decoded instruction. If a fault occurs, `status` is set to
    // Status::Error and `halt_reason` is populated; the caller should stop
    // stepping once `status` is no longer Status::Running.
    Instruction step();

    // Run until halted, faulted, or `max_steps` is reached. Pass
    // std::nullopt for `max_steps` to run unbounded.
    void run(std::optional<long> max_steps = 100'000,
             const std::function<void(const Instruction&)>& on_step = nullptr);

    Memory memory;
    RegisterFile regs;
    std::uint32_t pc;
    Status status = Status::Running;
    std::optional<std::string> halt_reason;
    std::int32_t exit_code = 0;
    long step_count = 0;

private:
    void execute(const Instruction& instr);
};

}  // namespace riscv_sim
