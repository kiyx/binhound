#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

#include <tl/expected.hpp>

namespace binhound
{

struct Error
{
    enum class Code : std::uint8_t
    {
        FileNotFound,
        FileTooLarge,
        ReadFailed,
        NotElf,
        Truncated,
        BadEndianness,
        UnsupportedClass,
        BadSection,
        BadSymbol,
        BadNote,
        DatabaseMissing
    };

    Code code;
    std::string message;
};

[[nodiscard]] inline Error makeError(Error::Code code, std::string message)
{
    return Error{.code = code, .message = std::move(message)};
}

[[nodiscard]] std::string_view errorCodeName(Error::Code code) noexcept;

template <typename T> using Result = tl::expected<T, Error>;

} // namespace binhound
