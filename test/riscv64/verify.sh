#!/usr/bin/env bash
# cross-compile the freestanding rv64 fixture and byte-verify the result with the
# llvm tools. proves the riscv64 backend emits a correct, fully linked RV64 ELF
# without running it (no exit syscall path exists until the inline-asm slice lands).
#
# usage: verify.sh [path-to-mach]   (defaults to `mach` on PATH)
#
# requires: llvm-readelf, llvm-objdump, llvm-readobj (unversioned or `-NN` suffixed).
set -euo pipefail

mach="${1:-mach}"
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$here"

fail() { echo "FAIL: $1" >&2; exit 1; }

# resolve an llvm tool by its unversioned name, falling back to the highest
# `-NN` suffixed variant on PATH (ubuntu ships e.g. llvm-objdump-18).
resolve_tool() {
    local base="$1" cand
    if command -v "$base" >/dev/null 2>&1; then echo "$base"; return 0; fi
    cand="$(compgen -c "$base-" | sort -V | tail -n 1)"
    if [ -n "$cand" ] && command -v "$cand" >/dev/null 2>&1; then echo "$cand"; return 0; fi
    return 1
}

readelf="$(resolve_tool llvm-readelf)" || fail "llvm-readelf not found"
objdump="$(resolve_tool llvm-objdump)" || fail "llvm-objdump not found"
readobj="$(resolve_tool llvm-readobj)" || fail "llvm-readobj not found"

target="linux-riscv64"
exe="out/$target/rvprobe"

echo "cross-compiling fixture for $target with $mach"
rm -rf out
"$mach" build . --target "$target" --profile debug
obj="$(find out -name '*.o' -print -quit)"

echo "verifying elf header of $exe"
hdr="$("$readelf" -h "$exe")"
echo "$hdr" | grep -q 'Class:.*ELF64'            || fail "exe is not ELFCLASS64"
echo "$hdr" | grep -q 'Machine:.*RISC-V'         || fail "exe e_machine is not EM_RISCV"
echo "$hdr" | grep -q 'Type:.*EXEC'              || fail "exe is not ET_EXEC"
echo "$hdr" | grep -qE 'Entry point address:.*0x[0-9a-fA-F]+' || fail "exe has no entry point"
echo "$hdr" | grep -q 'Entry point address:.*0x0$' && fail "exe entry point is zero"

echo "verifying disassembly of $exe"
dis="$("$objdump" -d "$exe")"
echo "$dis" | grep -q 'file format elf64-littleriscv' || fail "objdump did not parse a little-endian rv64 elf"
echo "$dis" | grep -qi '<unknown>'                    && fail "objdump found an unknown instruction word"
for mnem in auipc jalr ld sd addi ret; do
    echo "$dis" | grep -qw "$mnem" || fail "expected mnemonic '$mnem' not in disassembly"
done

echo "verifying relocations in $obj"
rel="$("$readobj" -r "$obj")"
for r in R_RISCV_PCREL_HI20 R_RISCV_PCREL_LO12_I R_RISCV_PCREL_LO12_S R_RISCV_CALL_PLT; do
    echo "$rel" | grep -q "$r" || fail "expected relocation '$r' not emitted"
done

echo "OK: riscv64 backend emits a correct, fully linked RV64 ELF"
