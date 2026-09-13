#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace binhound
{

enum class Format : std::uint8_t
{
    Unknown,
    Elf
};

[[nodiscard]] Format detectFormat(std::span<const std::byte> data) noexcept;

[[nodiscard]] std::string_view formatName(Format format) noexcept;

} // namespace binhound
