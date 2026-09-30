#!/usr/bin/env python3
"""writes the Mach-O object the macho-objc-stubs-* link cases link (#4256).

it is what clang -c produces for

    extern void send(void *self, void *cmd) __asm__("_objc_msgSend$addItem:");
    void probe_send(void *self) { send(self, 0); }

that is a call of the selector stub `_objc_msgSend$addItem:`, the form apple
clang emits for a message send on arm64 and newer Xcode. the objects are checked
in so the case needs no clang or SDK on the runner. rebuild with

    python3 test/link/objc-probe.py aarch64 test/link/cases/macho-objc-stubs-aarch64/probe.o
    python3 test/link/objc-probe.py x86_64  test/link/cases/macho-objc-stubs-x86_64/probe.o
"""
import struct
import sys

SYMBOL = b"_objc_msgSend$addItem:"

CODE = {
    # stp x29, x30, [sp, #-16]!; mov x29, sp; bl <stub>; ldp x29, x30, [sp], #16; ret
    "aarch64": (0x0100000C, 0, struct.pack("<5I", 0xA9BF7BFD, 0x910003FD, 0x94000000, 0xA8C17BFD, 0xD65F03C0), 8, 2),
    # push rbp; mov rbp, rsp; call <stub>; pop rbp; ret
    "x86_64": (0x01000007, 3, bytes([0x55, 0x48, 0x89, 0xE5, 0xE8, 0, 0, 0, 0, 0x5D, 0xC3]), 5, 2),
}


def build(arch):
    cputype, cpusubtype, code, call_at, branch = CODE[arch]
    # nlist_64 entries: the defined function first, then the undefined stub
    strtab = b"\0_probe_send\0" + SYMBOL + b"\0"
    strx_send = 1
    strx_stub = strx_send + len(b"_probe_send") + 1
    nlists = struct.pack("<IBBHQ", strx_send, 0x0F, 1, 0, 0)
    nlists += struct.pack("<IBBHQ", strx_stub, 0x01, 0, 0, 0)
    reloc = struct.pack("<II", call_at, 1 | (1 << 24) | (2 << 25) | (1 << 27) | (branch << 28))

    hdr = 32
    seg_size = 72 + 80
    symtab_size = 24
    dysymtab_size = 80
    text_off = hdr + seg_size + symtab_size + dysymtab_size
    reloc_off = text_off + len(code)
    reloc_off += (-reloc_off) % 8
    sym_off = reloc_off + len(reloc)
    str_off = sym_off + len(nlists)

    sect = (b"__text".ljust(16, b"\0") + b"__TEXT".ljust(16, b"\0")
            + struct.pack("<QQIIIIIIII", 0, len(code), text_off, 2 if arch == "aarch64" else 0, reloc_off, 1,
                          0x80000400, 0, 0, 0))
    segment = (struct.pack("<II16sQQQQIIII", 0x19, seg_size, b"", 0, len(code), text_off, len(code), 7, 7, 1, 0) + sect)
    symtab = struct.pack("<IIIIII", 2, symtab_size, sym_off, 2, str_off, len(strtab))
    dysymtab = struct.pack("<II" + "I" * 18, 0xB, dysymtab_size, 0, 0, 0, 1, 1, 1, *([0] * 12))
    header = struct.pack("<IiiIIIII", 0xFEEDFACF, cputype, cpusubtype, 1, 3, seg_size + symtab_size + dysymtab_size,
                         0x2000, 0)
    out = header + segment + symtab + dysymtab + code
    out += b"\0" * (reloc_off - len(out)) + reloc + nlists + strtab
    return out


if __name__ == "__main__":
    with open(sys.argv[2], "wb") as f:
        f.write(build(sys.argv[1]))
