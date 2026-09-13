#include "doctest.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

#include "util/bytes.hpp"

using binhound::Endian;

TEST_CASE("bytes: u16 in both endiannesses")
{
    const std::array<std::byte, 2> data{std::byte{0x01}, std::byte{0x02}};

    CHECK(binhound::readU16(data, 0, Endian::Little) == 0x0201);
    CHECK(binhound::readU16(data, 0, Endian::Big) == 0x0102);
}

TEST_CASE("bytes: u32 in both endiannesses")
{
    const std::array<std::byte, 4> data{std::byte{0x01}, std::byte{0x02}, std::byte{0x03},
                                        std::byte{0x04}};

    CHECK(binhound::readU32(data, 0, Endian::Little) == 0x04030201);
    CHECK(binhound::readU32(data, 0, Endian::Big) == 0x01020304);
}

TEST_CASE("bytes: u64 in both endiannesses")
{
    const std::array<std::byte, 8> data{std::byte{0x01}, std::byte{0x02}, std::byte{0x03},
                                        std::byte{0x04}, std::byte{0x05}, std::byte{0x06},
                                        std::byte{0x07}, std::byte{0x08}};

    CHECK(binhound::readU64(data, 0, Endian::Little) == 0x0807060504030201ULL);
    CHECK(binhound::readU64(data, 0, Endian::Big) == 0x0102030405060708ULL);
}

TEST_CASE("bytes: out of bounds reads")
{
    const std::array<std::byte, 4> data{};

    CHECK_FALSE(binhound::readU16(data, 3, Endian::Little).has_value());
    CHECK_FALSE(binhound::readU32(data, 1, Endian::Little).has_value());
    CHECK_FALSE(binhound::readU64(data, 0, Endian::Little).has_value());
    CHECK(binhound::readU32(data, 0, Endian::Little).has_value());
}

TEST_CASE("bytes: toHex")
{
    const std::array<std::byte, 4> data{std::byte{0xDE}, std::byte{0xAD}, std::byte{0xBE},
                                        std::byte{0xEF}};

    CHECK(binhound::toHex(data) == "deadbeef");
    CHECK(binhound::toHex(std::span<const std::byte>{}).empty());
}
