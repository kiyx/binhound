#include "parser/elf/symbols.hpp"

#include "util/bytes.hpp"

// Layouts, see man7 elf(5): st_info packs binding (high nibble) and type
// (low nibble), like the ELF64_ST_BIND / ELF64_ST_TYPE macros.
namespace binhound
{

namespace
{

constexpr std::uint32_t kSymbolTableType = 2;     // SHT_SYMTAB
constexpr std::uint32_t kDynSymbolTableType = 11; // SHT_DYNSYM

constexpr std::size_t kSym32Size = 16;
constexpr std::size_t kSym64Size = 24;

constexpr std::size_t kSym32Name = 0;
constexpr std::size_t kSym32Value = 4;
constexpr std::size_t kSym32SizeOff = 8;
constexpr std::size_t kSym32Info = 12;
constexpr std::size_t kSym32Shndx = 14;

constexpr std::size_t kSym64Name = 0;
constexpr std::size_t kSym64Info = 4;
constexpr std::size_t kSym64Shndx = 6;
constexpr std::size_t kSym64Value = 8;
constexpr std::size_t kSym64SizeOff = 16;

Result<std::string> resolveName(std::span<const std::byte> strings, std::uint32_t offset)
{
    const auto name = readCString(strings, offset);
    if(!name)
    {
        return tl::unexpected(
            makeError(Error::Code::BadSymbol, "symbol name outside the string table"));
    }
    return std::string(*name);
}

} // namespace

Result<void> parseTable(std::span<const std::byte> data, const ElfHeader& header,
                        const Section& section, const std::vector<Section>& sections,
                        std::vector<Symbol>& symbols)
{
    const std::size_t entrySize = header.is64Bit ? kSym64Size : kSym32Size;
    if(section.entrySize != entrySize)
    {
        return tl::unexpected(makeError(Error::Code::BadSymbol, "unexpected symbol entry size"));
    }
    if(section.size % entrySize != 0)
    {
        return tl::unexpected(
            makeError(Error::Code::BadSymbol, "symbol table size is not a multiple of entries"));
    }
    if(section.link >= sections.size())
    {
        return tl::unexpected(
            makeError(Error::Code::BadSymbol, "symbol table references a missing string table"));
    }
    const Section& stringsSection = sections[section.link];
    if(!fitsInFile(data, stringsSection.offset, stringsSection.size))
    {
        return tl::unexpected(makeError(Error::Code::BadSymbol, "string table outside the file"));
    }
    if(!fitsInFile(data, section.offset, section.size))
    {
        return tl::unexpected(makeError(Error::Code::BadSymbol, "symbol table outside the file"));
    }
    const auto strings = data.subspan(static_cast<std::size_t>(stringsSection.offset),
                                      static_cast<std::size_t>(stringsSection.size));

    const std::uint64_t count = section.size / static_cast<std::uint64_t>(entrySize);
    for(std::uint64_t i = 0; i < count; ++i)
    {
        const auto at =
            static_cast<std::size_t>(section.offset + i * static_cast<std::uint64_t>(entrySize));
        Symbol symbol;
        if(header.is64Bit)
        {
            auto name = resolveName(strings, readU32Or(data, at + kSym64Name, header.endian));
            if(!name)
            {
                return tl::unexpected(name.error());
            }
            symbol.name = std::move(*name);
            const auto info = static_cast<std::uint8_t>(data[at + kSym64Info]);
            symbol.binding = static_cast<std::uint8_t>(info >> 4U);
            symbol.type = static_cast<std::uint8_t>(info & 0x0FU);
            symbol.sectionIndex = readU16Or(data, at + kSym64Shndx, header.endian);
            symbol.value = readU64Or(data, at + kSym64Value, header.endian);
            symbol.size = readU64Or(data, at + kSym64SizeOff, header.endian);
        }
        else
        {
            auto name = resolveName(strings, readU32Or(data, at + kSym32Name, header.endian));
            if(!name)
            {
                return tl::unexpected(name.error());
            }
            symbol.name = std::move(*name);
            const auto info = static_cast<std::uint8_t>(data[at + kSym32Info]);
            symbol.binding = static_cast<std::uint8_t>(info >> 4U);
            symbol.type = static_cast<std::uint8_t>(info & 0x0FU);
            symbol.sectionIndex = readU16Or(data, at + kSym32Shndx, header.endian);
            symbol.value = readU32Or(data, at + kSym32Value, header.endian);
            symbol.size = readU32Or(data, at + kSym32SizeOff, header.endian);
        }
        symbols.push_back(std::move(symbol));
    }
    return {};
}

Result<std::vector<Symbol>> parseSymbols(std::span<const std::byte> data, const ElfHeader& header,
                                         const std::vector<Section>& sections)
{
    std::vector<Symbol> symbols;
    for(const auto& section : sections)
    {
        if(section.type != kSymbolTableType && section.type != kDynSymbolTableType)
        {
            continue;
        }
        const auto table = parseTable(data, header, section, sections, symbols);
        if(!table)
        {
            return tl::unexpected(table.error());
        }
    }
    return symbols;
}

} // namespace binhound
