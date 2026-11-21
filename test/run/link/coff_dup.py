#!/usr/bin/env python3
# writes coff_dup_a.lib and coff_dup_b.lib beside this script: two byte-identical archives of one
# x86-64 COFF member as zig writes compiler_rt, which coff_dup.mach links both of (Microsoft PE
# Format, Archive (Library) File Format, COFF Symbol Table, COMDAT Sections, Auxiliary Format 3:
# Weak Externals).
#
#   test/run/link/coff_dup.py
#
# the member, dup.obj:
#   section 1 .text  COMDAT NODUPLICATES: lea eax, [rcx+2] ; ret, padded to 16 with int3
#   `.weak.dup_fn.default`  external, defined at the start of .text
#   `dup_fn`                weak external, search alias, whose default is `.weak.dup_fn.default`
# the archive's linker member indexes both names at the member, as llvm-ar does. the bytes are a
# function of this script alone: no time stamp.
import os, struct, zlib

TEXT = 0x20 | 0x1000 | 0x00500000 | 0x20000000 | 0x40000000
NODUPLICATES = 1
WEAK_EXTERNAL = 0x69
SEARCH_ALIAS = 3

code = bytes([0x8D, 0x41, 0x02, 0xC3]).ljust(16, b'\xcc')
strings = b''


def name8(name):
    global strings
    if len(name) <= 8:
        return name.ljust(8, b'\0')
    offset = 4 + len(strings)
    strings += name + b'\0'
    return struct.pack('<II', 0, offset)


def symbol(name, value, section, typ, storage, aux=b''):
    return name8(name) + struct.pack('<IhHBB', value, section, typ, storage, len(aux) // 18) + aux


checksum = zlib.crc32(code, 0xFFFFFFFF) ^ 0xFFFFFFFF
symbols = [
    symbol(b'.text', 0, 1, 0, 3, struct.pack('<IHHIHB3x', len(code), 0, 0, checksum, 1, NODUPLICATES)),
    symbol(b'.weak.dup_fn.default', 0, 1, 0x20, 2),
    symbol(b'dup_fn', 0, 0, 0, WEAK_EXTERNAL, struct.pack('<II10x', 2, SEARCH_ALIAS)),
]
count = sum(len(s) // 18 for s in symbols)
raw = 20 + 40
table_at = raw + len(code)
head = struct.pack('<HHIIIHH', 0x8664, 1, 0, table_at, count, 0, 0)
section = b'.text'.ljust(8, b'\0') + struct.pack('<6I2HI', 0, 0, len(code), raw, 0, 0, 0, 0, TEXT)
obj = head + section + code + b''.join(symbols) + struct.pack('<I', 4 + len(strings)) + strings


def member(name, data):
    header = name.ljust(16) + b'0'.ljust(12) + b'0'.ljust(6) + b'0'.ljust(6) + b'644'.ljust(8) + str(len(data)).encode().ljust(10) + b'`\n'
    return header + data + (b'\n' if len(data) % 2 else b'')


names = [b'.weak.dup_fn.default', b'dup_fn']
index_size = 4 + 4 * len(names) + sum(len(n) + 1 for n in names)
at = 8 + 60 + index_size + index_size % 2
index = struct.pack('>I', len(names)) + struct.pack('>I', at) * len(names) + b''.join(n + b'\0' for n in names)
archive = b'!<arch>\n' + member(b'/', index) + member(b'dup.obj/', obj)
here = os.path.dirname(os.path.abspath(__file__))
for copy in ('a', 'b'):
    with open(os.path.join(here, f'coff_dup_{copy}.lib'), 'wb') as f:
        f.write(archive)
