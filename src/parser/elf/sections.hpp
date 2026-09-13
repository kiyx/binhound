#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "parser/elf/header.hpp"
#include "util/error.hpp"

namespace binhound
{

struct Section
{
    std::string name;
    std::uint32_t type = 0;
    std::uint64_t offset = 0;
    std::uint64_t size = 0;
    std::uint64_t flags = 0;
    // Index of the linked section (e.g. the string table for symbol tables).
    std::uint32_t link = 0;
    // Size of one table entry (e.g. for symbol tables), zero otherwise.
    std::uint64_t entrySize = 0;
};

// Parses the section header table. A file without a table (e_shoff == 0)
// yields an empty vector, not an error. Extended numbering (e_shnum == 0,
// e_shstrndx == SHN_XINDEX) is resolved per man7 elf(5).
[[nodiscard]] Result<std::vector<Section>> parseSections(std::span<const std::byte> data,
                                                         const ElfHeader& header);

// Linear search by name; the returned pointer is non-owning (nullptr if absent).
[[nodiscard]] const Section* findSection(const std::vector<Section>& sections,
                                         std::string_view name) noexcept;

} // namespace binhound
