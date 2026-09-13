<div align="center">

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="assets/banner-dark.svg">
  <img src="assets/banner.svg" alt="BinHound" width="760">
</picture>

[![CI](https://github.com/kiyx/binhound/actions/workflows/ci.yml/badge.svg)](https://github.com/kiyx/binhound/actions/workflows/ci.yml)
[![CodeQL](https://github.com/kiyx/binhound/actions/workflows/codeql.yml/badge.svg)](https://github.com/kiyx/binhound/actions/workflows/codeql.yml)
[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](#build)
[![PRs welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](CONTRIBUTING.md)
[![OpenSSF Scorecard](https://api.securityscorecards.dev/projects/github.com/kiyx/binhound/badge)](https://scorecard.dev/viewer/?uri=github.com/kiyx/binhound)

**[What it is](#what-it-is) · [Features](#features) · [Quick start](#quick-start) · [Usage](#usage) · [How it works](#how-it-works) · [Roadmap](#roadmap) · [Contributors](#contributors) · [Contributing](#contributing)**

</div>

---

## What it is

BinHound is a static analyzer for compiled binaries and firmware. Given a file with no source
code available, it reconstructs the software inventory: which components are bundled, which
versions can be proven, which cryptography is in use, and which known vulnerabilities apply —
each result with the evidence behind it and a confidence level.

It is built for the era of the **EU Cyber Resilience Act** and the **post-quantum migration**,
where knowing what is inside a product is a legal and practical requirement, not an option.

## Features

- **Component inventory (SBOM).** Libraries and versions detected from strings, symbols and
  binary fingerprints.
- **Cryptography inventory (CBOM).** Algorithms, protocols and certificates, for post-quantum
  planning.
- **Vulnerability matching.** Components mapped to OSV/CVE data and reported separately as VEX.
- **Evidence and confidence everywhere.** No silent guessing: every result says why it was
  detected and how certain it is.
- **Coverage scorecard.** Every report states how much of the file could actually be analyzed.
- **Standards-ready output.** CycloneDX SBOM, CBOM and VEX, plus a human-readable report.
- **Safe by default.** The analyzed file is never executed; no network access unless requested.

## Quick start

Requirements: CMake 3.28+, Ninja, a C++20 compiler (GCC 13+ or Clang 18+).

```bash
git clone https://github.com/kiyx/binhound.git
cd binhound
cmake --workflow --preset debug      # configure, build and test
```

Release build with LTO and hardening:

```bash
cmake --preset release
cmake --build --preset release
cmake --install build/release --prefix "$HOME/.local"
binhound --help
```

## Usage

<p align="center">
  <img src="assets/demo.png" alt="binhound scan /bin/ls" width="820">
</p>

```bash
binhound scan /bin/ls
```

```
File:       /bin/ls
Class:      ELF64
Endianness: little
Type:       DYN
Machine:    x86-64
Entry:      0x6d30
Sections:   31
```

Exit codes: `0` success, `1` findings, `2` error — suitable for scripts and CI.

## How it works

```
binary ──► parse ──► extract evidence ──► detect ──► report
           ELF      strings, symbols,     rules and   CycloneDX,
                    Build-ID               confidence  coverage, text
```

The core is a C++20 library (`binhound_core`); the command line is a thin adapter on top of it.
Every finding carries its evidence and confidence, and every report states its coverage.

## Design principles

- **Evidence first.** A result without evidence is not a result.
- **Honest coverage.** Reports state what could not be analyzed; low coverage is a finding,
  not a failure.
- **Open and inspectable.** Apache-2.0, no closed detection logic.
- **Safe by default.** No code execution, no network access unless explicitly requested.

## Roadmap

> **Status: early development.** ELF header analysis works today; component detection and
> SBOM export are the next milestone. Each milestone ends with a tagged release.

| Version | Focus |
| :--- | :--- |
| **v0.1** | ELF parsing, strings, symbols and Build-ID, signature detection, CycloneDX SBOM, coverage scorecard |
| v0.2 | CBOM, Go/Rust metadata, automated signature database |
| v0.3 | Vulnerability matching (OSV) and VEX |
| v0.4 | CRA readiness report |
| Later | PE and firmware support, medical (DICOM) module |

## Built with

- [doctest](https://github.com/doctest/doctest) - unit testing
- [tl::expected](https://github.com/TartanLlama/expected) - error handling
- [CMake](https://cmake.org) and [Ninja](https://ninja-build.org) - build system
- [CycloneDX](https://cyclonedx.org) - SBOM, CBOM and VEX formats
- [OpenSSF Scorecard](https://scorecard.dev) - repository security posture

## Contributors

Thanks to everyone who has contributed to BinHound.

<p align="center">
  <a href="https://github.com/kiyx/binhound/graphs/contributors">
    <img src="https://contrib.rocks/image?repo=kiyx/binhound" alt="Contributors">
  </a>
</p>

## Contributing

Bug reports, small fixes, tests, documentation and **feature proposals** are welcome. Open an
issue or start a discussion first; see [CONTRIBUTING.md](CONTRIBUTING.md) for the workflow and
the quality gates, and [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) for the community rules.

## Security

Please report vulnerabilities privately; see [SECURITY.md](SECURITY.md).

## License

Apache-2.0 — see [LICENSE](LICENSE). If you use BinHound in your work, see [CITATION.cff](CITATION.cff).
