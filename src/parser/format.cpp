#include "parser/format.hpp"

#include "util/reader.hpp"

namespace binhound
{

Format detectFormat(std::span<const std::byte> data) noexcept
{
    return hasElfMagic(data) ? Format::Elf : Format::Unknown;
}

std::string_view formatName(Format format) noexcept
{
    // Exhaustive over Format: the fallthrough arc is unreachable.
    switch(format) // GCOV_EXCL_BR_LINE
    {
    case Format::Elf:
        return "ELF";
    case Format::Unknown:
        return "unknown";
    }
    return "unknown"; // GCOV_EXCL_LINE
}

} // namespace binhound
