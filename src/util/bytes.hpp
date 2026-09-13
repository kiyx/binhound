#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>

namespace binhound
{

enum class Endian : std::uint8_t
{
    Little,
    Big
};

[[nodiscard]] std::optional<std::uint16_t> readU16(std::span<const std::byte> data,
                                                   std::size_t offset, Endian endian) noexcept;

[[nodiscard]] std::optional<std::uint32_t> readU32(std::span<const std::byte> data,
                                                   std::size_t offset, Endian endian) noexcept;

[[nodiscard]] std::optional<std::uint64_t> readU64(std::span<const std::byte> data,
                                                   std::size_t offset, Endian endian) noexcept;

[[nodiscard]] std::string toHex(std::span<const std::byte> data);

} // namespace binhound
