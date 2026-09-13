#include "doctest.hpp"

#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <system_error>
#include <vector>

#include "util/reader.hpp"

namespace
{

class TempFile
{
public:
    explicit TempFile(const std::string& name)
        : path_(std::filesystem::temp_directory_path() / ("binhound_test_" + name + ".bin"))
    {
    }

    ~TempFile()
    {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
    }

    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;
    TempFile(TempFile&&) = delete;
    TempFile& operator=(TempFile&&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const noexcept
    {
        return path_;
    }

    void write(std::span<const std::byte> bytes) const
    {
        std::ofstream stream(path_, std::ios::binary);
        stream.write(reinterpret_cast<const char*>(bytes.data()),
                     static_cast<std::streamsize>(bytes.size()));
    }

private:
    std::filesystem::path path_;
};

} // namespace

TEST_CASE("reader: reads file content")
{
    const TempFile file("read");
    const std::array<std::byte, 4> content{std::byte{0x7F}, std::byte{'E'}, std::byte{'L'},
                                           std::byte{'F'}};
    file.write(content);

    const auto result = binhound::readFile(file.path());
    REQUIRE(result.has_value());
    CHECK(result->bytes == std::vector<std::byte>(content.begin(), content.end()));
    CHECK(result->path == file.path());
}

TEST_CASE("reader: missing file")
{
    const auto missing = std::filesystem::temp_directory_path() / "binhound_test_missing_file.bin";
    const auto result = binhound::readFile(missing);

    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().code == binhound::Error::Code::FileNotFound);
}

TEST_CASE("reader: enforces the size limit")
{
    const TempFile file("large");
    const std::array<std::byte, 10> content{};
    file.write(content);

    const auto result = binhound::readFile(file.path(), 4);

    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().code == binhound::Error::Code::FileTooLarge);
}

TEST_CASE("reader: empty file is valid")
{
    const TempFile file("empty");
    file.write(std::span<const std::byte>{});

    const auto result = binhound::readFile(file.path());
    REQUIRE(result.has_value());
    CHECK(result->bytes.empty());
}

TEST_CASE("reader: slice bounds")
{
    const TempFile file("slice");
    const std::array<std::byte, 10> content{};
    file.write(content);

    const auto result = binhound::readFile(file.path());
    REQUIRE(result.has_value());

    CHECK(binhound::slice(*result, 8, 2).has_value());
    CHECK_FALSE(binhound::slice(*result, 8, 4).has_value());
    CHECK_FALSE(binhound::slice(*result, 11, 0).has_value());
    CHECK(binhound::slice(*result, 10, 0).has_value());
}

TEST_CASE("reader: elf magic")
{
    const std::array<std::byte, 4> elf{std::byte{0x7F}, std::byte{'E'}, std::byte{'L'},
                                       std::byte{'F'}};
    const std::array<std::byte, 4> other{std::byte{0xFF}, std::byte{'E'}, std::byte{'L'},
                                         std::byte{'F'}};

    CHECK(binhound::hasElfMagic(elf));
    CHECK_FALSE(binhound::hasElfMagic(other));
    CHECK_FALSE(binhound::hasElfMagic(std::span<const std::byte>(elf).first(2)));
}
