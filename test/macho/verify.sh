#!/usr/bin/env bash
# cross-compile the freestanding Mach-O fixture for both Darwin triples and
# byte-verify the emitted relocatable object and the linked executable with the
# llvm tools. proves the Mach-O object format emits a correct object (header, load
# commands, sections, nlist symbols, relocations) and a structurally valid static
# executable (__PAGEZERO, segments, LC_UNIXTHREAD entry) without running them:
# Darwin binaries cannot run on this Linux host, and a Darwin exit needs the
# libSystem runtime, which is the #1178 follow-up. this mirrors the riscv64 cross
# lane - byte-verification, no execution step.
#
# usage: verify.sh [path-to-mach]   (defaults to `mach` on PATH)
#
# requires: file, llvm-objdump, llvm-otool (unversioned or `-NN` suffixed).
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

objdump="$(resolve_tool llvm-objdump)" || fail "llvm-objdump not found"
otool="$(resolve_tool llvm-otool)"     || fail "llvm-otool not found"

# verify_target <target> <machine> <thread-flavor> <reloc>...
verify_target() {
    local target="$1" machine="$2" flavor="$3"; shift 3
    local exe="out/$target/machoprobe"

    echo "cross-compiling fixture for $target with $mach"
    "$mach" build . --target "$target" --profile debug
    local obj
    obj="$(find "out/$target/obj" -name '*.o' -print -quit)"
    [ -n "$obj" ] || fail "$target: no object emitted"

    echo "verifying $target object $obj"
    file "$obj" | grep -q "Mach-O 64-bit $machine object" || fail "$target: object is not a Mach-O $machine object"
    "$objdump" --macho -t "$obj" | grep -qw '_main'       || fail "$target: object missing the _main entry symbol"

    local dis
    dis="$("$objdump" --macho -d "$obj")"
    echo "$dis" | grep -qi 'unknown' && fail "$target: object disassembly has an unknown instruction"

    local rel
    rel="$("$objdump" --macho -r "$obj")"
    local r
    for r in "$@"; do
        echo "$rel" | grep -q "$r" || fail "$target: expected relocation '$r' not emitted"
    done

    echo "verifying $target executable $exe"
    file "$exe" | grep -q "Mach-O 64-bit $machine executable" || fail "$target: not a Mach-O $machine executable"
    local lc
    lc="$("$otool" -l "$exe")"
    echo "$lc" | grep -q '__PAGEZERO'    || fail "$target: executable missing __PAGEZERO"
    echo "$lc" | grep -q '__TEXT'        || fail "$target: executable missing __TEXT segment"
    echo "$lc" | grep -q 'LC_UNIXTHREAD' || fail "$target: executable missing LC_UNIXTHREAD"
    echo "$lc" | grep -q "$flavor"       || fail "$target: executable thread state is not $flavor"
}

rm -rf out
verify_target darwin-x86_64  x86_64 x86_THREAD_STATE64 SIGNED
verify_target darwin-aarch64 arm64  ARM_THREAD_STATE64 PAGE21 PAGOF12 BR26

echo "OK: Mach-O object format emits correct objects and executables for both Darwin triples"
