#pragma once

#include <cstdint>
#include <string>
#include <string_view>

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
        DatabaseMissing
    };

    Code code;
    std::string message;
};

[[nodiscard]] std::string_view errorCodeName(Error::Code code) noexcept;

template <typename T> using Result = tl::expected<T, Error>;

} // namespace binhound
