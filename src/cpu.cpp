#include "riscv_sim/cpu.hpp"

#include "riscv_sim/bits.hpp"
#include "riscv_sim/errors.hpp"

namespace riscv_sim {

Instruction CPU::step() {
    try {
        std::uint32_t raw = memory.read_word(pc);
        Instruction instr = decode(raw);
        execute(instr);
        ++step_count;
        return instr;
    } catch (const SimulatorError& exc) {
        status = Status::Error;
        halt_reason = exc.what();
        throw;
    }
}

void CPU::run(std::optional<long> max_steps, const std::function<void(const Instruction&)>& on_step) {
    long steps = 0;
    while (status == Status::Running) {
        if (max_steps.has_value() && steps >= *max_steps) {
            status = Status::Error;
            halt_reason = "exceeded max_steps (" + std::to_string(*max_steps) + "); possible infinite loop";
            break;
        }
        Instruction instr = step();
        if (on_step) {
            on_step(instr);
        }
        ++steps;
    }
}

void CPU::execute(const Instruction& instr) {
    std::uint32_t pc_next = pc + 4;
    std::uint32_t rs1 = regs.read(instr.rs1);
    std::uint32_t rs2 = regs.read(instr.rs2);
    std::int32_t rs1_s = regs.read_signed(instr.rs1);
    std::int32_t rs2_s = regs.read_signed(instr.rs2);
    const std::string& m = instr.mnemonic;

    if (m == "add") {
        regs.write(instr.rd, rs1 + rs2);
    } else if (m == "sub") {
        regs.write(instr.rd, rs1 - rs2);
    } else if (m == "sll") {
        regs.write(instr.rd, rs1 << (rs2 & 0x1F));
    } else if (m == "slt") {
        regs.write(instr.rd, rs1_s < rs2_s ? 1 : 0);
    } else if (m == "sltu") {
        regs.write(instr.rd, rs1 < rs2 ? 1 : 0);
    } else if (m == "xor") {
        regs.write(instr.rd, rs1 ^ rs2);
    } else if (m == "srl") {
        regs.write(instr.rd, rs1 >> (rs2 & 0x1F));
    } else if (m == "sra") {
        regs.write(instr.rd, to_unsigned(rs1_s >> (rs2 & 0x1F)));
    } else if (m == "or") {
        regs.write(instr.rd, rs1 | rs2);
    } else if (m == "and") {
        regs.write(instr.rd, rs1 & rs2);

    } else if (m == "addi") {
        regs.write(instr.rd, rs1 + instr.imm);
    } else if (m == "slti") {
        regs.write(instr.rd, rs1_s < instr.imm ? 1 : 0);
    } else if (m == "sltiu") {
        regs.write(instr.rd, rs1 < to_unsigned(instr.imm) ? 1 : 0);
    } else if (m == "xori") {
        regs.write(instr.rd, rs1 ^ to_unsigned(instr.imm));
    } else if (m == "ori") {
        regs.write(instr.rd, rs1 | to_unsigned(instr.imm));
    } else if (m == "andi") {
        regs.write(instr.rd, rs1 & to_unsigned(instr.imm));
    } else if (m == "slli") {
        regs.write(instr.rd, rs1 << instr.imm);
    } else if (m == "srli") {
        regs.write(instr.rd, rs1 >> instr.imm);
    } else if (m == "srai") {
        regs.write(instr.rd, to_unsigned(rs1_s >> instr.imm));

    } else if (m == "lb" || m == "lh" || m == "lw" || m == "lbu" || m == "lhu") {
        std::uint32_t address = to_unsigned(rs1 + instr.imm);
        std::int32_t value;
        if (m == "lb") {
            value = memory.read_byte(address, /*signed_=*/true);
        } else if (m == "lh") {
            value = memory.read_half(address, /*signed_=*/true);
        } else if (m == "lw") {
            value = static_cast<std::int32_t>(memory.read_word(address));
        } else if (m == "lbu") {
            value = memory.read_byte(address, /*signed_=*/false);
        } else {
            value = memory.read_half(address, /*signed_=*/false);
        }
        regs.write(instr.rd, value);

    } else if (m == "sb" || m == "sh" || m == "sw") {
        std::uint32_t address = to_unsigned(rs1 + instr.imm);
        if (m == "sb") {
            memory.write_byte(address, rs2);
        } else if (m == "sh") {
            memory.write_half(address, rs2);
        } else {
            memory.write_word(address, rs2);
        }

    } else if (m == "beq" || m == "bne" || m == "blt" || m == "bge" || m == "bltu" || m == "bgeu") {
        bool taken = (m == "beq" && rs1 == rs2) || (m == "bne" && rs1 != rs2) ||
                     (m == "blt" && rs1_s < rs2_s) || (m == "bge" && rs1_s >= rs2_s) ||
                     (m == "bltu" && rs1 < rs2) || (m == "bgeu" && rs1 >= rs2);
        if (taken) {
            pc_next = to_unsigned(static_cast<std::int64_t>(pc) + instr.imm);
        }

    } else if (m == "lui") {
        regs.write(instr.rd, instr.imm);
    } else if (m == "auipc") {
        regs.write(instr.rd, to_unsigned(static_cast<std::int64_t>(pc) + instr.imm));

    } else if (m == "jal") {
        regs.write(instr.rd, pc_next);
        pc_next = to_unsigned(static_cast<std::int64_t>(pc) + instr.imm);
    } else if (m == "jalr") {
        regs.write(instr.rd, pc_next);
        pc_next = to_unsigned(rs1 + instr.imm) & ~0x1u;

    } else if (m == "fence") {
        // no-op
    } else if (m == "ecall") {
        status = Status::Halted;
        halt_reason = "ecall";
        exit_code = regs.read_signed(10);
    } else if (m == "ebreak") {
        status = Status::Halted;
        halt_reason = "ebreak";
        exit_code = regs.read_signed(10);
    } else {
        throw IllegalInstructionError("unimplemented mnemonic '" + m + "'");
    }

    pc = pc_next;
}

}  // namespace riscv_sim
