// Builds the flat-binary example programs under examples/.
//
// Run with: ./build_examples (from the build directory), or
// ./build/build_examples from the repo root.
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "riscv_sim/encoder.hpp"

namespace e = riscv_sim::encoder;
namespace fs = std::filesystem;

namespace {

const fs::path kExamplesDir = fs::path(__FILE__).parent_path().parent_path() / "examples";

void write_program(const std::string& rel_path, const std::vector<std::string>& source_lines,
                    const std::vector<std::uint32_t>& words) {
    fs::path out_path = kExamplesDir / rel_path;
    fs::create_directories(out_path.parent_path());

    std::ofstream bin_file(out_path, std::ios::binary);
    for (std::uint32_t word : words) {
        std::uint8_t bytes[4] = {
            static_cast<std::uint8_t>(word & 0xFF),
            static_cast<std::uint8_t>((word >> 8) & 0xFF),
            static_cast<std::uint8_t>((word >> 16) & 0xFF),
            static_cast<std::uint8_t>((word >> 24) & 0xFF),
        };
        bin_file.write(reinterpret_cast<const char*>(bytes), 4);
    }
    bin_file.close();

    fs::path listing_path = out_path;
    listing_path.replace_extension(".s");
    std::ofstream listing_file(listing_path);
    listing_file << "# " << rel_path << " -- assembly listing (for reference; not assembled by the simulator)\n";
    for (std::size_t i = 0; i < source_lines.size() && i < words.size(); ++i) {
        char addr_buf[16];
        std::snprintf(addr_buf, sizeof(addr_buf), "0x%04x", static_cast<unsigned>(i * 4));
        listing_file << "# " << addr_buf << ": " << source_lines[i] << "\n";
    }
    listing_file.close();

    std::printf("wrote examples/%s (%zu instructions)\n", rel_path.c_str(), words.size());
}

void build_arithmetic() {
    std::vector<std::string> source = {
        "addi x1, x0, 5",
        "addi x2, x0, 10",
        "add  x3, x1, x2",
        "ebreak",
    };
    std::vector<std::uint32_t> words = {
        e::addi(1, 0, 5),
        e::addi(2, 0, 10),
        e::add(3, 1, 2),
        e::ebreak(),
    };
    write_program("arithmetic/addi.bin", source, words);
}

void build_branches() {
    // Sums 1..5 into x2 using a countdown loop in x1.
    std::vector<std::string> source = {
        "addi x1, x0, 5      # counter",
        "addi x2, x0, 0      # sum",
        "add  x2, x2, x1     # loop:",
        "addi x1, x1, -1",
        "bne  x1, x0, loop",
        "ebreak",
    };
    std::vector<std::uint32_t> words = {
        e::addi(1, 0, 5),
        e::addi(2, 0, 0),
        e::add(2, 2, 1),
        e::addi(1, 1, -1),
        e::bne(1, 0, -8),
        e::ebreak(),
    };
    write_program("branches/loop.bin", source, words);
}

void build_memory() {
    std::vector<std::string> source = {
        "addi x1, x0, 100    # value",
        "addi x2, x0, 64     # address",
        "sw   x1, 0(x2)",
        "lw   x3, 0(x2)",
        "ebreak",
    };
    std::vector<std::uint32_t> words = {
        e::addi(1, 0, 100),
        e::addi(2, 0, 64),
        e::sw(2, 1, 0),
        e::lw(3, 2, 0),
        e::ebreak(),
    };
    write_program("memory/load_store.bin", source, words);
}

}  // namespace

int main() {
    build_arithmetic();
    build_branches();
    build_memory();
    return 0;
}
