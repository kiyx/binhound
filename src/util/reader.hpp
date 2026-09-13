#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <span>
#include <vector>

#include "util/error.hpp"

namespace binhound
{

struct FileData
{
    std::filesystem::path path;
    std::vector<std::byte> bytes;
};

inline constexpr std::size_t kDefaultMaxFileSize = 512ULL * 1024ULL * 1024ULL;

[[nodiscard]] Result<FileData> readFile(const std::filesystem::path& path,
                                        std::size_t maxBytes = kDefaultMaxFileSize);

[[nodiscard]] std::optional<std::span<const std::byte>>
slice(const FileData& file, std::size_t offset, std::size_t length) noexcept;

[[nodiscard]] bool hasElfMagic(std::span<const std::byte> data) noexcept;

} // namespace binhound
