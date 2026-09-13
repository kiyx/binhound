#include "doctest.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#include "elf_fixture.hpp"
#include "parser/elf/symbols.hpp"

using binhound::Error;

namespace
{

void writeSym64(std::vector<std::byte>& image, std::size_t at, std::uint32_t name,
                std::uint8_t info, std::uint16_t shndx, std::uint64_t value, std::uint64_t size)
{
    fixture::put32(image, at, name);
    image[at + 4] = static_cast<std::byte>(info);
    image[at + 5] = std::byte{0};
    fixture::put16(image, at + 6, shndx);
    fixture::put64(image, at + 8, value);
    fixture::put64(image, at + 16, size);
}

// [null, .text, .dynsym -> .dynstr, .dynstr, .symtab -> .strtab, .strtab,
// .shstrtab], little-endian 64-bit.
std::vector<std::byte> tableImage()
{
    std::vector<std::byte> image = fixture::elf64Header(64, 7, 6);
    fixture::reserveShdrs(image, true, 7);

    const std::size_t text = fixture::appendBytes(image, 16, std::byte{0x90});
    const std::size_t dynstr = fixture::appendStrtab(image, {"printf"});
    const std::size_t dynsym = image.size();
    image.resize(dynsym + 48, std::byte{0});
    const std::size_t strtab = fixture::appendStrtab(image, {"main", "x"});
    const std::size_t symtab = image.size();
    image.resize(symtab + 48, std::byte{0});
    const std::size_t names = fixture::appendStrtab(
        image, {".text", ".dynsym", ".dynstr", ".symtab", ".strtab", ".shstrtab"});

    writeSym64(image, dynsym + 24, 1, 0x12, 1, 0x401000, 0);
    writeSym64(image, symtab + 24, 1, 0x12, 1, 0x401020, 32);

    fixture::writeShdr64(image, 64, 0, 0, 0, 0, 0, 0, 0, 0);
    fixture::writeShdr64(image, 64, 1, 1, 1, 6, text, 16, 0, 0);
    fixture::writeShdr64(image, 64, 2, 7, 11, 2, dynsym, 48, 3, 0, 24);
    fixture::writeShdr64(image, 64, 3, 15, 3, 2, dynstr, 8, 0, 0);
    fixture::writeShdr64(image, 64, 4, 23, 2, 0, symtab, 48, 5, 0, 24);
    fixture::writeShdr64(image, 64, 5, 31, 3, 0, strtab, 8, 0, 0);
    fixture::writeShdr64(image, 64, 6, 39, 3, 0, names, 49, 0, 0);
    return image;
}

binhound::ElfHeader headerOf(const std::vector<std::byte>& image)
{
    const auto header = binhound::parseElfHeader(image);
    REQUIRE(header.has_value());
    return *header;
}

binhound::Result<std::vector<binhound::Section>> sectionsOf(const std::vector<std::byte>& image,
                                                            const binhound::ElfHeader& header)
{
    return binhound::parseSections(image, header);
}

Error::Code parseSymbolsCode(const std::vector<std::byte>& image)
{
    const auto header = binhound::parseElfHeader(image);
    REQUIRE(header.has_value());
    const auto sections = binhound::parseSections(image, *header);
    REQUIRE(sections.has_value());
    const auto symbols = binhound::parseSymbols(image, *header, *sections);
    REQUIRE_FALSE(symbols.has_value());
    return symbols.error().code;
}

} // namespace

TEST_CASE("symbols: parses dynamic and static tables")
{
    const auto image = tableImage();
    const auto header = headerOf(image);
    const auto sections = sectionsOf(image, header);
    REQUIRE(sections.has_value());

    const auto symbols = binhound::parseSymbols(image, header, *sections);

    REQUIRE(symbols.has_value());
    REQUIRE(symbols->size() == 4);
    CHECK(symbols->at(0).name.empty());
    CHECK(symbols->at(1).name == "printf");
    CHECK(symbols->at(1).value == 0x401000);
    CHECK(symbols->at(1).binding == 1);
    CHECK(symbols->at(1).type == 2);
    CHECK(symbols->at(1).sectionIndex == 1);
    CHECK(symbols->at(3).name == "main");
    CHECK(symbols->at(3).value == 0x401020);
    CHECK(symbols->at(3).size == 32);
}

TEST_CASE("symbols: parses a 32-bit table")
{
    std::vector<std::byte> image = fixture::elf32Header(52, 5, 4);
    fixture::reserveShdrs(image, false, 5);

    const std::size_t text = fixture::appendBytes(image, 8, std::byte{0x90});
    const std::size_t dynstr = fixture::appendStrtab(image, {"f"});
    const std::size_t dynsym = image.size();
    image.resize(dynsym + 16, std::byte{0});
    const std::size_t names =
        fixture::appendStrtab(image, {".text", ".dynsym", ".dynstr", ".shstrtab"});

    fixture::put32(image, dynsym, 1);
    fixture::put32(image, dynsym + 4, 0x8000);
    fixture::put32(image, dynsym + 8, 4);
    image[dynsym + 12] = std::byte{0x12};
    image[dynsym + 13] = std::byte{0};
    fixture::put16(image, dynsym + 14, 1);

    fixture::writeShdr32(image, 52, 0, 0, 0, 0, 0, 0, 0, 0);
    fixture::writeShdr32(image, 52, 1, 1, 1, 6, static_cast<std::uint32_t>(text), 8, 0, 0);
    fixture::writeShdr32(image, 52, 2, 7, 11, 2, static_cast<std::uint32_t>(dynsym), 16, 3, 0, 16);
    fixture::writeShdr32(image, 52, 3, 15, 3, 2, static_cast<std::uint32_t>(dynstr), 3, 0, 0);
    fixture::writeShdr32(image, 52, 4, 23, 3, 0, static_cast<std::uint32_t>(names), 33, 0, 0);

    const auto header = headerOf(image);
    const auto sections = sectionsOf(image, header);
    REQUIRE(sections.has_value());
    const auto symbols = binhound::parseSymbols(image, header, *sections);

    REQUIRE(symbols.has_value());
    REQUIRE(symbols->size() == 1);
    CHECK(symbols->at(0).name == "f");
    CHECK(symbols->at(0).value == 0x8000);
    CHECK(symbols->at(0).size == 4);
    CHECK(symbols->at(0).binding == 1);
    CHECK(symbols->at(0).type == 2);
}

TEST_CASE("symbols: rejects a bad 32-bit name")
{
    std::vector<std::byte> image = fixture::elf32Header(52, 5, 4);
    fixture::reserveShdrs(image, false, 5);

    const std::size_t text = fixture::appendBytes(image, 8, std::byte{0x90});
    const std::size_t dynstr = fixture::appendStrtab(image, {"f"});
    const std::size_t dynsym = image.size();
    image.resize(dynsym + 16, std::byte{0});
    const std::size_t names =
        fixture::appendStrtab(image, {".text", ".dynsym", ".dynstr", ".shstrtab"});

    fixture::writeShdr32(image, 52, 0, 0, 0, 0, 0, 0, 0, 0);
    fixture::writeShdr32(image, 52, 1, 1, 1, 6, static_cast<std::uint32_t>(text), 8, 0, 0);
    fixture::writeShdr32(image, 52, 2, 7, 11, 2, static_cast<std::uint32_t>(dynsym), 16, 3, 0, 16);
    fixture::writeShdr32(image, 52, 3, 15, 3, 2, static_cast<std::uint32_t>(dynstr), 3, 0, 0);
    fixture::writeShdr32(image, 52, 4, 23, 3, 0, static_cast<std::uint32_t>(names), 33, 0, 0);

    fixture::put32(image, dynsym, 0xFFFFFFU);
    CHECK(parseSymbolsCode(image) == Error::Code::BadSymbol);
}

TEST_CASE("symbols: empty without tables")
{
    const auto image = fixture::elf64Header(0, 0, 0);
    const auto header = headerOf(image);
    const auto sections = sectionsOf(image, header);
    REQUIRE(sections.has_value());

    const auto symbols = binhound::parseSymbols(image, header, *sections);

    REQUIRE(symbols.has_value());
    CHECK(symbols->empty());
}

TEST_CASE("symbols: rejects bad tables")
{
    const auto good = tableImage();

    auto badLink = good;
    fixture::put32(badLink, 232, 99);
    CHECK(parseSymbolsCode(badLink) == Error::Code::BadSymbol);

    auto badEntrySize = good;
    fixture::put64(badEntrySize, 248, 16);
    CHECK(parseSymbolsCode(badEntrySize) == Error::Code::BadSymbol);

    auto badSize = good;
    fixture::put64(badSize, 224, 25);
    CHECK(parseSymbolsCode(badSize) == Error::Code::BadSymbol);

    auto badOffset = good;
    fixture::put64(badOffset, 216, 0xFFFFFFU);
    CHECK(parseSymbolsCode(badOffset) == Error::Code::BadSymbol);

    auto badName = good;
    fixture::put32(badName, 560, 0xFFFFFFU);
    CHECK(parseSymbolsCode(badName) == Error::Code::BadSymbol);

    auto badStrtab = good;
    fixture::put64(badStrtab, 280, 0xFFFFFFU);
    CHECK(parseSymbolsCode(badStrtab) == Error::Code::BadSymbol);
}
