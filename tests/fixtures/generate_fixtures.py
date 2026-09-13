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

# ELF64 with a section table: null, .text, .note (GNU Build-ID), .shstrtab.
# Used by the --verbose smoke test.
header = struct.pack(
    "<16sHHIQQQIHHHHHH",
    bytes([0x7F]) + b"ELF" + bytes([2, 1, 1, 0]) + b"\x00" * 8,
    2, 62, 1,
    0x401000,
    0, 64,
    0,
    64, 56, 0,
    64, 4, 3,
)
assert len(header) == 64, len(header)


def shdr(name, type, flags, offset, size, link=0, info=0, align=1, entsize=0):
    return struct.pack("<IIQQQQIIQQ", name, type, flags, 0, offset, size, link, info, align,
                       entsize)


blob = bytearray(header)
blob += shdr(0, 0, 0, 0, 0) + shdr(1, 1, 6, 320, 16) + shdr(7, 7, 2, 336, 36) + shdr(
    13, 3, 0, 372, 23)
assert len(blob) == 320, len(blob)
blob += b"\x00" * 16
blob += struct.pack("<III", 4, 20, 3) + b"GNU\x00" + bytes(range(1, 21))
blob += b"\x00.text\x00.note\x00.shstrtab\x00"
assert len(blob) == 395, len(blob)
(OUT_DIR / "elf64_sections.bin").write_bytes(bytes(blob))
print(f"wrote {OUT_DIR / 'elf64_sections.bin'}")
