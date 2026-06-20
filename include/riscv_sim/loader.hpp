// Loads program images (raw machine code) into simulator memory.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "riscv_sim/cpu.hpp"
#include "riscv_sim/memory.hpp"

namespace riscv_sim {

// Load a flat binary file of machine code into memory at `base_address`.
Memory load_binary(const std::string& path, std::uint32_t base_address = 0);

// Load a list of 32-bit instruction words into memory at `base_address`.
Memory load_words(const std::vector<std::uint32_t>& words, std::uint32_t base_address = 0);

// Build a ready-to-run CPU with a binary program loaded at `base_address`.
CPU cpu_from_binary(const std::string& path, std::uint32_t base_address = 0);

}  // namespace riscv_sim
