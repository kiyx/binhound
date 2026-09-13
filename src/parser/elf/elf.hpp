#pragma once

#include <optional>
#include <span>
#include <string>
#include <vector>

#include "parser/elf/header.hpp"
#include "parser/elf/notes.hpp"
#include "parser/elf/sections.hpp"
#include "parser/elf/symbols.hpp"
#include "util/error.hpp"

namespace binhound
{

struct ElfInfo
{
    ElfHeader header;
    std::vector<Section> sections;
    std::vector<Symbol> symbols;
    std::optional<std::string> buildId;
};

// Parses header, sections, symbols and notes. Pure orchestration: every
// failure is propagated unchanged from the stage that reported it.
[[nodiscard]] Result<ElfInfo> parseElf(std::span<const std::byte> data);

// True when the file can be dynamically linked (a .dynsym or .dynamic
// section is present).
[[nodiscard]] bool isDynamic(const ElfInfo& info) noexcept;

// True when no static symbol table is present (no .symtab section).
[[nodiscard]] bool isStripped(const ElfInfo& info) noexcept;

} // namespace binhound
