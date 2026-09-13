#include <cstddef>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

#include "parser/elf/elf.hpp"
#include "util/reader.hpp"

namespace
{

constexpr std::string_view kProgramName = "binhound";

void printUsage()
{
    std::cout << kProgramName << " " << BINHOUND_VERSION << '\n'
              << "Static SBOM/CBOM generator and CRA readiness checker for compiled binaries\n"
              << '\n'
              << "Usage:\n"
              << "  binhound scan <file> [--verbose]   Analyze a binary\n"
              << "  binhound --version                 Show version\n"
              << "  binhound --help                    Show this help\n";
}

void printError(const binhound::Error& error)
{
    std::cerr << "error: " << binhound::errorCodeName(error.code) << ": " << error.message << '\n';
}

void printHeader(const binhound::ElfHeader& header, const std::filesystem::path& path)
{
    std::cout << "File:       " << path.string() << '\n'
              << "Class:      " << (header.is64Bit ? "ELF64" : "ELF32") << '\n'
              << "Endianness: " << (header.endian == binhound::Endian::Little ? "little" : "big")
              << '\n'
              << "Type:       " << binhound::typeName(header.type) << '\n'
              << "Machine:    " << binhound::machineName(header.machine) << '\n'
              << "Entry:      0x" << std::hex << header.entry << std::dec << '\n'
              << "Sections:   " << header.sectionHeaderCount << '\n';
}

void printVerbose(const binhound::ElfInfo& info)
{
    for(std::size_t i = 0; i < info.sections.size(); ++i)
    {
        const auto& section = info.sections[i];
        std::cout << "Section[" << i << "]: " << section.name << " type=" << section.type
                  << " size=" << section.size << '\n';
    }
    std::cout << "Symbols:    " << info.symbols.size() << '\n';
    std::cout << "Build-ID:   " << info.buildId.value_or("none") << '\n';
}

int runScan(const std::filesystem::path& path, bool verbose)
{
    const auto file = binhound::readFile(path);
    if(!file)
    {
        printError(file.error());
        return 2;
    }

    const auto info = binhound::parseElf(file->bytes);
    if(!info)
    {
        printError(info.error());
        return 2;
    }

    printHeader(info->header, file->path);
    if(verbose)
    {
        printVerbose(*info);
    }
    return 0;
}

} // namespace

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        printUsage();
        return 0;
    }

    const std::string_view command = argv[1];

    if(command == "--version")
    {
        std::cout << kProgramName << " " << BINHOUND_VERSION << '\n';
        return 0;
    }

    if(command == "--help")
    {
        printUsage();
        return 0;
    }

    if(command == "scan")
    {
        if(argc < 3)
        {
            std::cerr << "error: scan requires a file argument\n";
            return 2;
        }
        bool verbose = false;
        if(argc > 3)
        {
            if(std::string_view(argv[3]) != "--verbose" || argc > 4)
            {
                std::cerr << "error: unexpected argument '" << argv[3] << "'\n";
                return 2;
            }
            verbose = true;
        }
        return runScan(std::filesystem::path{argv[2]}, verbose);
    }

    std::cerr << "error: unknown command '" << command << "'\n";
    printUsage();
    return 2;
}
