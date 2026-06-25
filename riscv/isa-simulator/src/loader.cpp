#include "riscv_sim/loader.hpp"

#include <fstream>
#include <stdexcept>

namespace riscv_sim {

namespace {
std::vector<std::uint8_t> read_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("could not open file: " + path);
    }
    std::streamsize length = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> data(static_cast<std::size_t>(length));
    file.read(reinterpret_cast<char*>(data.data()), length);
    return data;
}
}  // namespace

Memory load_binary(const std::string& path, std::uint32_t base_address) {
    std::vector<std::uint8_t> data = read_file(path);
    Memory memory;
    memory.load_bytes(base_address, data);
    return memory;
}

Memory load_words(const std::vector<std::uint32_t>& words, std::uint32_t base_address) {
    std::vector<std::uint8_t> data;
    data.reserve(words.size() * 4);
    for (std::uint32_t word : words) {
        data.push_back(static_cast<std::uint8_t>(word & 0xFF));
        data.push_back(static_cast<std::uint8_t>((word >> 8) & 0xFF));
        data.push_back(static_cast<std::uint8_t>((word >> 16) & 0xFF));
        data.push_back(static_cast<std::uint8_t>((word >> 24) & 0xFF));
    }
    Memory memory;
    memory.load_bytes(base_address, data);
    return memory;
}

CPU cpu_from_binary(const std::string& path, std::uint32_t base_address) {
    Memory memory = load_binary(path, base_address);
    return CPU(std::move(memory), base_address);
}

}  // namespace riscv_sim
