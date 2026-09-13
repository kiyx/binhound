#include "parser/elf/notes.hpp"

#include "util/bytes.hpp"

// Note layout (namesz, descsz, type, then both payloads padded to 4),
// see man7 elf(5).
namespace binhound
{

namespace
{

constexpr std::uint32_t kNoteSectionType = 7; // SHT_NOTE
constexpr std::uint32_t kGnuBuildIdType = 3;  // NT_GNU_BUILD_ID
constexpr std::string_view kGnuNoteName{"GNU\0", 4};

constexpr std::size_t kNoteHeaderSize = 12;

constexpr std::uint64_t alignUp4(std::uint64_t value) noexcept
{
    // NOTE: the mask must be 64-bit; `~3U` would clear the high half and turn
    // huge inputs (e.g. namesz 0xFFFFFFFF) into 0, defeating the bounds check.
    return (value + 3U) & ~std::uint64_t{3U};
}

} // namespace

Result<std::vector<Note>> parseNotes(std::span<const std::byte> data, const ElfHeader& header,
                                     const std::vector<Section>& sections)
{
    std::vector<Note> notes;
    for(const auto& section : sections)
    {
        if(section.type != kNoteSectionType)
        {
            continue;
        }
        if(!fitsInFile(data, section.offset, section.size))
        {
            return tl::unexpected(makeError(Error::Code::BadNote, "note section outside the file"));
        }

        std::uint64_t pos = section.offset;
        const std::uint64_t end = section.offset + section.size;
        while(pos < end)
        {
            if(!fitsInFile(data, pos, kNoteHeaderSize))
            {
                return tl::unexpected(makeError(Error::Code::BadNote, "truncated note header"));
            }
            const auto at = static_cast<std::size_t>(pos);
            const auto namesz = readU32Or(data, at, header.endian);
            const auto descsz = readU32Or(data, at + 4, header.endian);
            const auto type = readU32Or(data, at + 8, header.endian);

            const std::uint64_t nameAt = pos + kNoteHeaderSize;
            const std::uint64_t descAt = nameAt + alignUp4(namesz);
            const std::uint64_t next = descAt + alignUp4(descsz);
            if(next > end)
            {
                return tl::unexpected(makeError(Error::Code::BadNote, "truncated note payload"));
            }

            Note note;
            note.type = type;
            note.name =
                std::string(reinterpret_cast<const char*>(&data[static_cast<std::size_t>(nameAt)]),
                            static_cast<std::size_t>(namesz));
            note.description.assign(data.begin() + static_cast<std::ptrdiff_t>(descAt),
                                    data.begin() + static_cast<std::ptrdiff_t>(descAt + descsz));
            notes.push_back(std::move(note));
            pos = next;
        }
    }
    return notes;
}

std::optional<std::string> findBuildId(const std::vector<Note>& notes)
{
    for(const auto& note : notes)
    {
        if(note.type != kGnuBuildIdType)
        {
            continue;
        }
        if(note.name != kGnuNoteName)
        {
            continue;
        }
        if(note.description.empty())
        {
            continue;
        }
        return toHex(note.description);
    }
    return std::nullopt;
}

} // namespace binhound
