#include <cstddef>
#include <cstdint>
#include <span>

#include "parser/elf/header.hpp"
#include "util/bytes.hpp"
#include "util/reader.hpp"

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size)
{
    const std::span<const std::byte> input(reinterpret_cast<const std::byte*>(data), size);

    (void)binhound::hasElfMagic(input);
    (void)binhound::parseElfHeader(input);

    if(!input.empty())
    {
        const std::size_t preview = input.size() > 16 ? 16 : input.size();
        (void)binhound::toHex(input.first(preview));
    }

    return 0;
}
