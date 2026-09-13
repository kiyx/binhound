#pragma once

// Builders for synthetic in-memory ELF images used by the parser unit tests.
// Everything is little-endian: endianness handling is centralized in
// readU16/32/64 (covered by test_bytes), so fixtures only vary the class.

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace fixture
{

inline void put16(std::vector<std::byte>& image, std::size_t offset, std::uint16_t value)
{
    image[offset] = static_cast<std::byte>(value & 0xFFU);
    image[offset + 1] = static_cast<std::byte>(value >> 8U);
}

inline void put32(std::vector<std::byte>& image, std::size_t offset, std::uint32_t value)
{
    image[offset] = static_cast<std::byte>(value & 0xFFU);
    image[offset + 1] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    image[offset + 2] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    image[offset + 3] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

inline void put64(std::vector<std::byte>& image, std::size_t offset, std::uint64_t value)
{
    for(std::size_t i = 0; i < 8; ++i)
    {
        image[offset + i] = static_cast<std::byte>((value >> (8U * i)) & 0xFFU);
    }
}

inline std::vector<std::byte> elf64Header(std::uint64_t shoff, std::uint16_t shnum,
                                          std::uint16_t shstrndx)
{
    std::vector<std::byte> image(64, std::byte{0});
    image[0] = std::byte{0x7F};
    image[1] = std::byte{'E'};
    image[2] = std::byte{'L'};
    image[3] = std::byte{'F'};
    image[4] = std::byte{2};
    image[5] = std::byte{1};
    put64(image, 40, shoff);
    put16(image, 58, 64);
    put16(image, 60, shnum);
    put16(image, 62, shstrndx);
    return image;
}

inline std::vector<std::byte> elf32Header(std::uint64_t shoff, std::uint16_t shnum,
                                          std::uint16_t shstrndx)
{
    std::vector<std::byte> image(52, std::byte{0});
    image[0] = std::byte{0x7F};
    image[1] = std::byte{'E'};
    image[2] = std::byte{'L'};
    image[3] = std::byte{'F'};
    image[4] = std::byte{1};
    image[5] = std::byte{1};
    put32(image, 32, static_cast<std::uint32_t>(shoff));
    put16(image, 46, 40);
    put16(image, 48, shnum);
    put16(image, 50, shstrndx);
    return image;
}

// Appends `count` zero section slots; write them later with writeShdr64/32
// once content offsets are known.
inline std::size_t reserveShdrs(std::vector<std::byte>& image, bool is64Bit, std::size_t count)
{
    const std::size_t at = image.size();
    image.resize(at + count * (is64Bit ? 64U : 40U), std::byte{0});
    return at;
}

inline void writeShdr64(std::vector<std::byte>& image, std::size_t shoff, std::size_t slot,
                        std::uint32_t name, std::uint32_t type, std::uint64_t flags,
                        std::uint64_t offset, std::uint64_t size, std::uint32_t link,
                        std::uint32_t info, std::uint64_t entrySize = 0)
{
    const std::size_t at = shoff + slot * 64U;
    put32(image, at, name);
    put32(image, at + 4, type);
    put64(image, at + 8, flags);
    put64(image, at + 24, offset);
    put64(image, at + 32, size);
    put32(image, at + 40, link);
    put32(image, at + 44, info);
    put64(image, at + 56, entrySize);
}

inline void writeShdr32(std::vector<std::byte>& image, std::size_t shoff, std::size_t slot,
                        std::uint32_t name, std::uint32_t type, std::uint32_t flags,
                        std::uint32_t offset, std::uint32_t size, std::uint32_t link,
                        std::uint32_t info, std::uint32_t entrySize = 0)
{
    const std::size_t at = shoff + slot * 40U;
    put32(image, at, name);
    put32(image, at + 4, type);
    put32(image, at + 8, flags);
    put32(image, at + 16, offset);
    put32(image, at + 20, size);
    put32(image, at + 24, link);
    put32(image, at + 28, info);
    put32(image, at + 36, entrySize);
}

// Appends bytes, returns their offset.
inline std::size_t appendBytes(std::vector<std::byte>& image, std::size_t count, std::byte fill)
{
    const std::size_t at = image.size();
    image.insert(image.end(), count, fill);
    return at;
}

// Appends a string table (leading NUL, NUL-joined names), returns its offset.
inline std::size_t appendStrtab(std::vector<std::byte>& image,
                                std::initializer_list<std::string_view> names)
{
    const std::size_t at = image.size();
    image.push_back(std::byte{0});
    for(const auto name : names)
    {
        for(const char letter : name)
        {
            image.push_back(static_cast<std::byte>(letter));
        }
        image.push_back(std::byte{0});
    }
    return at;
}

} // namespace fixture
