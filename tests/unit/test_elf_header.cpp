#include "doctest.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "parser/elf/header.hpp"

using binhound::ElfHeader;
using binhound::Endian;
using binhound::Error;

namespace
{

constexpr std::size_t kElf64Size = 64;
constexpr std::size_t kElf32Size = 52;

void put16(std::span<std::byte> data, std::size_t offset, std::uint16_t value, bool little)
{
    if(little)
    {
        data[offset] = static_cast<std::byte>(value & 0xFFU);
        data[offset + 1] = static_cast<std::byte>(value >> 8U);
    }
    else
    {
        data[offset] = static_cast<std::byte>(value >> 8U);
        data[offset + 1] = static_cast<std::byte>(value & 0xFFU);
    }
}

void put32(std::span<std::byte> data, std::size_t offset, std::uint32_t value, bool little)
{
    if(little)
    {
        data[offset] = static_cast<std::byte>(value & 0xFFU);
        data[offset + 1] = static_cast<std::byte>((value >> 8U) & 0xFFU);
        data[offset + 2] = static_cast<std::byte>((value >> 16U) & 0xFFU);
        data[offset + 3] = static_cast<std::byte>((value >> 24U) & 0xFFU);
    }
    else
    {
        data[offset] = static_cast<std::byte>((value >> 24U) & 0xFFU);
        data[offset + 1] = static_cast<std::byte>((value >> 16U) & 0xFFU);
        data[offset + 2] = static_cast<std::byte>((value >> 8U) & 0xFFU);
        data[offset + 3] = static_cast<std::byte>(value & 0xFFU);
    }
}

void put64(std::span<std::byte> data, std::size_t offset, std::uint64_t value, bool little)
{
    for(std::size_t i = 0; i < 8; ++i)
    {
        const auto shift = static_cast<unsigned>(8U * (little ? i : 7U - i));
        data[offset + i] = static_cast<std::byte>((value >> shift) & 0xFFU);
    }
}

std::array<std::byte, kElf64Size> makeElf64(bool little)
{
    std::array<std::byte, kElf64Size> data{};
    data[0] = std::byte{0x7F};
    data[1] = std::byte{'E'};
    data[2] = std::byte{'L'};
    data[3] = std::byte{'F'};
    data[4] = std::byte{2};
    data[5] = little ? std::byte{1} : std::byte{2};

    put16(data, 16, 3, little);
    put16(data, 18, 0x3E, little);
    put64(data, 24, 0x401000, little);
    put64(data, 32, 64, little);
    put64(data, 40, 0x22428, little);
    put16(data, 52, 64, little);
    put16(data, 56, 13, little);
    put16(data, 58, 64, little);
    put16(data, 60, 31, little);
    put16(data, 62, 30, little);
    return data;
}

std::array<std::byte, kElf32Size> makeElf32(bool little)
{
    std::array<std::byte, kElf32Size> data{};
    data[0] = std::byte{0x7F};
    data[1] = std::byte{'E'};
    data[2] = std::byte{'L'};
    data[3] = std::byte{'F'};
    data[4] = std::byte{1};
    data[5] = little ? std::byte{1} : std::byte{2};

    put16(data, 16, 2, little);
    put16(data, 18, 0x28, little);
    put32(data, 24, 0x8000, little);
    put32(data, 28, 52, little);
    put32(data, 32, 0x2000, little);
    put16(data, 40, 52, little);
    put16(data, 44, 8, little);
    put16(data, 46, 40, little);
    put16(data, 48, 25, little);
    put16(data, 50, 24, little);
    return data;
}

} // namespace

TEST_CASE("elf header: parses ELF64 little endian")
{
    const auto data = makeElf64(true);
    const auto header = binhound::parseElfHeader(data);

    REQUIRE(header.has_value());
    CHECK(header->is64Bit);
    CHECK(header->endian == Endian::Little);
    CHECK(header->type == 3);
    CHECK(header->machine == 0x3E);
    CHECK(header->entry == 0x401000);
    CHECK(header->programHeaderOffset == 64);
    CHECK(header->sectionHeaderOffset == 0x22428);
    CHECK(header->programHeaderCount == 13);
    CHECK(header->sectionHeaderCount == 31);
    CHECK(header->sectionNameIndex == 30);
    CHECK(header->headerSize == 64);
    CHECK(header->sectionHeaderEntrySize == 64);
}

TEST_CASE("elf header: parses ELF64 big endian")
{
    const auto data = makeElf64(false);
    const auto header = binhound::parseElfHeader(data);

    REQUIRE(header.has_value());
    CHECK(header->endian == Endian::Big);
    CHECK(header->entry == 0x401000);
    CHECK(binhound::machineName(header->machine) == "x86-64");
}

TEST_CASE("elf header: parses ELF32")
{
    const auto data = makeElf32(true);
    const auto header = binhound::parseElfHeader(data);

    REQUIRE(header.has_value());
    CHECK_FALSE(header->is64Bit);
    CHECK(header->entry == 0x8000);
    CHECK(header->programHeaderCount == 8);
    CHECK(header->sectionHeaderCount == 25);
    CHECK(header->sectionHeaderOffset == 0x2000);
    CHECK(header->headerSize == 52);
    CHECK(header->sectionHeaderEntrySize == 40);
    CHECK(binhound::machineName(header->machine) == "ARM");
    CHECK(binhound::typeName(header->type) == "EXEC");
}

TEST_CASE("elf header: rejects bad inputs")
{
    const auto good = makeElf64(true);

    CHECK(binhound::parseElfHeader(std::span<const std::byte>(good).first(10)).error().code ==
          Error::Code::Truncated);

    auto notElf = good;
    notElf[0] = std::byte{0x00};
    CHECK(binhound::parseElfHeader(notElf).error().code == Error::Code::NotElf);

    auto badClass = good;
    badClass[4] = std::byte{3};
    CHECK(binhound::parseElfHeader(badClass).error().code == Error::Code::UnsupportedClass);

    auto badEndian = good;
    badEndian[5] = std::byte{3};
    CHECK(binhound::parseElfHeader(badEndian).error().code == Error::Code::BadEndianness);

    CHECK(binhound::parseElfHeader(std::span<const std::byte>(good).first(20)).error().code ==
          Error::Code::Truncated);
}

TEST_CASE("elf header: rejects short non-elf files")
{
    const std::array<std::byte, 10> text{};
    CHECK(binhound::parseElfHeader(text).error().code == Error::Code::NotElf);
}

TEST_CASE("elf header: names")
{
    CHECK(binhound::machineName(0x9999) == "unknown");
    CHECK(binhound::typeName(0x9999) == "unknown");
    CHECK(binhound::machineName(0xB7) == "AArch64");
}

TEST_CASE("elf header: parses ELF32 big endian")
{
    const auto data = makeElf32(false);
    const auto header = binhound::parseElfHeader(data);

    REQUIRE(header.has_value());
    CHECK_FALSE(header->is64Bit);
    CHECK(header->endian == Endian::Big);
    CHECK(header->entry == 0x8000);
    CHECK(binhound::machineName(header->machine) == "ARM");
}

TEST_CASE("elf header: truncated ELF64 header")
{
    const auto data = makeElf64(true);

    CHECK(binhound::parseElfHeader(std::span<const std::byte>(data).first(63)).error().code ==
          Error::Code::Truncated);
}

TEST_CASE("elf header: truncated ELF32 header")
{
    const auto data = makeElf32(true);

    CHECK(binhound::parseElfHeader(std::span<const std::byte>(data).first(51)).error().code ==
          Error::Code::Truncated);
    CHECK(binhound::parseElfHeader(std::span<const std::byte>(data).first(20)).error().code ==
          Error::Code::Truncated);
}

TEST_CASE("elf header: all machine and type names")
{
    CHECK(binhound::machineName(0x03) == "x86");
    CHECK(binhound::machineName(0x28) == "ARM");
    CHECK(binhound::machineName(0x08) == "MIPS");
    CHECK(binhound::machineName(0x14) == "PowerPC");
    CHECK(binhound::machineName(0xF3) == "RISC-V");
    CHECK(binhound::typeName(0) == "NONE");
    CHECK(binhound::typeName(1) == "REL");
    CHECK(binhound::typeName(2) == "EXEC");
    CHECK(binhound::typeName(3) == "DYN");
    CHECK(binhound::typeName(4) == "CORE");
}
