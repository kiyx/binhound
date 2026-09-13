# BinHound

> Static analysis for compiled binaries: it inventories the software components, and later the
> cryptography and known vulnerabilities, with evidence and confidence.

English | [Italiano](README.it.md)

[![CI](https://github.com/kiyx/binhound/actions/workflows/ci.yml/badge.svg)](https://github.com/kiyx/binhound/actions/workflows/ci.yml)
[![CodeQL](https://github.com/kiyx/binhound/actions/workflows/codeql.yml/badge.svg)](https://github.com/kiyx/binhound/actions/workflows/codeql.yml)
[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE)

**Status: early development.** Only the ELF header analysis is implemented; component detection,
SBOM export and the rest are in progress. Built in public, step by step.

## What it does

Given a compiled binary, with no source code available, BinHound answers three questions:

1. **What is inside?** Components and their versions, each with evidence and a confidence level.
2. **Which cryptography is used?** Algorithms, protocols and certificates (planned).
3. **What is dangerous?** Known vulnerabilities, reported separately from the inventory (planned).

Results are exported as standard CycloneDX documents (SBOM, CBOM, VEX) plus a human-readable
report. Every report states its **coverage level**: how much of the file could actually be analyzed.

## What it is not

Not an antivirus, not an exploitation tool, not a decompiler. It never executes the analyzed file,
and it never treats "no findings" as "no risk".

## Usage

```bash
binhound scan /bin/ls
binhound --version
binhound --help
```

Current output:

```
File:       /bin/ls
Class:      ELF64
Endianness: little
Type:       DYN
Machine:    x86-64
Entry:      0x6d30
Sections:   31
```

Exit codes: `0` success, `2` error.

## Build

Requirements: CMake 3.28 or newer, a C++20 compiler (GCC 13+, Clang 18+), Ninja.

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug

cmake --preset release
cmake --build --preset release
cmake --install build/release --prefix "$HOME/.local"
```

Presets: `debug`, `relwithdebinfo`, `release` (LTO and hardening), `bench`, `asan`, `ci`.
The same checks run in CI on Linux, Windows and macOS, with sanitizers, clang-tidy,
clang-format, coverage and CodeQL.

## Project layout

```
binhound/
├── src/
│   ├── cli/          # command-line entry point
│   ├── util/         # file reading, endian-aware integers, errors
│   └── parser/elf/   # ELF header parsing
├── tests/
│   ├── unit/         # unit tests (doctest)
│   └── fixtures/     # generated test binaries
└── data/signatures/  # component signatures (in progress)
```

## Roadmap

- **v0.1** - ELF parsing, string and symbol extraction, signature detection, CycloneDX SBOM,
  coverage scorecard, colored text output.
- **v0.2** - cryptography inventory (CBOM), Go/Rust metadata, automated signature database.
- **v0.3** - vulnerability matching (OSV) and VEX.
- **v0.4** - CRA readiness report.
- **Later** - PE and firmware support, medical (DICOM) module.

## Contributing

Bug reports, small fixes, tests and feature proposals are welcome. Open an issue to discuss an
idea before working on it, and see [CONTRIBUTING.md](CONTRIBUTING.md) for the workflow, the
quality gates and the conventions.

## Design principles

- **Evidence first.** Every finding states why it was detected and how confident we are.
- **Honest coverage.** Reports state what could not be analyzed; low coverage is a result,
  not a failure.
- **Open and inspectable.** Apache-2.0, no closed detection logic.
- **Safe by default.** No code execution, no network access unless explicitly requested.

## License

Apache-2.0. See [LICENSE](LICENSE).
