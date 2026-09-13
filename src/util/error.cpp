#include "util/error.hpp"

namespace binhound
{

std::string_view errorCodeName(Error::Code code) noexcept
{
    // Exhaustive over Error::Code: the switch fallthrough arc is unreachable.
    switch(code) // GCOV_EXCL_BR_LINE
    {
    case Error::Code::FileNotFound:
        return "FileNotFound";
    case Error::Code::FileTooLarge:
        return "FileTooLarge";
    case Error::Code::ReadFailed:
        return "ReadFailed";
    case Error::Code::NotElf:
        return "NotElf";
    case Error::Code::Truncated:
        return "Truncated";
    case Error::Code::BadEndianness:
        return "BadEndianness";
    case Error::Code::UnsupportedClass:
        return "UnsupportedClass";
    case Error::Code::BadSection:
        return "BadSection";
    case Error::Code::BadSymbol:
        return "BadSymbol";
    case Error::Code::BadNote:
        return "BadNote";
    case Error::Code::DatabaseMissing:
        return "DatabaseMissing";
    }
    return "Unknown"; // GCOV_EXCL_LINE
}

} // namespace binhound
