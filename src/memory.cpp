#include "riscv_sim/memory.hpp"

#include <algorithm>
#include <cstdio>

#include "riscv_sim/errors.hpp"

namespace riscv_sim {

void Memory::check_range(std::uint32_t address, std::size_t length) const {
    if (static_cast<std::size_t>(address) + length > size_) {
        char buf[128];
        std::snprintf(buf, sizeof(buf),
                       "address 0x%08x (length %zu) is out of bounds for memory of size %zu bytes",
                       address, length, size_);
        throw MemoryAccessError(buf);
    }
}

std::vector<std::uint8_t> Memory::read_bytes(std::uint32_t address, std::size_t length) const {
    check_range(address, length);
    return std::vector<std::uint8_t>(data_.begin() + address, data_.begin() + address + length);
}

void Memory::write_bytes(std::uint32_t address, const std::uint8_t* data, std::size_t length) {
    check_range(address, length);
    std::copy(data, data + length, data_.begin() + address);
}

std::int32_t Memory::read_byte(std::uint32_t address, bool signed_) const {
    auto bytes = read_bytes(address, 1);
    std::uint8_t value = bytes[0];
    if (signed_ && (value & 0x80)) {
        return static_cast<std::int32_t>(value) - 256;
    }
    return value;
}

std::int32_t Memory::read_half(std::uint32_t address, bool signed_) const {
    auto bytes = read_bytes(address, 2);
    std::uint32_t value = static_cast<std::uint32_t>(bytes[0]) | (static_cast<std::uint32_t>(bytes[1]) << 8);
    if (signed_ && (value & 0x8000)) {
        return static_cast<std::int32_t>(value) - 65536;
    }
    return static_cast<std::int32_t>(value);
}

std::uint32_t Memory::read_word(std::uint32_t address) const {
    auto bytes = read_bytes(address, 4);
    return static_cast<std::uint32_t>(bytes[0]) | (static_cast<std::uint32_t>(bytes[1]) << 8) |
           (static_cast<std::uint32_t>(bytes[2]) << 16) | (static_cast<std::uint32_t>(bytes[3]) << 24);
}

void Memory::write_byte(std::uint32_t address, std::uint32_t value) {
    std::uint8_t byte = static_cast<std::uint8_t>(value & 0xFF);
    write_bytes(address, &byte, 1);
}

void Memory::write_half(std::uint32_t address, std::uint32_t value) {
    std::uint16_t half = static_cast<std::uint16_t>(value & 0xFFFF);
    std::uint8_t bytes[2] = {static_cast<std::uint8_t>(half & 0xFF), static_cast<std::uint8_t>(half >> 8)};
    write_bytes(address, bytes, 2);
}

void Memory::write_word(std::uint32_t address, std::uint32_t value) {
    std::uint8_t bytes[4] = {
        static_cast<std::uint8_t>(value & 0xFF),
        static_cast<std::uint8_t>((value >> 8) & 0xFF),
        static_cast<std::uint8_t>((value >> 16) & 0xFF),
        static_cast<std::uint8_t>((value >> 24) & 0xFF),
    };
    write_bytes(address, bytes, 4);
}

void Memory::load_bytes(std::uint32_t address, const std::vector<std::uint8_t>& data) {
    write_bytes(address, data.data(), data.size());
}

}  // namespace riscv_sim
