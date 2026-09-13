#!/usr/bin/env python3
"""Generate the minimal ELF fixtures used by the smoke tests."""
import pathlib
import struct
import sys

OUT_DIR = pathlib.Path(sys.argv[1])
OUT_DIR.mkdir(parents=True, exist_ok=True)

header = struct.pack(
    "<16sHHIQQQIHHHHHH",
    bytes([0x7F]) + b"ELF" + bytes([2, 1, 1, 0]) + b"\x00" * 8,
    2, 62, 1,
    0x401000,
    64, 0,
    0,
    64, 56, 1,
    64, 0, 0,
)
assert len(header) == 64, len(header)
(OUT_DIR / "elf64_minimal.bin").write_bytes(header)
print(f"wrote {OUT_DIR / 'elf64_minimal.bin'}")
