#include "doctest.hpp"

#include <array>
#include <cstddef>
#include <span>

#include "parser/format.hpp"

TEST_CASE("format: detects ELF by magic")
{
    const std::array<std::byte, 4> elf{std::byte{0x7F}, std::byte{'E'}, std::byte{'L'},
                                       std::byte{'F'}};
    const std::array<std::byte, 4> other{std::byte{0xFF}, std::byte{'E'}, std::byte{'L'},
                                         std::byte{'F'}};

    CHECK(binhound::detectFormat(elf) == binhound::Format::Elf);
    CHECK(binhound::detectFormat(other) == binhound::Format::Unknown);
    CHECK(binhound::detectFormat(std::span<const std::byte>{}) == binhound::Format::Unknown);
}

TEST_CASE("format: names")
{
    CHECK(binhound::formatName(binhound::Format::Elf) == "ELF");
    CHECK(binhound::formatName(binhound::Format::Unknown) == "unknown");
}
