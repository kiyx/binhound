#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

#include "util/bytes.hpp"
#include "util/error.hpp"

namespace binhound
{

struct ElfHeader
{
    bool is64Bit = false;
    Endian endian = Endian::Little;
    std::uint16_t type = 0;
    std::uint16_t machine = 0;
    std::uint64_t entry = 0;
    std::uint64_t programHeaderOffset = 0;
    std::uint64_t sectionHeaderOffset = 0;
    std::uint16_t programHeaderCount = 0;
    std::uint16_t sectionHeaderCount = 0;
    std::uint16_t sectionNameIndex = 0;
    std::uint16_t headerSize = 0;
    // Raw e_shentsize: entry size of the section header table (40 or 64).
    // parseSections validates it before reading the table.
    std::uint16_t sectionHeaderEntrySize = 0;
};

[[nodiscard]] Result<ElfHeader> parseElfHeader(std::span<const std::byte> data);

[[nodiscard]] std::string_view machineName(std::uint16_t machine) noexcept;

[[nodiscard]] std::string_view typeName(std::uint16_t type) noexcept;

} // namespace binhound
