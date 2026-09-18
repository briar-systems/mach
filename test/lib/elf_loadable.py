#!/usr/bin/env python3
"""Re-lay a freestanding ELF executable so qemu-user can map it.

A freestanding image places each segment at its address with no regard for file
offsets, which is what a flash or bare-metal loader wants and what an mmap-based
loader refuses. This writes the same loaded bytes as one read-write-execute
PT_LOAD that starts at file offset 0 with the headers in the page below the
image, as a hosted linker lays one out, keeping the entry point and the flags.
Only the loader's view changes: every byte the program sees is the linker's.

usage: elf_loadable.py <in> <out>
"""
import os
import struct
import sys

PAGE = 0x1000
# the lowest address a host lets a user process map
MMAP_MIN = 0x10000
PT_LOAD = 1
PF_RWX = 7

# per class: ehdr fields after e_ident, phdr layout and sizes
CLASSES = {
    1: ("HHIIIIIHHHHHH", "IIIIIIII", 52, 32, 40),
    2: ("HHIQQQIHHHHHH", "IIQQQQQQ", 64, 56, 64),
}


def fail(msg):
    sys.stderr.write("elf_loadable: " + msg + "\n")
    sys.exit(1)


def phdr(cls, fields):
    # the 64-bit layout moves p_flags up beside p_type
    p_type, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_flags, p_align = fields
    if cls == 1:
        return (p_type, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_flags, p_align)
    return (p_type, p_flags, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_align)


def main():
    if len(sys.argv) != 3:
        fail("usage: elf_loadable.py <in> <out>")
    data = open(sys.argv[1], "rb").read()
    if len(data) < 16 or data[:4] != b"\x7fELF" or data[4] not in CLASSES or data[5] != 1:
        fail("not a little-endian ELF file")
    cls = data[4]
    ehdr_fmt, phdr_fmt, ehsize, phentsize, shentsize = CLASSES[cls]
    if len(data) < ehsize:
        fail("the ELF header does not fit the file")
    (e_type, e_machine, e_version, e_entry, e_phoff, _shoff, e_flags, _ehsize,
     e_phentsize, e_phnum, _shentsize, _shnum, _shstrndx) = struct.unpack_from("<" + ehdr_fmt, data, 16)
    if e_phentsize != phentsize or e_phoff + e_phnum * phentsize > len(data):
        fail("program header table does not fit the file")
    loads = []
    for i in range(e_phnum):
        raw = struct.unpack_from("<" + phdr_fmt, data, e_phoff + i * phentsize)
        if cls == 1:
            p_type, p_offset, p_vaddr, _paddr, p_filesz, p_memsz, _flags, _align = raw
        else:
            p_type, _flags, p_offset, p_vaddr, _paddr, p_filesz, p_memsz, _align = raw
        if p_type != PT_LOAD or p_memsz == 0:
            continue
        if p_filesz > p_memsz or p_offset + p_filesz > len(data):
            fail("segment %d does not fit the file" % i)
        loads.append((p_vaddr, p_offset, p_filesz, p_memsz))
    if not loads:
        fail("no loadable segment")
    low = min(v for v, _, _, _ in loads) & ~(PAGE - 1)
    if low - PAGE < MMAP_MIN:
        fail("the image must be based at 0x%x or above to leave a page for the headers" % (MMAP_MIN + PAGE))
    file_end = max(v + f for v, _, f, _ in loads)
    mem_end = max(v + m for v, _, _, m in loads)
    image = bytearray(file_end - low)
    for v, o, f, _ in sorted(loads):
        image[v - low:v - low + f] = data[o:o + f]
    header = bytearray(PAGE)
    header[:16] = data[:16]
    struct.pack_into("<" + ehdr_fmt, header, 16, e_type, e_machine, e_version, e_entry,
                     ehsize, 0, e_flags, ehsize, phentsize, 1, shentsize, 0, 0)
    struct.pack_into("<" + phdr_fmt, header, ehsize,
                     *phdr(cls, (PT_LOAD, 0, low - PAGE, low - PAGE, PAGE + len(image),
                                 PAGE + mem_end - low, PF_RWX, PAGE)))
    with open(sys.argv[2], "wb") as out:
        out.write(header)
        out.write(image)
    # qemu-user refuses a file without execute permission as a format error
    os.chmod(sys.argv[2], 0o755)


main()
