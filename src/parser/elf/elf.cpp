#include "parser/elf/elf.hpp"

namespace binhound
{

Result<ElfInfo> parseElf(std::span<const std::byte> data)
{
    auto header = parseElfHeader(data);
    if(!header)
    {
        return tl::unexpected(header.error());
    }

    auto sections = parseSections(data, *header);
    if(!sections)
    {
        return tl::unexpected(sections.error());
    }

    auto symbols = parseSymbols(data, *header, *sections);
    if(!symbols)
    {
        return tl::unexpected(symbols.error());
    }

    auto notes = parseNotes(data, *header, *sections);
    if(!notes)
    {
        return tl::unexpected(notes.error());
    }

    ElfInfo info;
    info.header = *header;
    info.sections = std::move(*sections);
    info.symbols = std::move(*symbols);
    info.buildId = findBuildId(*notes);
    return info;
}

bool isDynamic(const ElfInfo& info) noexcept
{
    return findSection(info.sections, ".dynsym") != nullptr ||
           findSection(info.sections, ".dynamic") != nullptr;
}

bool isStripped(const ElfInfo& info) noexcept
{
    return findSection(info.sections, ".symtab") == nullptr;
}

} // namespace binhound
