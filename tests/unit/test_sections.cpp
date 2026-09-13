#include "doctest.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#include "elf_fixture.hpp"
#include "parser/elf/sections.hpp"

using binhound::Error;

namespace
{

// [null, .text PROGBITS, .dynstr STRTAB, .shstrtab STRTAB], all little-endian.
std::vector<std::byte> tableImage()
{
    std::vector<std::byte> image = fixture::elf64Header(64, 4, 3);
    fixture::reserveShdrs(image, true, 4);

    const std::size_t text = fixture::appendBytes(image, 16, std::byte{0x90});
    const std::size_t dynstr = fixture::appendStrtab(image, {"printf", "exit"});
    const std::size_t names = fixture::appendStrtab(image, {".text", ".dynstr", ".shstrtab"});

    fixture::writeShdr64(image, 64, 0, 0, 0, 0, 0, 0, 0, 0);
    fixture::writeShdr64(image, 64, 1, 1, 1, 6, text, 16, 0, 0);
    fixture::writeShdr64(image, 64, 2, 7, 3, 2, dynstr, 13, 0, 0);
    fixture::writeShdr64(image, 64, 3, 15, 3, 0, names, 25, 0, 0);
    return image;
}

binhound::ElfHeader headerOf(const std::vector<std::byte>& image)
{
    const auto header = binhound::parseElfHeader(image);
    REQUIRE(header.has_value());
    return *header;
}

// Re-parses the header from the (possibly poked) image, so corruptions of
// header fields are visible to the parser. Never call .error() on a result
// that may hold a value: tl::expected aborts on such access in debug builds.
Error::Code parseSectionsCode(const std::vector<std::byte>& image)
{
    const auto header = binhound::parseElfHeader(image);
    REQUIRE(header.has_value());
    const auto sections = binhound::parseSections(image, *header);
    REQUIRE_FALSE(sections.has_value());
    return sections.error().code;
}

} // namespace

TEST_CASE("sections: parses a table with resolved names")
{
    const auto image = tableImage();
    const auto sections = binhound::parseSections(image, headerOf(image));

    REQUIRE(sections.has_value());
    REQUIRE(sections->size() == 4);
    CHECK(sections->at(1).name == ".text");
    CHECK(sections->at(1).type == 1);
    CHECK(sections->at(1).offset == 320);
    CHECK(sections->at(1).size == 16);
    CHECK(sections->at(1).flags == 6);
    CHECK(sections->at(2).name == ".dynstr");
    CHECK(sections->at(3).name == ".shstrtab");

    const auto* found = binhound::findSection(*sections, ".text");
    REQUIRE(found != nullptr);
    CHECK(found->size == 16);
    CHECK(binhound::findSection(*sections, ".missing") == nullptr);
    CHECK(binhound::findSection({}, ".text") == nullptr);
}

TEST_CASE("sections: parses a 32-bit table")
{
    std::vector<std::byte> image = fixture::elf32Header(52, 3, 2);
    fixture::reserveShdrs(image, false, 3);

    const std::size_t text = fixture::appendBytes(image, 8, std::byte{0x90});
    const std::size_t names = fixture::appendStrtab(image, {".text", ".shstrtab"});

    fixture::writeShdr32(image, 52, 0, 0, 0, 0, 0, 0, 0, 0);
    fixture::writeShdr32(image, 52, 1, 1, 1, 6, static_cast<std::uint32_t>(text), 8, 0, 0);
    fixture::writeShdr32(image, 52, 2, 7, 3, 0, static_cast<std::uint32_t>(names), 17, 0, 0);

    const auto sections = binhound::parseSections(image, headerOf(image));

    REQUIRE(sections.has_value());
    REQUIRE(sections->size() == 3);
    CHECK(sections->at(1).name == ".text");
    CHECK(sections->at(1).offset == 172);
    CHECK(sections->at(1).size == 8);
}

TEST_CASE("sections: empty without a table")
{
    const auto image = fixture::elf64Header(0, 0, 0);
    const auto sections = binhound::parseSections(image, headerOf(image));

    REQUIRE(sections.has_value());
    CHECK(sections->empty());
}

TEST_CASE("sections: rejects bad tables")
{
    const auto good = tableImage();

    auto badEntrySize = good;
    badEntrySize[58] = std::byte{0x3F};
    CHECK(parseSectionsCode(badEntrySize) == Error::Code::BadSection);

    auto badOffset = good;
    fixture::put64(badOffset, 40, 0xFFFFFFU);
    CHECK(parseSectionsCode(badOffset) == Error::Code::BadSection);

    auto badCount = good;
    fixture::put16(badCount, 60, 9);
    CHECK(parseSectionsCode(badCount) == Error::Code::BadSection);

    auto badStrndx = good;
    fixture::put16(badStrndx, 62, 9);
    CHECK(parseSectionsCode(badStrndx) == Error::Code::BadSection);

    auto badName = good;
    fixture::put32(badName, 128, 0xFFFFFFU);
    CHECK(parseSectionsCode(badName) == Error::Code::BadSection);

    auto badStrtab = good;
    fixture::put64(badStrtab, 280, 0xFFFFFFU);
    CHECK(parseSectionsCode(badStrtab) == Error::Code::BadSection);

    auto noNames = good;
    fixture::put16(noNames, 62, 0);
    const auto header = headerOf(noNames);
    const auto unnamed = binhound::parseSections(noNames, header);
    REQUIRE(unnamed.has_value());
    CHECK(unnamed->at(1).name.empty());
}

TEST_CASE("sections: resolves extended numbering")
{
    std::vector<std::byte> image = fixture::elf64Header(64, 0, 0xFFFF);
    fixture::reserveShdrs(image, true, 2);

    const std::size_t names = fixture::appendStrtab(image, {".shstrtab"});

    fixture::writeShdr64(image, 64, 0, 0, 0, 0, 0, 2, 1, 0);
    fixture::writeShdr64(image, 64, 1, 1, 3, 0, names, 11, 0, 0);

    const auto sections = binhound::parseSections(image, headerOf(image));

    REQUIRE(sections.has_value());
    REQUIRE(sections->size() == 2);
    CHECK(sections->at(1).name == ".shstrtab");
}

TEST_CASE("sections: degenerate extended count means empty")
{
    std::vector<std::byte> image = fixture::elf64Header(64, 0, 0);
    fixture::reserveShdrs(image, true, 1);
    fixture::writeShdr64(image, 64, 0, 0, 0, 0, 0, 0, 0, 0);

    const auto sections = binhound::parseSections(image, headerOf(image));

    REQUIRE(sections.has_value());
    CHECK(sections->empty());
}
