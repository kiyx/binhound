#include "util/bytes.hpp"

#include <string_view>

namespace binhound
{

namespace
{

constexpr std::string_view kHexDigits = "0123456789abcdef";

template <typename UInt>
std::optional<UInt> readUnsigned(std::span<const std::byte> data, std::size_t offset,
                                 Endian endian) noexcept
{
    constexpr auto size = sizeof(UInt);
    if(offset > data.size() || data.size() - offset < size)
    {
        return std::nullopt;
    }

    UInt value = 0;
    if(endian == Endian::Little)
    {
        for(std::size_t i = 0; i < size; ++i)
        {
            const auto byte = std::to_integer<UInt>(data[offset + i]);
            value |= static_cast<UInt>(byte << (8U * static_cast<unsigned>(i)));
        }
    }
    else
    {
        for(std::size_t i = 0; i < size; ++i)
        {
            const auto byte = std::to_integer<UInt>(data[offset + i]);
            value = static_cast<UInt>((value << 8U) | byte);
        }
    }
    return value;
}

} // namespace

std::optional<std::uint16_t> readU16(std::span<const std::byte> data, std::size_t offset,
                                     Endian endian) noexcept
{
    return readUnsigned<std::uint16_t>(data, offset, endian);
}

std::optional<std::uint32_t> readU32(std::span<const std::byte> data, std::size_t offset,
                                     Endian endian) noexcept
{
    return readUnsigned<std::uint32_t>(data, offset, endian);
}

std::optional<std::uint64_t> readU64(std::span<const std::byte> data, std::size_t offset,
                                     Endian endian) noexcept
{
    return readUnsigned<std::uint64_t>(data, offset, endian);
}

std::string toHex(std::span<const std::byte> data)
{
    std::string out;
    out.reserve(data.size() * 2);

    for(const auto byte : data)
    {
        const auto value = std::to_integer<std::uint8_t>(byte);
        out.push_back(kHexDigits[static_cast<std::size_t>(value >> 4U)]);
        out.push_back(kHexDigits[static_cast<std::size_t>(value & 0x0FU)]);
    }

    return out;
}

} // namespace binhound
