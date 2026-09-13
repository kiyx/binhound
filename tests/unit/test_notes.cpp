#include "doctest.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#include "elf_fixture.hpp"
#include "parser/elf/notes.hpp"

using binhound::Error;

namespace
{

void appendNote(std::vector<std::byte>& image, const std::vector<std::byte>& name,
                std::uint32_t type, const std::vector<std::byte>& desc)
{
    const std::size_t at = image.size();
    image.resize(at + 12, std::byte{0});
    fixture::put32(image, at, static_cast<std::uint32_t>(name.size()));
    fixture::put32(image, at + 4, static_cast<std::uint32_t>(desc.size()));
    fixture::put32(image, at + 8, type);
    const auto pad = [&image](const std::vector<std::byte>& bytes)
    {
        image.insert(image.end(), bytes.begin(), bytes.end());
        while((image.size() % 4U) != 0U)
        {
            image.push_back(std::byte{0});
        }
    };
    pad(name);
    pad(desc);
}

std::vector<std::byte> bytes(std::initializer_list<std::uint8_t> values)
{
    std::vector<std::byte> out;
    for(const auto value : values)
    {
        out.push_back(static_cast<std::byte>(value));
    }
    return out;
}

std::vector<std::byte> bytes(std::size_t count, std::uint8_t fill)
{
    return std::vector<std::byte>(count, static_cast<std::byte>(fill));
}

// One NOTE section with ABI-tag, empty GNU, foreign-owner and Build-ID notes.
std::vector<std::byte> notesImage()
{
    std::vector<std::byte> image = fixture::elf64Header(64, 3, 2);
    fixture::reserveShdrs(image, true, 3);

    const std::size_t note = image.size();
    appendNote(image, bytes({'G', 'N', 'U', 0}), 1, bytes(16, 0));
    appendNote(image, bytes({'G', 'N', 'U', 0}), 3, {});
    appendNote(image, bytes({'F', 'O', 'O', 0}), 3, bytes({0xAA, 0xBB, 0xCC, 0xDD}));
    std::vector<std::byte> buildId;
    for(std::uint8_t i = 1; i <= 20; ++i)
    {
        buildId.push_back(static_cast<std::byte>(i));
    }
    appendNote(image, bytes({'G', 'N', 'U', 0}), 3, buildId);
    const std::size_t noteSize = image.size() - note;

    const std::size_t names = fixture::appendStrtab(image, {".note", ".shstrtab"});

    fixture::writeShdr64(image, 64, 0, 0, 0, 0, 0, 0, 0, 0);
    fixture::writeShdr64(image, 64, 1, 1, 7, 2, note, noteSize, 0, 0);
    fixture::writeShdr64(image, 64, 2, 8, 3, 0, names, 17, 0, 0);
    return image;
}

binhound::ElfHeader headerOf(const std::vector<std::byte>& image)
{
    const auto header = binhound::parseElfHeader(image);
    REQUIRE(header.has_value());
    return *header;
}

Error::Code parseNotesCode(const std::vector<std::byte>& image)
{
    const auto header = binhound::parseElfHeader(image);
    REQUIRE(header.has_value());
    const auto sections = binhound::parseSections(image, *header);
    REQUIRE(sections.has_value());
    const auto notes = binhound::parseNotes(image, *header, *sections);
    REQUIRE_FALSE(notes.has_value());
    return notes.error().code;
}

} // namespace

TEST_CASE("notes: parses entries and finds the Build-ID")
{
    const auto image = notesImage();
    const auto header = headerOf(image);
    const auto sections = binhound::parseSections(image, header);
    REQUIRE(sections.has_value());
    const auto notes = binhound::parseNotes(image, header, *sections);

    REQUIRE(notes.has_value());
    REQUIRE(notes->size() == 4);
    CHECK(notes->at(0).type == 1);
    CHECK(notes->at(0).name == std::string("GNU\0", 4));
    CHECK(notes->at(0).description.size() == 16);

    const auto buildId = binhound::findBuildId(*notes);
    REQUIRE(buildId.has_value());
    CHECK(buildId == "0102030405060708090a0b0c0d0e0f1011121314");
}

TEST_CASE("notes: empty without note sections")
{
    const auto image = fixture::elf64Header(0, 0, 0);
    const auto header = headerOf(image);
    const auto sections = binhound::parseSections(image, header);
    REQUIRE(sections.has_value());
    const auto notes = binhound::parseNotes(image, header, *sections);

    REQUIRE(notes.has_value());
    CHECK(notes->empty());
    CHECK_FALSE(binhound::findBuildId(*notes).has_value());
}

TEST_CASE("notes: rejects truncated notes")
{
    const auto good = notesImage();
    const auto header = headerOf(good);
    const auto sections = binhound::parseSections(good, header);
    REQUIRE(sections.has_value());

    auto cutPayload = good;
    cutPayload.resize(350);
    CHECK(binhound::parseNotes(cutPayload, header, *sections).error().code == Error::Code::BadNote);

    auto tinySection = good;
    fixture::put64(tinySection, 160, 5);
    CHECK(parseNotesCode(tinySection) == Error::Code::BadNote);

    auto hugeName = good;
    fixture::put32(hugeName, 256, 0xFFFFFFFFU);
    CHECK(parseNotesCode(hugeName) == Error::Code::BadNote);

    auto badOffset = good;
    fixture::put64(badOffset, 152, 0xFFFFFFU);
    CHECK(parseNotesCode(badOffset) == Error::Code::BadNote);
}
