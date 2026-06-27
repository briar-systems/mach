#!/usr/bin/env bash
# cross-compile the freestanding Mach-O fixtures for both Darwin triples and
# byte-verify the emitted objects and executables with the llvm tools. proves the
# Mach-O object format emits a correct object (header, load commands, sections,
# nlist symbols, relocations), a structurally valid STATIC executable (__PAGEZERO,
# segments, LC_UNIXTHREAD entry), and a dyld-loadable DYNAMIC executable
# (LC_LOAD_DYLINKER, LC_LOAD_DYLIB, an LC_DYLD_INFO_ONLY bind stream, the
# LC_SYMTAB/LC_DYSYMTAB pair, and an import stub the call site is redirected to).
# nothing is run: Darwin binaries cannot run on this Linux host, and an arm64
# Darwin binary additionally needs a code signature the kernel verifies, which
# this writer does not emit. this mirrors the riscv64 cross lane - byte
# verification, no execution step.
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

# verify_static <target> <machine> <thread-flavor> <reloc>...
#
# cross-compiles the static fixture (no dependencies, so the linker takes the
# emit_exec path) and checks its relocatable object and its LC_UNIXTHREAD exe.
verify_static() {
    local target="$1" machine="$2" flavor="$3"; shift 3
    local exe="out/$target/machoprobe"

    echo "cross-compiling static fixture for $target with $mach"
    "$mach" build . --target "$target" --profile debug
    local obj
    obj="$(find "out/$target/obj" -name '*.o' -print -quit)"
    [ -n "$obj" ] || fail "$target: no object emitted"

    echo "verifying $target object $obj"
    file "$obj" | grep -q "Mach-O 64-bit $machine object" || fail "$target: object is not a Mach-O $machine object"
    "$objdump" --macho -t "$obj" | grep -qw 'start'       || fail "$target: object missing the 'start' entry symbol"

    local dis
    dis="$("$objdump" --macho -d "$obj")"
    if echo "$dis" | grep -qi 'unknown'; then fail "$target: object disassembly has an unknown instruction"; fi

    local rel
    rel="$("$objdump" --macho -r "$obj")"
    local r
    for r in "$@"; do
        echo "$rel" | grep -q "$r" || fail "$target: expected relocation '$r' not emitted"
    done

    echo "verifying $target static executable $exe"
    file "$exe" | grep -q "Mach-O 64-bit $machine executable" || fail "$target: not a Mach-O $machine executable"
    local lc
    lc="$("$otool" -l "$exe")"
    echo "$lc" | grep -q '__PAGEZERO'    || fail "$target: executable missing __PAGEZERO"
    echo "$lc" | grep -q '__TEXT'        || fail "$target: executable missing __TEXT segment"
    echo "$lc" | grep -q 'LC_UNIXTHREAD' || fail "$target: executable missing LC_UNIXTHREAD"
    echo "$lc" | grep -q "$flavor"       || fail "$target: executable thread state is not $flavor"
}

# verify_dynamic <target> <machine>
#
# cross-compiles the dynamic fixture (a libprobe.dylib dependency and a
# `probe_add` import, so the linker takes the emit_dyn_exec path) and checks every
# dynamic-linking structure: the loader and dependency load commands, the dyld
# bind stream naming the import, the symbol tables, and the import stub.
verify_dynamic() {
    local target="$1" machine="$2"
    local exe="dynamic/out/$target/machodyn"

    echo "cross-compiling dynamic fixture for $target with $mach"
    "$mach" build dynamic --target "$target" --profile debug

    echo "verifying $target dynamic executable $exe"
    file "$exe" | grep -q "Mach-O 64-bit $machine executable" || fail "$target: not a Mach-O $machine executable"

    local lc
    lc="$("$otool" -l "$exe")"
    echo "$lc" | grep -q '__PAGEZERO'         || fail "$target: dyn exe missing __PAGEZERO"
    echo "$lc" | grep -q '__LINKEDIT'         || fail "$target: dyn exe missing __LINKEDIT"
    echo "$lc" | grep -q 'LC_DYLD_INFO_ONLY'  || fail "$target: dyn exe missing LC_DYLD_INFO_ONLY"
    echo "$lc" | grep -q 'LC_SYMTAB'          || fail "$target: dyn exe missing LC_SYMTAB"
    echo "$lc" | grep -q 'LC_DYSYMTAB'        || fail "$target: dyn exe missing LC_DYSYMTAB"
    echo "$lc" | grep -q 'LC_LOAD_DYLINKER'   || fail "$target: dyn exe missing LC_LOAD_DYLINKER"
    echo "$lc" | grep -q '/usr/lib/dyld'      || fail "$target: dyn exe dylinker is not /usr/lib/dyld"
    echo "$lc" | grep -q 'LC_LOAD_DYLIB'      || fail "$target: dyn exe missing LC_LOAD_DYLIB"
    echo "$lc" | grep -q 'LC_UNIXTHREAD'      || fail "$target: dyn exe missing LC_UNIXTHREAD"

    "$otool" -L "$exe" | grep -q 'libprobe.dylib' || fail "$target: dyn exe does not link libprobe.dylib"

    # the import is an undefined two-level symbol bound by a dyld pointer record.
    "$objdump" --macho -t "$exe"   | grep -q 'probe_add' || fail "$target: dyn exe missing the probe_add import symbol"
    "$objdump" --macho --bind "$exe" | grep -q 'probe_add' || fail "$target: dyn exe has no bind record for probe_add"

    # the deferred call site is redirected into the __stubs section.
    "$otool" -l "$exe" | grep -q '__stubs' || fail "$target: dyn exe missing the __stubs section"
    local dis
    dis="$("$objdump" --macho -d --section=__stubs "$exe")"
    if echo "$dis" | grep -qi 'unknown'; then fail "$target: __stubs disassembly has an unknown instruction"; fi
}

rm -rf out dynamic/out
verify_static  darwin-x86_64  x86_64 x86_THREAD_STATE64 SIGNED
verify_static  darwin-aarch64 arm64  ARM_THREAD_STATE64 PAGE21 PAGOF12 BR26
verify_dynamic darwin-x86_64  x86_64
verify_dynamic darwin-aarch64 arm64

echo "OK: Mach-O emits correct objects and static + dynamic executables for both Darwin triples"
