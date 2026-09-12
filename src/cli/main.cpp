#include <iostream>
#include <string_view>

namespace {

constexpr std::string_view kProgramName = "binhound";

void print_usage() {
    std::cout << "binhound " << BINHOUND_VERSION << "\n"
              << "Static SBOM/CBOM generator and CRA readiness checker for compiled binaries\n"
              << "\n"
              << "Usage:\n"
              << "  binhound --version        Show version\n"
              << "  binhound --help           Show this help\n"
              << "  binhound scan <file>      Analyze a binary (not implemented yet)\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 0;
    }

    const std::string_view command = argv[1];

    if (command == "--version") {
        std::cout << kProgramName << " " << BINHOUND_VERSION << "\n";
        return 0;
    }

    if (command == "--help") {
        print_usage();
        return 0;
    }

    if (command == "scan") {
        std::cerr << "error: scan is not implemented yet\n";
        return 2;
    }

    std::cerr << "error: unknown command '" << command << "'\n";
    print_usage();
    return 2;
}
