#include "util/reader.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <system_error>

namespace binhound
{

namespace
{

constexpr std::array<std::byte, 4> kElfMagic = {std::byte{0x7F}, std::byte{'E'}, std::byte{'L'},
                                                std::byte{'F'}};

Error makeError(Error::Code code, std::string message)
{
    return Error{code, std::move(message)};
}

} // namespace

Result<FileData> readFile(const std::filesystem::path& path, std::size_t maxBytes)
{
    std::error_code ec;
    if(!std::filesystem::is_regular_file(path, ec))
    {
        return tl::unexpected(
            makeError(Error::Code::FileNotFound, "not a regular file: " + path.string()));
    }

    const auto size = std::filesystem::file_size(path, ec);
    if(ec)
    {
        return tl::unexpected(
            makeError(Error::Code::ReadFailed, "cannot read the size of: " + path.string()));
    }

    if(size > static_cast<std::uintmax_t>(maxBytes))
    {
        return tl::unexpected(makeError(Error::Code::FileTooLarge,
                                        "file larger than the configured limit: " + path.string()));
    }

    std::ifstream stream(path, std::ios::binary);
    if(!stream)
    {
        return tl::unexpected(makeError(Error::Code::ReadFailed, "cannot open: " + path.string()));
    }

    std::vector<std::byte> bytes(static_cast<std::size_t>(size));
    if(!bytes.empty())
    {
        stream.read(reinterpret_cast<char*>(bytes.data()),
                    static_cast<std::streamsize>(bytes.size()));
        if(!stream || stream.gcount() != static_cast<std::streamsize>(bytes.size()))
        {
            return tl::unexpected(
                makeError(Error::Code::ReadFailed, "short read on: " + path.string()));
        }
    }

    return FileData{path, std::move(bytes)};
}

std::optional<std::span<const std::byte>> slice(const FileData& file, std::size_t offset,
                                                std::size_t length) noexcept
{
    if(offset > file.bytes.size() || length > file.bytes.size() - offset)
    {
        return std::nullopt;
    }

    return std::span<const std::byte>(file.bytes).subspan(offset, length);
}

bool hasElfMagic(std::span<const std::byte> data) noexcept
{
    return data.size() >= kElfMagic.size() &&
           std::equal(kElfMagic.begin(), kElfMagic.end(), data.begin());
}

} // namespace binhound
