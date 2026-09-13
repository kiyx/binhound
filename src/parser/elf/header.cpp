#include "parser/elf/header.hpp"

#include <optional>

#include "util/reader.hpp"

namespace binhound
{

namespace
{

constexpr std::size_t kIdentSize = 16;
constexpr std::size_t kIdentClass = 4;
constexpr std::size_t kIdentData = 5;

constexpr std::uint8_t kElfClass32 = 1;
constexpr std::uint8_t kElfClass64 = 2;
constexpr std::uint8_t kElfDataLittle = 1;
constexpr std::uint8_t kElfDataBig = 2;

constexpr std::size_t kElf32HeaderSize = 52;
constexpr std::size_t kElf32Type = 16;
constexpr std::size_t kElf32Machine = 18;
constexpr std::size_t kElf32Entry = 24;
constexpr std::size_t kElf32PhOffset = 28;
constexpr std::size_t kElf32ShOffset = 32;
constexpr std::size_t kElf32EhSize = 40;
constexpr std::size_t kElf32PhCount = 44;
constexpr std::size_t kElf32ShCount = 48;
constexpr std::size_t kElf32ShStrIndex = 50;

constexpr std::size_t kElf64HeaderSize = 64;
constexpr std::size_t kElf64Type = 16;
constexpr std::size_t kElf64Machine = 18;
constexpr std::size_t kElf64Entry = 24;
constexpr std::size_t kElf64PhOffset = 32;
constexpr std::size_t kElf64ShOffset = 40;
constexpr std::size_t kElf64EhSize = 52;
constexpr std::size_t kElf64PhCount = 56;
constexpr std::size_t kElf64ShCount = 60;
constexpr std::size_t kElf64ShStrIndex = 62;

Error makeError(Error::Code code, std::string message)
{
    return Error{code, std::move(message)};
}

std::optional<Endian> endianFromByte(std::uint8_t value) noexcept
{
    if(value == kElfDataLittle)
    {
        return Endian::Little;
    }
    if(value == kElfDataBig)
    {
        return Endian::Big;
    }
    return std::nullopt;
}

std::uint16_t u16At(std::span<const std::byte> data, std::size_t offset, Endian endian) noexcept
{
    return readU16(data, offset, endian).value_or(0);
}

std::uint32_t u32At(std::span<const std::byte> data, std::size_t offset, Endian endian) noexcept
{
    return readU32(data, offset, endian).value_or(0);
}

std::uint64_t u64At(std::span<const std::byte> data, std::size_t offset, Endian endian) noexcept
{
    return readU64(data, offset, endian).value_or(0);
}

Result<ElfHeader> parseElf32(std::span<const std::byte> data, Endian endian)
{
    if(data.size() < kElf32HeaderSize)
    {
        return tl::unexpected(makeError(Error::Code::Truncated, "ELF32 header is truncated"));
    }

    return ElfHeader{
        .is64Bit = false,
        .endian = endian,
        .type = u16At(data, kElf32Type, endian),
        .machine = u16At(data, kElf32Machine, endian),
        .entry = u32At(data, kElf32Entry, endian),
        .programHeaderOffset = u32At(data, kElf32PhOffset, endian),
        .sectionHeaderOffset = u32At(data, kElf32ShOffset, endian),
        .programHeaderCount = u16At(data, kElf32PhCount, endian),
        .sectionHeaderCount = u16At(data, kElf32ShCount, endian),
        .sectionNameIndex = u16At(data, kElf32ShStrIndex, endian),
        .headerSize = u16At(data, kElf32EhSize, endian),
    };
}

Result<ElfHeader> parseElf64(std::span<const std::byte> data, Endian endian)
{
    if(data.size() < kElf64HeaderSize)
    {
        return tl::unexpected(makeError(Error::Code::Truncated, "ELF64 header is truncated"));
    }

    return ElfHeader{
        .is64Bit = true,
        .endian = endian,
        .type = u16At(data, kElf64Type, endian),
        .machine = u16At(data, kElf64Machine, endian),
        .entry = u64At(data, kElf64Entry, endian),
        .programHeaderOffset = u64At(data, kElf64PhOffset, endian),
        .sectionHeaderOffset = u64At(data, kElf64ShOffset, endian),
        .programHeaderCount = u16At(data, kElf64PhCount, endian),
        .sectionHeaderCount = u16At(data, kElf64ShCount, endian),
        .sectionNameIndex = u16At(data, kElf64ShStrIndex, endian),
        .headerSize = u16At(data, kElf64EhSize, endian),
    };
}

} // namespace

Result<ElfHeader> parseElfHeader(std::span<const std::byte> data)
{
    if(!hasElfMagic(data))
    {
        return tl::unexpected(makeError(Error::Code::NotElf, "missing ELF magic"));
    }

    if(data.size() < kIdentSize)
    {
        return tl::unexpected(
            makeError(Error::Code::Truncated, "file smaller than the ELF identification"));
    }

    const auto endian = endianFromByte(std::to_integer<std::uint8_t>(data[kIdentData]));
    if(!endian)
    {
        return tl::unexpected(makeError(Error::Code::BadEndianness, "unknown EI_DATA value"));
    }

    switch(std::to_integer<std::uint8_t>(data[kIdentClass]))
    {
    case kElfClass32:
        return parseElf32(data, *endian);
    case kElfClass64:
        return parseElf64(data, *endian);
    default:
        return tl::unexpected(
            makeError(Error::Code::UnsupportedClass, "unsupported EI_CLASS value"));
    }
}

std::string_view machineName(std::uint16_t machine) noexcept
{
    switch(machine)
    {
    case 0x03:
        return "x86";
    case 0x3E:
        return "x86-64";
    case 0x28:
        return "ARM";
    case 0xB7:
        return "AArch64";
    case 0x08:
        return "MIPS";
    case 0x14:
        return "PowerPC";
    case 0xF3:
        return "RISC-V";
    default:
        return "unknown";
    }
}

std::string_view typeName(std::uint16_t type) noexcept
{
    switch(type)
    {
    case 0:
        return "NONE";
    case 1:
        return "REL";
    case 2:
        return "EXEC";
    case 3:
        return "DYN";
    case 4:
        return "CORE";
    default:
        return "unknown";
    }
}

} // namespace binhound
