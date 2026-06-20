// A simple byte-addressable, little-endian memory model.
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace riscv_sim {

// Fixed-size byte-addressable memory with little-endian word access.
class Memory {
public:
    explicit Memory(std::size_t size = 1u << 20) : size_(size), data_(size, 0) {}

    std::size_t size() const { return size_; }

    std::vector<std::uint8_t> read_bytes(std::uint32_t address, std::size_t length) const;
    void write_bytes(std::uint32_t address, const std::uint8_t* data, std::size_t length);

    std::int32_t read_byte(std::uint32_t address, bool signed_ = false) const;
    std::int32_t read_half(std::uint32_t address, bool signed_ = false) const;
    std::uint32_t read_word(std::uint32_t address) const;

    void write_byte(std::uint32_t address, std::uint32_t value);
    void write_half(std::uint32_t address, std::uint32_t value);
    void write_word(std::uint32_t address, std::uint32_t value);

    // Load a block of raw bytes (e.g. a program image) into memory.
    void load_bytes(std::uint32_t address, const std::vector<std::uint8_t>& data);

private:
    void check_range(std::uint32_t address, std::size_t length) const;

    std::size_t size_;
    std::vector<std::uint8_t> data_;
};

}  // namespace riscv_sim
