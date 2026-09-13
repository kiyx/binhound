#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "parser/elf/header.hpp"
#include "parser/elf/sections.hpp"
#include "util/error.hpp"

namespace binhound
{

struct Note
{
    std::string name;
    std::uint32_t type = 0;
    std::vector<std::byte> description;
};

// Parses every SHT_NOTE section. Only section notes are read; notes from
// program headers (PT_NOTE) need program header parsing, a later milestone.
[[nodiscard]] Result<std::vector<Note>> parseNotes(std::span<const std::byte> data,
                                                   const ElfHeader& header,
                                                   const std::vector<Section>& sections);

// Hex Build-ID of the first GNU note of type NT_GNU_BUILD_ID with a
// non-empty description; nullopt when absent.
[[nodiscard]] std::optional<std::string> findBuildId(const std::vector<Note>& notes);

} // namespace binhound
