#!/usr/bin/env bash
# cross-compile the freestanding rv64 fixture, byte-verify the result with the llvm
# tools, then run it under qemu-riscv64 and assert its exit code. proves the riscv64
# backend emits a correct, fully linked RV64 ELF that runs to its inline-asm `exit`.
#
# usage: verify.sh [path-to-mach]   (defaults to `mach` on PATH)
#
# requires: llvm-readelf, llvm-objdump, llvm-readobj (unversioned or `-NN` suffixed)
# and qemu-riscv64 (or qemu-riscv64-static) for the exit-code run.
set -euo pipefail

# the computed exit code the fixture returns (sum 0..9 plus the const-shift call
# argument 1 << 3 = 8, i.e. 45 + 8), the qemu e2e asserts it.
expect_code=53

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
for mnem in auipc jalr ld sd addi sll ret ecall; do
    echo "$dis" | grep -qw "$mnem" || fail "expected mnemonic '$mnem' not in disassembly"
done

echo "verifying relocations in $obj"
rel="$("$readobj" -r "$obj")"
for r in R_RISCV_PCREL_HI20 R_RISCV_PCREL_LO12_I R_RISCV_PCREL_LO12_S R_RISCV_CALL_PLT; do
    echo "$rel" | grep -q "$r" || fail "expected relocation '$r' not emitted"
done

echo "running $exe under qemu-riscv64"
qemu="$(command -v qemu-riscv64 || command -v qemu-riscv64-static || true)"
[ -n "$qemu" ] || fail "qemu-riscv64 not found"
set +e
"$qemu" "$exe"
code=$?
set -e
[ "$code" -eq "$expect_code" ] || fail "exit code $code, expected $expect_code"

echo "OK: riscv64 backend emits a correct RV64 ELF that runs to exit code $expect_code"
