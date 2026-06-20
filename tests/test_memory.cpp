#include "riscv_sim/memory.hpp"

#include "riscv_sim/errors.hpp"
#include "test_framework.hpp"

using namespace riscv_sim;

TEST_CASE("word_read_write_round_trip") {
    Memory mem(256);
    mem.write_word(0, 0x12345678);
    CHECK(mem.read_word(0) == 0x12345678u);
}

TEST_CASE("little_endian_byte_order") {
    Memory mem(256);
    mem.write_word(0, 0x12345678);
    auto bytes = mem.read_bytes(0, 4);
    CHECK(bytes[0] == 0x78);
    CHECK(bytes[1] == 0x56);
    CHECK(bytes[2] == 0x34);
    CHECK(bytes[3] == 0x12);
}

TEST_CASE("byte_and_half_round_trip") {
    Memory mem(256);
    mem.write_byte(4, 0xFF);
    CHECK(mem.read_byte(4, /*signed_=*/false) == 0xFF);
    CHECK(mem.read_byte(4, /*signed_=*/true) == -1);

    mem.write_half(8, 0x8000);
    CHECK(mem.read_half(8, /*signed_=*/false) == 0x8000);
    CHECK(mem.read_half(8, /*signed_=*/true) == -32768);
}

TEST_CASE("out_of_bounds_access_raises") {
    Memory mem(16);
    CHECK_THROWS_AS(mem.read_word(16), MemoryAccessError);
    CHECK_THROWS_AS(mem.write_byte(static_cast<std::uint32_t>(-1), 0), MemoryAccessError);
}

TEST_CASE("load_bytes_places_program_image") {
    Memory mem(64);
    mem.load_bytes(0, {0x01, 0x02, 0x03, 0x04});
    CHECK(mem.read_word(0) == 0x04030201u);
}
