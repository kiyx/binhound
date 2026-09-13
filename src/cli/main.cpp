#include <filesystem>
#include <iostream>
#include <string_view>

#include "parser/elf/header.hpp"
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
              << "  binhound scan <file>   Analyze a binary\n"
              << "  binhound --version     Show version\n"
              << "  binhound --help        Show this help\n";
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

int runScan(const std::filesystem::path& path)
{
    const auto file = binhound::readFile(path);
    if(!file)
    {
        printError(file.error());
        return 2;
    }

    const auto header = binhound::parseElfHeader(file->bytes);
    if(!header)
    {
        printError(header.error());
        return 2;
    }

    printHeader(*header, file->path);
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
        return runScan(std::filesystem::path{argv[2]});
    }

    std::cerr << "error: unknown command '" << command << "'\n";
    printUsage();
    return 2;
}
