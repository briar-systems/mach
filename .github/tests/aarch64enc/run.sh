#!/usr/bin/env bash
# aarch64enc integration test: byte-verify the aarch64-linux back end's emitted
# object and linked executable WITHOUT a qemu or any runnable aarch64 binary.
#
# the aarch64 target is complete through Phase 4 (compiles + links + correct
# scalar/FP/ABI) but the self-host fixpoint and a runtime to run binaries need
# qemu-user-static and the mach-std aarch64 runtime, neither of which is in this
# lane. this suite therefore proves the emitter statically: it cross-builds a
# tiny self-contained program (its own `_start`, no mach-std dependency) and
# inspects the bytes three ways:
#   1. instruction encoding — `llvm-objdump -d` shows the leaf prologue
#      (`stp x29, x30, [sp, #-0x10]!`), a body that calls (`bl`) and accesses a
#      global (`adrp`), and the `ret` epilogue, in that order; and, when llvm-mc
#      is present, the prologue/ret words are byte-identical to the assembler's.
#   2. relocations — `llvm-objdump -r` shows the four aarch64 reloc kinds the
#      back end emits: R_AARCH64_CALL26, R_AARCH64_ADR_PREL_PG_HI21,
#      R_AARCH64_LDST*_ABS_LO12_NC and R_AARCH64_ADD_ABS_LO12_NC.
#   3. object format — `readelf -h` shows Machine = AArch64 on both the object
#      and the linked EXEC, proving the reused (x86-shared) ELF writer stamps
#      EM_AARCH64 and the linker patches the aarch64 relocs into a real ELF exe.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
# absolutize a path argument: the suite cds into a temp dir, so a relative
# compiler path would break after the first cd.
case "$MACH" in */*) MACH="$(cd "$(dirname "$MACH")" && pwd)/$(basename "$MACH")";; esac
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# the byte-verification needs an aarch64 disassembler and an ELF header reader;
# skip cleanly (not fail) when the host lacks them, matching the other suites.
if ! command -v llvm-objdump >/dev/null 2>&1; then
    echo "SKIP aarch64enc: 'llvm-objdump' not available for the byte-verification"
    exit 0
fi
if ! command -v readelf >/dev/null 2>&1; then
    echo "SKIP aarch64enc: 'readelf' not available for the ELF-header check"
    exit 0
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
cp -r "$HERE/app" "$WORK/app"
APP="$WORK/app"

fail=0

# cross-compile to a relocatable object (stops before the link) and to a linked
# aarch64 executable (the own-`_start` makes the link runtime-free).
if ! ( cd "$APP" && "$MACH" build . --target arm64 --emit obj ) >"$WORK/obj.out" 2>&1; then
    echo "FAIL aarch64enc: cross-compile to object failed" >&2
    cat "$WORK/obj.out" >&2
    exit 1
fi
if ! ( cd "$APP" && "$MACH" build . --target arm64 ) >"$WORK/exe.out" 2>&1; then
    echo "FAIL aarch64enc: cross-link to executable failed" >&2
    cat "$WORK/exe.out" >&2
    exit 1
fi

OBJ="$(find "$APP/out" -name '*.o' -type f | head -1)"
EXE="$(find "$APP/out" -path '*/bin/*' -type f | head -1)"
test -n "$OBJ" || { echo "FAIL aarch64enc: no object produced" >&2; exit 1; }
test -n "$EXE" || { echo "FAIL aarch64enc: no executable produced" >&2; exit 1; }

# 3. object format: the reused ELF writer must stamp Machine = AArch64 on the
# object, and the linker must produce a real AArch64 EXEC.
if readelf -h "$OBJ" | grep -qi 'Machine:.*AArch64'; then
    echo "PASS aarch64enc: object ELF header is Machine = AArch64"
else
    echo "FAIL aarch64enc: object is not an AArch64 ELF" >&2
    readelf -h "$OBJ" >&2
    fail=1
fi
if readelf -h "$EXE" | grep -qi 'Machine:.*AArch64' && readelf -h "$EXE" | grep -qi 'Type:.*EXEC'; then
    echo "PASS aarch64enc: linked binary is an AArch64 EXEC ELF"
else
    echo "FAIL aarch64enc: linked binary is not an AArch64 EXEC" >&2
    readelf -h "$EXE" >&2
    fail=1
fi

# 1. instruction encoding: the ordered mnemonic stream of the object's .text.
mnem="$(llvm-objdump -d --no-show-raw-insn "$OBJ" | grep -E '^[[:space:]]+[0-9a-f]+:' | awk '{print $2}')"
first="$(printf '%s\n' "$mnem" | head -1)"
last="$(printf '%s\n' "$mnem" | tail -1)"

# the first instruction is the leaf prologue store-pair; the last is the return.
if [ "$first" = "stp" ]; then
    echo "PASS aarch64enc: first instruction is the prologue 'stp'"
else
    echo "FAIL aarch64enc: expected leading 'stp' prologue, got '$first'" >&2
    fail=1
fi
if [ "$last" = "ret" ]; then
    echo "PASS aarch64enc: last instruction is the 'ret' epilogue"
else
    echo "FAIL aarch64enc: expected trailing 'ret' epilogue, got '$last'" >&2
    fail=1
fi
# the body between prologue and epilogue must contain a call and a global access.
for want in bl adrp; do
    if printf '%s\n' "$mnem" | grep -qx "$want"; then
        echo "PASS aarch64enc: body emits '$want'"
    else
        echo "FAIL aarch64enc: body missing '$want'" >&2
        fail=1
    fi
done
# prologue must precede the first epilogue (stp before the first ret).
stp_line="$(printf '%s\n' "$mnem" | grep -nx stp | head -1 | cut -d: -f1)"
ret_line="$(printf '%s\n' "$mnem" | grep -nx ret | head -1 | cut -d: -f1)"
if [ -n "$stp_line" ] && [ -n "$ret_line" ] && [ "$stp_line" -lt "$ret_line" ]; then
    echo "PASS aarch64enc: prologue precedes epilogue (stp before ret)"
else
    echo "FAIL aarch64enc: prologue/epilogue order wrong (stp@$stp_line ret@$ret_line)" >&2
    fail=1
fi

# 1b. byte-identity cross-check against the reference assembler, when present:
# the mach-encoded prologue/ret words must equal llvm-mc's encoding of the same
# mnemonics. mc_word assembles one single-word instruction and returns its
# 32-bit encoding as the big-endian hex word llvm-objdump prints.
if command -v llvm-mc >/dev/null 2>&1; then
    mc_word() {
        printf '%s\n' "$1" | llvm-mc -triple=aarch64 --show-encoding 2>/dev/null \
            | sed -n 's/.*encoding: \[\(.*\)\].*/\1/p' | tr -d ' ' \
            | awk -F, '{ for (i = NF; i >= 1; i--) { b = $i; sub(/0x/, "", b); printf "%s", b } printf "\n" }'
    }
    words="$(llvm-objdump -d "$OBJ" | grep -oiE '^[[:space:]]+[0-9a-f]+:[[:space:]]+[0-9a-f]{8}' | awk '{print tolower($2)}')"
    check_word() {
        local w; w="$(mc_word "$1")"
        if [ -n "$w" ] && printf '%s\n' "$words" | grep -qx "$w"; then
            echo "PASS aarch64enc: '$1' encodes to 0x$w, byte-identical to llvm-mc"
        else
            echo "FAIL aarch64enc: '$1' word 0x$w not found in the object (encoder vs llvm-mc)" >&2
            fail=1
        fi
    }
    check_word 'stp x29, x30, [sp, #-16]!'
    check_word 'ret'
else
    echo "INFO aarch64enc: 'llvm-mc' not available; skipping the assembler byte-identity cross-check"
fi

# 2. relocations: the object must carry each aarch64 reloc kind the back end
# emits. LDST*_ABS_LO12_NC matches any access width (the suite forces a 64-bit
# global, so LDST64; the suffix is left open so a width change can't silently
# break the assertion).
relocs="$(llvm-objdump -r "$OBJ")"
assert_reloc() {
    if printf '%s\n' "$relocs" | grep -qE "$1"; then
        echo "PASS aarch64enc: relocation $2 present"
    else
        echo "FAIL aarch64enc: relocation $2 missing" >&2
        fail=1
    fi
}
assert_reloc 'R_AARCH64_CALL26'                  'R_AARCH64_CALL26 (call)'
assert_reloc 'R_AARCH64_ADR_PREL_PG_HI21'        'R_AARCH64_ADR_PREL_PG_HI21 (ADRP page)'
assert_reloc 'R_AARCH64_LDST[0-9]+_ABS_LO12_NC'  'R_AARCH64_LDST*_ABS_LO12_NC (folded load/store)'
assert_reloc 'R_AARCH64_ADD_ABS_LO12_NC'         'R_AARCH64_ADD_ABS_LO12_NC (address-of low-12)'

exit "$fail"
