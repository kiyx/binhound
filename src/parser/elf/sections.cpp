#include "parser/elf/sections.hpp"

#include "util/bytes.hpp"

// Layouts, types and special indices, see man7 elf(5).
namespace binhound
{

namespace
{

constexpr std::uint16_t kSectionIndexUndefined = 0;     // SHN_UNDEF
constexpr std::uint16_t kSectionIndexExtended = 0xFFFF; // SHN_XINDEX

constexpr std::size_t kShdr32Size = 40;
constexpr std::size_t kShdr64Size = 64;

constexpr std::size_t kSh32Name = 0;
constexpr std::size_t kSh32Type = 4;
constexpr std::size_t kSh32Flags = 8;
constexpr std::size_t kSh32Offset = 16;
constexpr std::size_t kSh32Size = 20;
constexpr std::size_t kSh32Link = 24;
constexpr std::size_t kSh32EntrySize = 36;

constexpr std::size_t kSh64Name = 0;
constexpr std::size_t kSh64Type = 4;
constexpr std::size_t kSh64Flags = 8;
constexpr std::size_t kSh64Offset = 24;
constexpr std::size_t kSh64Size = 32;
constexpr std::size_t kSh64Link = 40;
constexpr std::size_t kSh64EntrySize = 56;

struct RawSection
{
    std::uint32_t name = 0;
    std::uint32_t type = 0;
    std::uint64_t flags = 0;
    std::uint64_t offset = 0;
    std::uint64_t size = 0;
    std::uint32_t link = 0;
    std::uint64_t entrySize = 0;
};

// Reads one entry at index. Precondition: the caller validated that the whole
// table (count * entry size) fits in the file, so these reads cannot fail.
RawSection readEntry(std::span<const std::byte> table, std::size_t index, bool is64Bit,
                     Endian endian) noexcept
{
    const std::size_t entrySize = is64Bit ? kShdr64Size : kShdr32Size;
    const std::size_t at = index * entrySize;
    if(is64Bit)
    {
        return RawSection{readU32Or(table, at + kSh64Name, endian),
                          readU32Or(table, at + kSh64Type, endian),
                          readU64Or(table, at + kSh64Flags, endian),
                          readU64Or(table, at + kSh64Offset, endian),
                          readU64Or(table, at + kSh64Size, endian),
                          readU32Or(table, at + kSh64Link, endian),
                          readU64Or(table, at + kSh64EntrySize, endian)};
    }
    return RawSection{
        readU32Or(table, at + kSh32Name, endian),     readU32Or(table, at + kSh32Type, endian),
        readU32Or(table, at + kSh32Flags, endian),    readU32Or(table, at + kSh32Offset, endian),
        readU32Or(table, at + kSh32Size, endian),     readU32Or(table, at + kSh32Link, endian),
        readU32Or(table, at + kSh32EntrySize, endian)};
}

} // namespace
Result<std::vector<Section>> parseSections(std::span<const std::byte> data, const ElfHeader& header)
{
    if(header.sectionHeaderOffset == 0)
    {
        return std::vector<Section>{};
    }

    const std::size_t entrySize = header.is64Bit ? kShdr64Size : kShdr32Size;
    if(header.sectionHeaderEntrySize != entrySize)
    {
        return tl::unexpected(
            makeError(Error::Code::BadSection, "unexpected section header entry size"));
    }

    // The table must at least hold the initial entry: extended numbering
    // reads the real section count from it.
    if(!fitsInFile(data, header.sectionHeaderOffset, entrySize))
    {
        return tl::unexpected(
            makeError(Error::Code::BadSection, "section header table outside the file"));
    }

    std::uint64_t count = header.sectionHeaderCount;
    if(count == 0)
    {
        const auto table =
            data.subspan(static_cast<std::size_t>(header.sectionHeaderOffset), entrySize);
        count = readEntry(table, 0, header.is64Bit, header.endian).size;
        if(count == 0)
        {
            return std::vector<Section>{};
        }
    }

    const std::uint64_t tableSize = count * static_cast<std::uint64_t>(entrySize);
    if(!fitsInFile(data, header.sectionHeaderOffset, tableSize))
    {
        return tl::unexpected(
            makeError(Error::Code::BadSection, "section header table outside the file"));
    }
    const auto table = data.subspan(static_cast<std::size_t>(header.sectionHeaderOffset),
                                    static_cast<std::size_t>(tableSize));

    std::uint64_t strIndex = header.sectionNameIndex;
    if(strIndex == kSectionIndexExtended)
    {
        strIndex = readEntry(table, 0, header.is64Bit, header.endian).link;
    }

    std::span<const std::byte> strings;
    bool haveStrings = false;
    if(strIndex != kSectionIndexUndefined)
    {
        if(strIndex >= count)
        {
            return tl::unexpected(
                makeError(Error::Code::BadSection, "section name table index out of range"));
        }
        const RawSection strtab =
            readEntry(table, static_cast<std::size_t>(strIndex), header.is64Bit, header.endian);
        if(!fitsInFile(data, strtab.offset, strtab.size))
        {
            return tl::unexpected(
                makeError(Error::Code::BadSection, "section name table outside the file"));
        }
        strings = data.subspan(static_cast<std::size_t>(strtab.offset),
                               static_cast<std::size_t>(strtab.size));
        haveStrings = true;
    }

    std::vector<Section> sections;
    sections.reserve(static_cast<std::size_t>(count));
    for(std::uint64_t i = 0; i < count; ++i)
    {
        const RawSection raw =
            readEntry(table, static_cast<std::size_t>(i), header.is64Bit, header.endian);
        Section section;
        section.type = raw.type;
        section.offset = raw.offset;
        section.size = raw.size;
        section.flags = raw.flags;
        section.link = raw.link;
        section.entrySize = raw.entrySize;
        if(haveStrings)
        {
            const auto name = readCString(strings, raw.name);
            if(!name)
            {
                return tl::unexpected(
                    makeError(Error::Code::BadSection, "section name outside the string table"));
            }
            section.name = std::string(*name);
        }
        sections.push_back(std::move(section));
    }
    return sections;
}

const Section* findSection(const std::vector<Section>& sections, std::string_view name) noexcept
{
    for(const auto& section : sections)
    {
        if(section.name == name)
        {
            return &section;
        }
    }
    return nullptr;
}

} // namespace binhound
