#!/usr/bin/env python3
"""Generate a deterministic libFuzzer seed corpus for the ELF header harness.

Usage: python3 generate_corpus.py <out-dir>

Seeds are generated instead of committed so the repository stays free of
binary blobs (see also tests/fixtures/generate_fixtures.py).
Requires only the standard library.
"""

import random
import struct
import sys
from pathlib import Path


def elf64_minimal() -> bytes:
    """64-byte minimal ELF64 little-endian header (valid magic, class, data)."""
    ident = b"\x7fELF" + bytes([2, 1, 1, 0]) + bytes(8)
    return ident + struct.pack(
        "<HHIQQQIHHHHHH",
        2,  # e_type: EXEC
        0x3E,  # e_machine: x86-64
        1,  # e_version
        0x400000,  # e_entry
        64,  # e_phoff
        0,  # e_shoff
        0,  # e_flags
        64,  # e_ehsize
        56,  # e_phentsize
        0,  # e_phnum
        64,  # e_shentsize
        0,  # e_shnum
        0,  # e_shstrndx
    )


def truncated() -> bytes:
    """10-byte input: valid ELF magic but shorter than the 16-byte ident."""
    return b"\x7fELF" + bytes([2, 1, 1, 0, 0, 0])


def random_blob() -> bytes:
    """100 deterministic pseudo-random bytes (fixed seed)."""
    rng = random.Random(0xB17C0D3)
    return bytes(rng.getrandbits(8) for _ in range(100))


def main() -> int:
    if len(sys.argv) != 2:
        print(f"usage: {sys.argv[0]} <out-dir>", file=sys.stderr)
        return 2
    out = Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    seeds = {
        "elf64.bin": elf64_minimal(),
        "truncated.bin": truncated(),
        "random.bin": random_blob(),
    }
    for name, data in seeds.items():
        (out / name).write_bytes(data)
        print(f"wrote {out / name} ({len(data)} bytes)")
    assert len(elf64_minimal()) == 64
    assert len(truncated()) == 10
    assert len(random_blob()) == 100
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
