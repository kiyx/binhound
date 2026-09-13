#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "parser/elf/header.hpp"
#include "parser/elf/sections.hpp"
#include "util/error.hpp"

namespace binhound
{

struct Symbol
{
    std::string name;
    std::uint64_t value = 0;
    std::uint64_t size = 0;
    std::uint8_t type = 0;
    std::uint8_t binding = 0;
    std::uint16_t sectionIndex = 0;
};

// Parses every SHT_SYMTAB and SHT_DYNSYM table, resolving names through each
// table's linked string table (sh_link). Missing tables yield an empty vector,
// not an error.
[[nodiscard]] Result<std::vector<Symbol>> parseSymbols(std::span<const std::byte> data,
                                                       const ElfHeader& header,
                                                       const std::vector<Section>& sections);

} // namespace binhound
