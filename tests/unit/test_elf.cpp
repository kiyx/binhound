#include "doctest.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "elf_fixture.hpp"
#include "parser/elf/elf.hpp"

using binhound::Error;

namespace
{

void writeSym(std::vector<std::byte>& image, std::size_t at, std::uint32_t name, std::uint8_t info,
              std::uint16_t shndx, std::uint64_t value, std::uint64_t size)
{
    fixture::put32(image, at, name);
    image[at + 4] = static_cast<std::byte>(info);
    image[at + 5] = std::byte{0};
    fixture::put16(image, at + 6, shndx);
    fixture::put64(image, at + 8, value);
    fixture::put64(image, at + 16, size);
}

void appendBuildIdNote(std::vector<std::byte>& image)
{
    const std::size_t at = image.size();
    image.resize(at + 12, std::byte{0});
    fixture::put32(image, at, 4);
    fixture::put32(image, at + 4, 4);
    fixture::put32(image, at + 8, 3);
    for(const char letter : {'G', 'N', 'U', '\0', '\xDE', '\xAD', '\xBE', '\xEF'})
    {
        image.push_back(static_cast<std::byte>(letter));
    }
}

// Dynamic and stripped: .dynsym but no .symtab, one GNU Build-ID note.
std::vector<std::byte> dynamicImage()
{
    std::vector<std::byte> image = fixture::elf64Header(64, 6, 5);
    fixture::reserveShdrs(image, true, 6);

    const std::size_t text = fixture::appendBytes(image, 16, std::byte{0x90});
    const std::size_t dynstr = fixture::appendStrtab(image, {"printf"});
    const std::size_t dynsym = image.size();
    image.resize(dynsym + 48, std::byte{0});
    const std::size_t note = image.size();
    appendBuildIdNote(image);
    const std::size_t noteSize = image.size() - note;
    const std::size_t names =
        fixture::appendStrtab(image, {".text", ".dynsym", ".dynstr", ".note", ".shstrtab"});

    writeSym(image, dynsym + 24, 1, 0x12, 1, 0x401000, 0);

    fixture::writeShdr64(image, 64, 0, 0, 0, 0, 0, 0, 0, 0);
    fixture::writeShdr64(image, 64, 1, 1, 1, 6, text, 16, 0, 0);
    fixture::writeShdr64(image, 64, 2, 7, 11, 2, dynsym, 48, 3, 0, 24);
    fixture::writeShdr64(image, 64, 3, 15, 3, 2, dynstr, 8, 0, 0);
    fixture::writeShdr64(image, 64, 4, 23, 7, 2, note, noteSize, 0, 0);
    fixture::writeShdr64(image, 64, 5, 29, 3, 0, names, 39, 0, 0);
    return image;
}

// Static with symbols: .symtab but no .dynsym and no .dynamic.
std::vector<std::byte> staticImage()
{
    std::vector<std::byte> image = fixture::elf64Header(64, 5, 4);
    fixture::reserveShdrs(image, true, 5);

    const std::size_t text = fixture::appendBytes(image, 8, std::byte{0x90});
    const std::size_t strtab = fixture::appendStrtab(image, {"main"});
    const std::size_t symtab = image.size();
    image.resize(symtab + 48, std::byte{0});
    const std::size_t names =
        fixture::appendStrtab(image, {".text", ".symtab", ".strtab", ".shstrtab"});

    writeSym(image, symtab + 24, 1, 0x12, 1, 0x401020, 8);

    fixture::writeShdr64(image, 64, 0, 0, 0, 0, 0, 0, 0, 0);
    fixture::writeShdr64(image, 64, 1, 1, 1, 6, text, 8, 0, 0);
    fixture::writeShdr64(image, 64, 2, 7, 2, 0, symtab, 48, 3, 0, 24);
    fixture::writeShdr64(image, 64, 3, 15, 3, 0, strtab, 6, 0, 0);
    fixture::writeShdr64(image, 64, 4, 23, 3, 0, names, 33, 0, 0);
    return image;
}

// Dynamic through a .dynamic section only.
std::vector<std::byte> dynamicOnlyImage()
{
    std::vector<std::byte> image = fixture::elf64Header(64, 4, 3);
    fixture::reserveShdrs(image, true, 4);

    const std::size_t text = fixture::appendBytes(image, 8, std::byte{0x90});
    const std::size_t names = fixture::appendStrtab(image, {".text", ".dynamic", ".shstrtab"});

    fixture::writeShdr64(image, 64, 0, 0, 0, 0, 0, 0, 0, 0);
    fixture::writeShdr64(image, 64, 1, 1, 1, 6, text, 8, 0, 0);
    fixture::writeShdr64(image, 64, 2, 7, 6, 2, 0, 0, 0, 0);
    fixture::writeShdr64(image, 64, 3, 17, 3, 0, names, 26, 0, 0);
    return image;
}

} // namespace

TEST_CASE("elf: parses a dynamic stripped file")
{
    const auto info = binhound::parseElf(dynamicImage());

    REQUIRE(info.has_value());
    CHECK(info->header.is64Bit);
    REQUIRE(info->sections.size() == 6);
    REQUIRE(info->symbols.size() == 2);
    CHECK(info->symbols.at(1).name == "printf");
    REQUIRE(info->buildId.has_value());
    CHECK(info->buildId == "deadbeef");
    CHECK(binhound::isDynamic(*info));
    CHECK(binhound::isStripped(*info));
}

TEST_CASE("elf: parses a static file with symbols")
{
    const auto info = binhound::parseElf(staticImage());

    REQUIRE(info.has_value());
    CHECK_FALSE(binhound::isDynamic(*info));
    CHECK_FALSE(binhound::isStripped(*info));
    REQUIRE(info->symbols.size() == 2);
    CHECK(info->symbols.at(1).name == "main");
}

TEST_CASE("elf: dynamic via .dynamic section")
{
    const auto info = binhound::parseElf(dynamicOnlyImage());

    REQUIRE(info.has_value());
    CHECK(binhound::isDynamic(*info));
    CHECK(binhound::isStripped(*info));
}

TEST_CASE("elf: header-only file has no tables")
{
    const auto info = binhound::parseElf(fixture::elf64Header(0, 0, 0));

    REQUIRE(info.has_value());
    CHECK(info->sections.empty());
    CHECK(info->symbols.empty());
    CHECK_FALSE(info->buildId.has_value());
    CHECK_FALSE(binhound::isDynamic(*info));
    CHECK(binhound::isStripped(*info));
}

TEST_CASE("elf: propagates stage errors")
{
    const auto good = dynamicImage();
    const std::vector<std::byte> shortMagic{
        std::byte{0x7F}, std::byte{'E'}, std::byte{'L'}, std::byte{'F'}, std::byte{2},
        std::byte{1},    std::byte{1},   std::byte{0},   std::byte{0},   std::byte{0}};
    CHECK(binhound::parseElf(shortMagic).error().code == Error::Code::Truncated);

    auto badSections = good;
    fixture::put64(badSections, 40, 0xFFFFFFU);
    CHECK(binhound::parseElf(badSections).error().code == Error::Code::BadSection);

    auto badSymbols = good;
    fixture::put32(badSymbols, 232, 99);
    CHECK(binhound::parseElf(badSymbols).error().code == Error::Code::BadSymbol);

    auto badNotes = good;
    fixture::put32(badNotes, 524, 0xFFFFU);
    CHECK(binhound::parseElf(badNotes).error().code == Error::Code::BadNote);

    const std::vector<std::byte> notElf(10, std::byte{0});
    CHECK(binhound::parseElf(notElf).error().code == Error::Code::NotElf);
}
