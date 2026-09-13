#include "util/error.hpp"

namespace binhound
{

std::string_view errorCodeName(Error::Code code) noexcept
{
    switch(code)
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
    case Error::Code::DatabaseMissing:
        return "DatabaseMissing";
    }
    return "Unknown";
}

} // namespace binhound
