#!/usr/bin/env bash
. "$(dirname "$0")/common.sh"

# produce_macho_objc_stubs <engine> <leg> <binary>
# A call of `_objc_msgSend$<selector>` is a selector stub the link synthesizes
# (#4256), as ld64 does. Read the image back and follow the call the way the CPU
# would: the call must land on the first stub in __TEXT,__objc_stubs, the stub's
# load must read a cell of __DATA,__objc_selrefs, that cell must hold the address
# of the NUL-terminated selector in __RODATA,__objc_methname (the segment this linker gives read-only data), and the stub's jump
# must go through a GOT slot dyld binds to libobjc's `_objc_msgSend`. The selector
# cell holds an address the image's slide moves, so it must also carry a rebase
# row. The program sends to nil, so a native leg runs it and it must exit 0.
produce_macho_objc_stubs() {
    leg=$2
    bin=$3

    facts=$(python3 - "$bin" <<'EOF'
import struct
import sys

data = open(sys.argv[1], "rb").read()
magic, cputype, _, _, ncmds = struct.unpack_from("<IiiII", data, 0)
assert magic == 0xFEEDFACF
arm64 = cputype == 0x0100000C

sections = {}
off = 32
for _ in range(ncmds):
    cmd, size = struct.unpack_from("<II", data, off)
    if cmd == 0x19:
        nsects = struct.unpack_from("<I", data, off + 64)[0]
        for k in range(nsects):
            sh = off + 72 + k * 80
            name = data[sh:sh + 16].split(b"\0")[0].decode()
            seg = data[sh + 16:sh + 32].split(b"\0")[0].decode()
            addr, length, foff = struct.unpack_from("<QQI", data, sh + 32)
            sections[(seg, name)] = (addr, length, foff)
    off += size


def file_at(va, length=1):
    for addr, size, foff in sections.values():
        if addr <= va and va + length <= addr + size:
            return foff + va - addr
    raise SystemExit("link: macho-objc-stubs: address 0x%x is in no section" % va)


def need(seg, name):
    if (seg, name) not in sections:
        raise SystemExit("link: macho-objc-stubs: image has no %s,%s" % (seg, name))
    return sections[(seg, name)]


text = need("__TEXT", "__text")
stubs = need("__TEXT", "__objc_stubs")
selrefs = need("__DATA", "__objc_selrefs")
methname = need("__RODATA", "__objc_methname")
stub = stubs[0]
if stubs[1] != (32 if arm64 else 16):
    raise SystemExit("link: macho-objc-stubs: __objc_stubs holds %d bytes, expected one stub" % stubs[1])
if selrefs[1] != 8:
    raise SystemExit("link: macho-objc-stubs: __objc_selrefs holds %d bytes, expected one cell" % selrefs[1])

# the call in __text reaches the stub
reaches = False
body = data[text[2]:text[2] + text[1]]
if arm64:
    for i in range(0, len(body) - 3, 4):
        word = struct.unpack_from("<I", body, i)[0]
        if word >> 26 == 0x25:
            imm = word & 0x3FFFFFF
            imm -= (imm & 0x2000000) << 1
            reaches |= text[0] + i + imm * 4 == stub
else:
    for i in range(len(body) - 4):
        if body[i] == 0xE8:
            rel = struct.unpack_from("<i", body, i + 1)[0]
            reaches |= text[0] + i + 5 + rel == stub
print("call=reaches-stub" if reaches else "call=elsewhere")

# the stub loads a selref cell and jumps through a GOT slot
sf = stubs[2]
if arm64:
    def page(word, pc):
        imm = ((word >> 29) & 3) | (((word >> 5) & 0x7FFFF) << 2)
        imm -= (imm & 0x100000) << 1
        return (pc & ~0xFFF) + imm * 4096

    w = struct.unpack_from("<8I", data, sf)
    ok = (w[0] & 0x9F00001F) == 0x90000001 and (w[1] & 0xFFC003FF) == 0xF9400021
    ok &= (w[2] & 0x9F00001F) == 0x90000010 and (w[3] & 0xFFC003FF) == 0xF9400210
    ok &= w[4] == 0xD61F0200
    if not ok:
        raise SystemExit("link: macho-objc-stubs: the stub is not adrp/ldr x1, adrp/ldr x16, br x16")
    cell = page(w[0], stub) + ((w[1] >> 10) & 0xFFF) * 8
    slot = page(w[2], stub + 8) + ((w[3] >> 10) & 0xFFF) * 8
else:
    if data[sf:sf + 3] != b"\x48\x8b\x35" or data[sf + 7:sf + 9] != b"\xff\x25":
        raise SystemExit("link: macho-objc-stubs: the stub is not mov rsi, [rip+d]; jmp [rip+d]")
    cell = stub + 7 + struct.unpack_from("<i", data, sf + 3)[0]
    slot = stub + 13 + struct.unpack_from("<i", data, sf + 9)[0]
print("stub=_objc_msgSend$addItem:")
if cell != selrefs[0]:
    raise SystemExit("link: macho-objc-stubs: the stub loads 0x%x, not the selector cell 0x%x" % (cell, selrefs[0]))

# the cell points at the selector text
target = struct.unpack_from("<Q", data, selrefs[2])[0]
name = data[file_at(target):].split(b"\0")[0].decode()
if not (methname[0] <= target < methname[0] + methname[1]):
    raise SystemExit("link: macho-objc-stubs: the selector cell points outside __objc_methname")
print("selref=%s" % name)
print("CELL=0x%X" % cell)
print("SLOT=0x%X" % slot)
EOF
    ) || return 2
    cell=$(printf '%s\n' "$facts" | sed -n 's/^CELL=//p')
    slot=$(printf '%s\n' "$facts" | sed -n 's/^SLOT=//p')

    rebases=$(macho_objdump --macho --rebase "$bin") || return 2
    rebased=no
    printf '%s\n' "$rebases" | grep -qi "__DATA.* $cell " && rebased=yes
    binds=$(macho_objdump --macho --bind "$bin") || return 2
    bound=missing
    printf '%s\n' "$binds" | grep -i " $slot .*libobjc.* _objc_msgSend\$" >/dev/null && bound=libobjc-bind
    printf '%s\n' "$facts" | grep -v '^\(CELL\|SLOT\)='
    echo "selref_rebased=$rebased"
    echo "msgsend=$bound"

    if [ "$leg" = aarch64-darwin ]; then
        out=$(mktemp)
        run_captured native "$leg" "$bin" "$out" || { rm -f "$out"; return 1; }
        rm -f "$out"
        [ "$run_status" -eq 0 ] || {
            report_run_failure "macho-objc-stubs: the native image" "$run_status" "$run_out"
            return 1
        }
    fi
}

produce_macho_objc_stubs "$@"
