#!/usr/bin/env python3
# writes coff_unwind.obj beside this script: an x86-64 COFF object as MinGW and zig write unwind
# data, which coff_unwind.mach links (Microsoft PE Format, COFF File Header, Section Table,
# COFF Symbol Table, COMDAT Sections, The .pdata Section, x64 exception handling).
#
#   test/run/link/coff_unwind.py
#
# sections:
#   1 .used     COMDAT ANY, keyed by `used_fn`: lea eax, [rcx+1] ; ret, padded to 16 with int3
#   2 .unused   COMDAT ANY, keyed by `unused_fn`: mov eax, ecx ; imul eax, ecx ; ret, padded alike
#   3 .xdata$   COMDAT ANY with no symbol naming it: one UNWIND_INFO per function
#   4 .pdata$   COMDAT ANY with no symbol naming it: two function table entries, the unused
#               function's first, each relocated against its function, then the used one's
#   5 .drectve  `-exclude-symbols:unused_fn,used_fn`
# the code sections take names of their own, so each lands in an output section a dump names.
# every section definition states its checksum and number as zig's and llvm's do, which mink's
# reader takes .drectve's at alone. the bytes are a function of this script alone: no time stamp.
import os, struct, zlib

ADDR32NB = 3
TEXT = 0x20 | 0x1000 | 0x00500000 | 0x20000000 | 0x40000000
DATA = 0x40 | 0x1000 | 0x00300000 | 0x40000000
INFO = 0x200 | 0x800 | 0x00100000
ANY = 2


def pad16(code):
    return code + b'\xcc' * (16 - len(code))


# (name, characteristics, bytes, relocations as (offset, symbol index))
used = pad16(bytes([0x8D, 0x41, 0x01, 0xC3]))
unused = pad16(bytes([0x89, 0xC8, 0x0F, 0xAF, 0xC1, 0xC3]))
xdata = bytes([1, 0, 0, 0]) * 2
# symbol indices: 0 .used, 2 used_fn, 3 .unused, 5 unused_fn, 6 .xdata$, 8 .pdata$, 10 .drectve
pdata = struct.pack('<6I', 0, 16, 4, 0, 16, 0)
pdata_relocs = [(0, 5), (4, 5), (8, 6), (12, 2), (16, 2), (20, 6)]
drectve = b' -exclude-symbols:unused_fn,used_fn'
sections = [
    (b'.used', TEXT, used, []),
    (b'.unused', TEXT, unused, []),
    (b'.xdata$', DATA, xdata, []),
    (b'.pdata$', DATA, pdata, pdata_relocs),
    (b'.drectve', INFO, drectve, []),
]

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


def section_aux(number):
    # the definition of section `number` (from 1) as zig and llvm write it: its size and counts,
    # the CRC-32 of its bytes (reflected IEEE polynomial from zero, no final complement) and its own number
    _, characteristics, data, relocs = sections[number - 1]
    selection = ANY if characteristics & 0x1000 else 0
    checksum = zlib.crc32(data, 0xFFFFFFFF) ^ 0xFFFFFFFF
    return struct.pack('<IHHIHB3x', len(data), len(relocs), 0, checksum, number, selection)


symbols = [
    symbol(b'.used', 0, 1, 0, 3, section_aux(1)),
    symbol(b'used_fn', 0, 1, 0x20, 2),
    symbol(b'.unused', 0, 2, 0, 3, section_aux(2)),
    symbol(b'unused_fn', 0, 2, 0x20, 2),
    symbol(b'.xdata$', 0, 3, 0, 3, section_aux(3)),
    symbol(b'.pdata$', 0, 4, 0, 3, section_aux(4)),
    symbol(b'.drectve', 0, 5, 0, 3, section_aux(5)),
]
count = sum(len(s) // 18 for s in symbols)

at = 20 + 40 * len(sections)
headers, body = b'', b''
for name, characteristics, data, relocs in sections:
    raw = at
    at += len(data)
    reloc_at = at if relocs else 0
    at += 10 * len(relocs)
    headers += name.ljust(8, b'\0') + struct.pack('<6I2HI', 0, 0, len(data), raw, reloc_at, 0, len(relocs), 0, characteristics)
    body += data + b''.join(struct.pack('<IIH', off, sym, ADDR32NB) for off, sym in relocs)

table = b''.join(symbols)
head = struct.pack('<HHIIIHH', 0x8664, len(sections), 0, at, count, 0, 0)
obj = head + headers + body + table + struct.pack('<I', 4 + len(strings)) + strings
with open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'coff_unwind.obj'), 'wb') as f:
    f.write(obj)
