# BinHound

> Working name. A static **SBOM / CBOM generator** and **CRA readiness checker** for compiled binaries and firmware.

**Status: early development. Nothing works yet. The project is being built in public, step by step.**

English | [Italiano](README.it.md)

## What it does

BinHound takes an executable file for which no source code is available (a stripped binary, a firmware image) and answers three questions:

1. **What is inside?** It identifies third-party components (libraries, tools) and their versions.
2. **What cryptography does it use?** It inventories algorithms, protocols and certificates (CBOM).
3. **What is dangerous?** It matches detected components against public vulnerability databases (CVE / OSV).

Results are exported as standard documents:

- **SBOM** in CycloneDX format (the software ingredient list)
- **CBOM** in CycloneDX format (`cryptoProperties`)
- **VEX** (which published vulnerabilities actually affect the product)
- a **human-readable report** with a CRA readiness checklist

Every finding carries **evidence** and a **confidence level** (high / medium / low). A single weak hint never produces a high-confidence result.

## Why

- The **EU Cyber Resilience Act** (Regulation 2024/2847) requires manufacturers selling in the EU to know and declare the software components of their products. Reporting obligations start on **11 September 2026**; full conformity is required by **11 December 2027**. Fines reach 15 million EUR or 2.5% of global turnover.
- Hospitals and public buyers increasingly refuse products without an SBOM; 35% of them report this as a procurement requirement.
- The migration to **post-quantum cryptography** requires knowing, in advance, where RSA and ECC are used. You cannot migrate what you cannot inventory. Banks and public administrations are starting this work now.

## What it is not

- Not an antivirus or malware scanner.
- Not an exploitation or penetration testing tool.
- Not a decompiler: it does not reconstruct source code.
- It never executes the analyzed file.
- It does not promise completeness. Binary analysis has hard limits; the tool documents them instead of hiding them.
- It never treats "no findings" as "no risk": every report states how much of the file could be analyzed.

## Example (planned output, not implemented yet)

```
$ binhound scan firmware.bin --format cyclonedx --output sbom.json

BinHound 0.1.0 - static component analysis
File      : firmware.bin (ELF 64-bit LSB executable, x86-64, stripped)
Scanned   : 1.2 MB in 0.42 s

Components (12 found)
  openssl   3.0.8     high     string "OpenSSL 3.0.8" at 0x000a1c30
  zlib      1.2.11    high     string "inflate 1.2.11"  at 0x000b0431
  busybox   1.35.0    medium   3 symbol matches
  ...

Vulnerabilities (4 found: 1 critical, 2 high, 1 medium)
  CVE-2023-0286   openssl 3.0.8   critical
  ...

CRA readiness
  [ok]    SBOM generated
  [warn]  Update mechanism not verifiable from the binary
  [todo]  Vulnerability disclosure policy not part of the artifact

Output: sbom.json (CycloneDX 1.6, schema-valid)
```

## How it works

```
 input file
     |
     v
+------------+    +-------------+    +----------------+    +-------------+
|  Parsers   | -> |  Detection  | -> |   Matching     | -> |   Reports   |
| ELF (v0.1) |    | signatures  |    | CVE / OSV      |    | CDX, VEX    |
| PE, FW     |    | + evidence  |    | + severity     |    | + CRA check |
+------------+    +-------------+    +----------------+    +-------------+
```

1. **Parsing**: the file is read as raw bytes; headers, sections and symbol tables are decoded. Nothing is executed.
2. **Detection**: a signature database maps strings, symbol names and binary constants to components, versions and confidence weights. Component identification and version attribution are separate steps; every match is recorded as evidence.
3. **Matching**: detected components and versions are matched against vulnerability databases.
4. **Reporting**: results are emitted as CycloneDX SBOM, CBOM and VEX documents, plus a readable compliance report.

## Repository layout (planned)

```
binhound/
├── CMakeLists.txt
├── src/
│   ├── cli/          # command line interface
│   ├── parser/       # binary formats (ELF first)
│   ├── detect/       # signature engine + confidence + evidence
│   ├── match/        # vulnerability matching (OSV / local cache)
│   ├── report/       # CycloneDX SBOM, CBOM, VEX, human report
│   └── util/         # bytes, endianness, strings, errors
├── data/
│   └── signatures/   # component signature database
├── tests/
│   └── fixtures/     # small binaries with known content
└── docs/
    ├── REQUIREMENTS.md
    ├── ROADMAP.md
    ├── RISKS.md
    ├── FORGE.md
    ├── LIMITATIONS.md
    └── STUDY-PLAN.it.md
```

## Requirements summary

See [docs/REQUIREMENTS.md](docs/REQUIREMENTS.md) for the full list. In short:

- C++20, CMake, minimal external dependencies (no mandatory runtime dependencies)
- Linux x86-64 first; Windows and macOS later
- ELF first (v0.1): 32/64-bit, little- and big-endian, architecture-agnostic (x86-64, ARM, MIPS); PE and firmware later
- Measured precision and coverage published with each release; every rule ships with positive/negative fixtures
- Coverage scorecard in every output: what was analyzable, and what was not
- Deterministic, schema-valid outputs
- Safe parsing of untrusted input; no crash on malformed files
- CI with tests, sanitizers, warnings-as-errors and schema validation

## Roadmap

See [docs/ROADMAP.md](docs/ROADMAP.md) for details.

| Version | Focus |
| :--- | :--- |
| v0.1 | Walking skeleton: ELF parsing, signature detection, CycloneDX SBOM, CLI, tests, CI |
| v0.2 | CBOM + embedded metadata (Go/Rust) + signature forge |
| v0.3 | Vulnerability matching (OSV) + VEX + confidence scoring |
| v0.4 | CRA readiness report |
| v0.5 | PE support + firmware handled via external tools (binwalk/unblob) |
| v0.6 | Medical module (DICOM, authorized targets only) |
| v1.0 | Stable database format, packaging, fuzzing, benchmarks |

Versions are targets, not promises. Each milestone ends with a tagged release and updated documentation.

## Building (planned, not implemented yet)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/bin/binhound scan /bin/ls --format cyclonedx --output sbom.json
```

## Contributing

- The signature database is designed to accept contributions without any C++ knowledge.
- Good first issues will be published with the first code release.
- Before contributing, read [docs/REQUIREMENTS.md](docs/REQUIREMENTS.md) and [docs/ROADMAP.md](docs/ROADMAP.md).

## License

**Apache-2.0.** Everything that determines detection results — rules, confidence computation, evidence — is open and inspectable: for a tool whose value is auditable output, closed detection logic would contradict its purpose. If a business model ever exists, it lives outside the code: support, consulting, training.
