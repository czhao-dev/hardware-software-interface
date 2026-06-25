// Command-line interface for the RV32I simulator.
#include <array>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "riscv_sim/cpu.hpp"
#include "riscv_sim/decoder.hpp"
#include "riscv_sim/loader.hpp"

namespace {

using namespace riscv_sim;

struct Args {
    std::string program;
    std::uint32_t base_address = 0;
    bool trace = false;
    bool step = false;
    bool dump_registers = false;
    long max_steps = 100'000;
};

void print_usage() {
    std::cerr << "usage: riscv-sim [-h] [--base-address BASE_ADDRESS] [--trace] [--step]\n"
                 "                  [--dump-registers] [--max-steps MAX_STEPS]\n"
                 "                  program\n\n"
                 "Run a RISC-V (RV32I) machine-code binary.\n\n"
                 "positional arguments:\n"
                 "  program               path to a flat binary file of RV32I machine code\n\n"
                 "options:\n"
                 "  -h, --help            show this help message and exit\n"
                 "  --base-address BASE_ADDRESS\n"
                 "                        address to load the program at\n"
                 "  --trace               print each instruction and its register changes\n"
                 "  --step                pause for Enter before executing each instruction\n"
                 "  --dump-registers      print the final register state\n"
                 "  --max-steps MAX_STEPS\n"
                 "                        abort after this many instructions\n";
}

Args parse_args(int argc, char** argv) {
    Args args;
    bool have_program = false;
    std::vector<std::string> argv_strs(argv + 1, argv + argc);

    for (std::size_t i = 0; i < argv_strs.size(); ++i) {
        const std::string& arg = argv_strs[i];
        if (arg == "-h" || arg == "--help") {
            print_usage();
            std::exit(0);
        } else if (arg == "--base-address") {
            if (i + 1 >= argv_strs.size()) {
                std::cerr << "error: --base-address requires a value\n";
                std::exit(2);
            }
            args.base_address = static_cast<std::uint32_t>(std::stoul(argv_strs[++i], nullptr, 0));
        } else if (arg == "--trace") {
            args.trace = true;
        } else if (arg == "--step") {
            args.step = true;
        } else if (arg == "--dump-registers") {
            args.dump_registers = true;
        } else if (arg == "--max-steps") {
            if (i + 1 >= argv_strs.size()) {
                std::cerr << "error: --max-steps requires a value\n";
                std::exit(2);
            }
            args.max_steps = std::stol(argv_strs[++i]);
        } else if (!have_program) {
            args.program = arg;
            have_program = true;
        } else {
            std::cerr << "error: unrecognized argument '" << arg << "'\n";
            std::exit(2);
        }
    }

    if (!have_program) {
        std::cerr << "error: the following arguments are required: program\n";
        print_usage();
        std::exit(2);
    }
    return args;
}

std::string format_registers_changed(const std::array<std::uint32_t, 32>& before,
                                      const std::array<std::uint32_t, 32>& after) {
    std::string out;
    for (int i = 0; i < 32; ++i) {
        if (before[i] != after[i]) {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "x%d = 0x%08x", i, after[i]);
            if (!out.empty()) out += "\n";
            out += buf;
        }
    }
    return out;
}

std::string dump_registers(const CPU& cpu) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "pc = 0x%08x", cpu.pc);
    std::string out = buf;
    for (int i = 0; i < 32; ++i) {
        std::snprintf(buf, sizeof(buf), "x%-2d = 0x%08x", i, cpu.regs.read(i));
        out += "\n";
        out += buf;
    }
    return out;
}

int run(int argc, char** argv) {
    Args args = parse_args(argc, argv);
    Memory memory = load_binary(args.program, args.base_address);
    CPU cpu(std::move(memory), args.base_address);

    while (cpu.status == Status::Running) {
        if (cpu.step_count >= args.max_steps) {
            cpu.status = Status::Error;
            cpu.halt_reason =
                "exceeded max_steps (" + std::to_string(args.max_steps) + "); possible infinite loop";
            break;
        }

        std::uint32_t pc_before = cpu.pc;
        std::uint32_t raw = cpu.memory.read_word(pc_before);
        Instruction instr = decode(raw);

        if (args.step) {
            char buf[128];
            std::snprintf(buf, sizeof(buf), "pc=0x%08x  instr=0x%08x  ", pc_before, raw);
            std::cout << buf << instr.to_string() << "  [Enter to step] ";
            std::cout.flush();
            std::string line;
            std::getline(std::cin, line);
        } else if (args.trace) {
            char buf[128];
            std::snprintf(buf, sizeof(buf), "pc=0x%08x  instr=0x%08x  ", pc_before, raw);
            std::cout << buf << instr.to_string() << "\n";
        }

        auto before = cpu.regs.snapshot();
        try {
            cpu.step();
        } catch (const std::exception& exc) {
            std::cerr << "error: " << exc.what() << "\n";
            break;
        }
        auto after = cpu.regs.snapshot();

        if (args.trace || args.step) {
            std::string changed = format_registers_changed(before, after);
            if (!changed.empty()) {
                std::cout << "Registers changed:\n" << changed << "\n";
            }
        }
    }

    if (cpu.halt_reason.has_value()) {
        std::cout << "halted: " << *cpu.halt_reason << " (exit code " << cpu.exit_code << ")\n";
    }

    if (args.dump_registers) {
        std::cout << dump_registers(cpu) << "\n";
    }

    return cpu.status == Status::Halted ? 0 : 1;
}

}  // namespace

int main(int argc, char** argv) { return run(argc, argv); }
