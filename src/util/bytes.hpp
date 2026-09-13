#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

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

// Read an integer, or `fallback` when out of bounds. Callers validate the
// range first; the fallback only guards against logic errors, never input.
[[nodiscard]] std::uint16_t readU16Or(std::span<const std::byte> data, std::size_t offset,
                                      Endian endian, std::uint16_t fallback = 0) noexcept;

[[nodiscard]] std::uint32_t readU32Or(std::span<const std::byte> data, std::size_t offset,
                                      Endian endian, std::uint32_t fallback = 0) noexcept;

[[nodiscard]] std::uint64_t readU64Or(std::span<const std::byte> data, std::size_t offset,
                                      Endian endian, std::uint64_t fallback = 0) noexcept;

// Read a NUL-terminated string at offset; nullopt when out of bounds or
// unterminated within the span.
[[nodiscard]] std::optional<std::string_view> readCString(std::span<const std::byte> data,
                                                          std::uint64_t offset) noexcept;

[[nodiscard]] std::string toHex(std::span<const std::byte> data);

// True when [offset, offset + size) lies within data (overflow-safe).
[[nodiscard]] bool fitsInFile(std::span<const std::byte> data, std::uint64_t offset,
                              std::uint64_t size) noexcept;

} // namespace binhound
