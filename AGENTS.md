# Agent guide

This document describes the project for automated assistants and new contributors who need to
understand the codebase quickly. Keep it accurate and short.

## Project

BinHound is a static analyzer for compiled binaries. It inventories the bundled software
components and, over time, the cryptography in use and the known vulnerabilities, reporting
evidence, confidence and coverage for every result. It never executes the analyzed file and never
claims completeness.

## Current state

- Implemented: ELF header parsing (ELF32 and ELF64, little and big endian), safe file reading,
  endian-aware integer access, the `scan` command, unit tests, and CI on Linux, Windows and macOS
  with sanitizers, clang-tidy, clang-format, coverage and CodeQL.
- Next: ELF sections, symbols and Build-ID; then string and symbol extraction, signature
  detection, CycloneDX SBOM, coverage scorecard and colored text output.
- Not implemented: component detection, SBOM export, PE support, firmware, network features.

## Layout

- `src/cli` - command-line entry point and exit codes (`0` ok, `1` findings, `2` error).
- `src/util` - file reading (`reader`), endian-aware integers (`bytes`), error type (`error`).
- `src/parser/elf` - ELF parsing: `header` today, sections, symbols and notes later.
- `tests/unit` - doctest unit tests. `tests/fixtures` - generated test binaries.
- `tests/fuzz` - libFuzzer harness; the seed corpus is generated at build time
  (`generate_corpus.py`), never committed.
- `data/signatures` - component signature database (in progress).

Targets: `binhound_core` is a static library with all logic; `binhound` is the executable;
`binhound_options` is an interface target carrying warnings, sanitizers, LTO and hardening.

## Build and test

```bash
cmake --workflow --preset debug    # configure, build and test
cmake --build --preset asan        # build with sanitizers
cmake --build --preset release     # build with LTO and hardening
```

## Conventions

- C++20. Prefer `std::span`, `std::string_view`, `std::filesystem`, `enum class`, `constexpr`,
  designated initializers, `[[nodiscard]]` and RAII. No owning raw pointers, no C-style casts.
- Errors are values: `binhound::Result<T>` (tl::expected) or `std::optional`. No exceptions across
  module boundaries.
- The core library performs no I/O: printing and exit codes live only in the CLI.
- Formatting: `.clang-format` (Allman braces, no space before parentheses). Static analysis:
  `.clang-tidy` with warnings as errors. Keep clangd diagnostics clean.
- Naming: files `snake_case`; types `PascalCase`; functions and variables `camelCase`; global
  constants `kPascalCase`; private members `name_`.

## Commits

Conventional commits (`feat:`, `fix:`, `refactor:`, `test:`, `build:`, `ci:`, `docs:`, `chore:`),
English, imperative mood, bullet points in the body. One logical change per commit; the tree must
build and pass tests at every commit.

## Quality gates before committing

```bash
cmake --workflow --preset debug
cmake --build --preset debug --target check   # format, clang-tidy, tests
```

The pre-push hook (`scripts/install-hooks.sh`) runs the same checks before every push.

## Scope discipline

- No dead code or placeholders: a file exists only when used and tested.
- New dependencies need a documented reason; prefer header-only libraries pinned via `FetchContent`.
- Documentation changes are limited to what the current milestone needs.
- When unsure about a practice, check authoritative sources and comparable projects first.
