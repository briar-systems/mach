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

# the computed exit code the fixture returns, low 8 bits of the running sum: sum 0..9
# (45) plus the const-shift call argument 1 << 3 (8), the stack-local frame-slot probe
# (#1670), the >4KiB long-branch relaxation probe (#1666), the 32-bit bitwise
# word-group probe (#1672), the lp64d register-plus-stack split probe (#1637, 35 =
# 1+..+7 + struct halves 2 + 5), the lp64d mixed float+integer probe (#1637 case A, 7 =
# f 2.0 in fa0 + i 5 in a0), the lp64d different-width float-pair probe (#1637 case B,
# 12 = a 4.0 in fa0 + b 8.0 in fa1), the lp64d sub-word mixed probe (#1637 case A
# sub-word, 9 = f 3.0 in fa0 + j 6 in a0 at offset 4, the 4-byte GP chunk), and the
# RV64A atomics probe (#1668, 18 = swapped 7 + post-rmw cell 11), wrapping to 133. the
# qemu e2e asserts the exact code, so a regression in any of those changes it.
expect_code=133

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

# the disassembly checks read from here-strings rather than `echo "$x" | grep`: under
# `pipefail` a `grep -q` that matches early closes the pipe, and on a large
# disassembly the producing `echo` then dies with SIGPIPE and fails the pipeline
# despite the match. a here-string has no pipe writer to kill.
echo "verifying elf header of $exe"
hdr="$("$readelf" -h "$exe")"
grep -q 'Class:.*ELF64'            <<< "$hdr" || fail "exe is not ELFCLASS64"
grep -q 'Machine:.*RISC-V'         <<< "$hdr" || fail "exe e_machine is not EM_RISCV"
grep -q 'Type:.*EXEC'              <<< "$hdr" || fail "exe is not ET_EXEC"
grep -qE 'Entry point address:.*0x[0-9a-fA-F]+' <<< "$hdr" || fail "exe has no entry point"
grep -q 'Entry point address:.*0x0$' <<< "$hdr" && fail "exe entry point is zero"

# the backend emits M (mul / div / rem), F/D (scalar float), and A (atomics)
# instructions and now writes a `.riscv.attributes` ISA-string section (#1673), so
# objdump reads the arch string off the section and decodes the full instruction set
# with no `--mattr` on the command line. the `<unknown>` guard then catches a
# genuinely malformed word rather than a correctly-encoded M / F / D / A instruction.
echo "verifying disassembly of $exe"
dis="$("$objdump" -d "$exe")"
grep -q 'file format elf64-littleriscv' <<< "$dis" || fail "objdump did not parse a little-endian rv64 elf"
grep -qi '<unknown>'                    <<< "$dis" && fail "objdump found an unknown instruction word"
for mnem in auipc jalr ld sd addi sll ret ecall; do
    grep -qw "$mnem" <<< "$dis" || fail "expected mnemonic '$mnem' not in disassembly"
done
# the RV64A atomics the probe emits must disassemble as the real instructions -
# decoded here with no `--mattr`, so this also proves the attributes section took.
for mnem in lr.d sc.d amoadd.d; do
    grep -qw "$mnem" <<< "$dis" || fail "expected RV64A mnemonic '$mnem' not in disassembly"
done

# the `.riscv.attributes` section the build now emits (#1673) is what lets objdump
# decode imafd above with no `--mattr`. assert it is present and carries the arch
# string naming the integer base plus the M / A / F / D extensions the backend uses,
# in both the linked exe and the relocatable object.
echo "verifying .riscv.attributes ISA string in $exe and $obj"
for f in "$exe" "$obj"; do
    attrs="$("$readelf" -A "$f")"
    grep -q 'riscv'              <<< "$attrs" || fail "$f has no .riscv.attributes vendor section"
    grep -qE 'rv64i.*m.*a.*f.*d' <<< "$attrs" || fail "$f Tag_RISCV_arch missing an imafd extension"
done

# assert the >4KiB probe forced long-branch relaxation (#1666): an inverted guard
# that skips exactly its own +8 (the trampoline word) immediately followed by an
# unconditional `j` to a target beyond the B-type +-4KiB range. without relaxation
# the build would have failed to encode, so this confirms the relaxed form shipped.
echo "verifying long-branch relaxation in $exe"
mapfile -t dlines < <(printf '%s\n' "$dis")
relax_found=0
for ((li=0; li<${#dlines[@]}-1; li++)); do
    l1="${dlines[li]}"; l2="${dlines[li+1]}"
    grep -qE $'\t(beqz|bnez|bltz|bgez|blez|bgtz|beq|bne|blt|bge|bltu|bgeu)\t' <<< "$l1" || continue
    grep -qE $'\tj\t' <<< "$l2" || continue
    a1="$(echo "${l1%%:*}" | tr -d '[:space:]')"
    aj="$(echo "${l2%%:*}" | tr -d '[:space:]')"
    [[ "$a1" =~ ^[0-9a-fA-F]+$ && "$aj" =~ ^[0-9a-fA-F]+$ ]] || continue
    t1="$(echo "$l1" | grep -oE '0x[0-9a-f]+' | head -1)"
    tj="$(echo "$l2" | grep -oE '0x[0-9a-f]+' | head -1)"
    [ -n "$t1" ] && [ -n "$tj" ] || continue
    da1=$((16#$a1)); daj=$((16#$aj)); dt1=$((t1)); dtj=$((tj))
    [ "$dt1" -eq $((da1 + 8)) ] || continue
    dd=$(( dtj > daj ? dtj - daj : daj - dtj ))
    if [ "$dd" -gt 4094 ]; then relax_found=1; break; fi
done
[ "$relax_found" -eq 1 ] || fail "no relaxed out-of-range conditional branch (inverted guard + jal) found"

echo "verifying relocations in $obj"
rel="$("$readobj" -r "$obj")"
for r in R_RISCV_PCREL_HI20 R_RISCV_PCREL_LO12_I R_RISCV_PCREL_LO12_S R_RISCV_CALL_PLT; do
    grep -q "$r" <<< "$rel" || fail "expected relocation '$r' not emitted"
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
