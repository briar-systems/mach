#!/usr/bin/env bash
# cross-compile the freestanding Mach-O fixtures for both Darwin triples and
# byte-verify the emitted objects and executables with the llvm tools. proves the
# Mach-O object format emits a correct object (header, load commands, sections,
# nlist symbols, relocations), a structurally valid STATIC executable (__PAGEZERO,
# segments, LC_UNIXTHREAD entry), and a dyld-loadable DYNAMIC executable
# (LC_LOAD_DYLINKER, LC_LOAD_DYLIB, an LC_DYLD_INFO_ONLY bind stream, the
# LC_SYMTAB/LC_DYSYMTAB pair, and an import stub the call site is redirected to).
# both executables also carry an ad-hoc LC_CODE_SIGNATURE, whose embedded
# SuperBlob + CodeDirectory this script parses and validates by hand (codesign is
# macOS-only). nothing is run: Darwin binaries cannot run on this Linux host. this
# mirrors the riscv64 cross lane - byte verification, no execution step.
#
# usage: verify.sh [path-to-mach]   (defaults to `mach` on PATH)
#
# requires: file, od, llvm-objdump, llvm-otool (unversioned or `-NN` suffixed).
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

# read a big-endian u32 at byte offset $2 of file $1, as a decimal value. the code
# signing structures are big-endian, unlike the little-endian Mach-O around them.
be32() {
    local hex; hex="$(od -An -tx1 -j "$2" -N 4 "$1" | tr -d ' \n')"
    printf '%d' "0x$hex"
}

# read a u8 at byte offset $2 of file $1, as a decimal value.
u8() {
    local hex; hex="$(od -An -tx1 -j "$2" -N 1 "$1" | tr -d ' \n')"
    printf '%d' "0x$hex"
}

# verify_codesig <target> <exe>
#
# asserts the executable carries an LC_CODE_SIGNATURE and that the bytes it points
# at are a well-formed embedded ad-hoc signature: a CSMAGIC_EMBEDDED_SIGNATURE
# SuperBlob whose sole sub-blob is a CSMAGIC_CODEDIRECTORY with the adhoc flag,
# SHA-256 hashes, a 4 KiB page size, codeLimit equal to the signature offset, and
# one code slot per page of [0, codeLimit). all fields are big-endian.
verify_codesig() {
    local target="$1" exe="$2"
    local lc; lc="$("$otool" -l "$exe")"
    echo "$lc" | grep -q 'LC_CODE_SIGNATURE' || fail "$target: executable missing LC_CODE_SIGNATURE"

    local dataoff datasize
    dataoff="$(echo "$lc"  | awk '/cmd LC_CODE_SIGNATURE/{f=1} f&&$1=="dataoff" {print $2; exit}')"
    datasize="$(echo "$lc" | awk '/cmd LC_CODE_SIGNATURE/{f=1} f&&$1=="datasize"{print $2; exit}')"
    [ -n "$dataoff" ] && [ -n "$datasize" ] || fail "$target: LC_CODE_SIGNATURE has no dataoff/datasize"

    # the signature must lie inside the __LINKEDIT segment's file range.
    local le_off le_size
    le_off="$(echo "$lc"  | awk '/segname __LINKEDIT/{f=1} f&&$1=="fileoff" {print $2; exit}')"
    le_size="$(echo "$lc" | awk '/segname __LINKEDIT/{f=1} f&&$1=="filesize"{print $2; exit}')"
    [ -n "$le_off" ] && [ -n "$le_size" ] || fail "$target: no __LINKEDIT segment for the signature"
    [ "$dataoff" -ge "$le_off" ] && [ $((dataoff + datasize)) -le $((le_off + le_size)) ] \
        || fail "$target: signature escapes __LINKEDIT"

    # SuperBlob: magic, total length == datasize, and a CSSLOT_CODEDIRECTORY index.
    [ "$(be32 "$exe" "$dataoff")" = "$((0xFADE0CC0))" ] || fail "$target: bad CSMAGIC_EMBEDDED_SIGNATURE"
    [ "$(be32 "$exe" $((dataoff + 4)))" = "$datasize" ] || fail "$target: SuperBlob length != datasize"
    [ "$(be32 "$exe" $((dataoff + 8)))" -ge 1 ]         || fail "$target: SuperBlob has no sub-blobs"
    [ "$(be32 "$exe" $((dataoff + 12)))" = 0 ]          || fail "$target: first slot is not CSSLOT_CODEDIRECTORY"
    local cd=$((dataoff + $(be32 "$exe" $((dataoff + 16)))))

    # CodeDirectory: magic, adhoc flag, SHA-256, 4 KiB pages, codeLimit, slot count.
    [ "$(be32 "$exe" "$cd")" = "$((0xFADE0C02))" ]      || fail "$target: bad CSMAGIC_CODEDIRECTORY"
    [ $(( $(be32 "$exe" $((cd + 12))) & 0x2 )) -ne 0 ]  || fail "$target: CodeDirectory adhoc flag not set"
    [ "$(be32 "$exe" $((cd + 24)))" = 0 ]               || fail "$target: CodeDirectory has special slots"
    [ "$(u8   "$exe" $((cd + 36)))" = 32 ]              || fail "$target: hashSize is not 32 (SHA-256)"
    [ "$(u8   "$exe" $((cd + 37)))" = 2 ]               || fail "$target: hashType is not SHA-256"
    [ "$(u8   "$exe" $((cd + 39)))" = 12 ]              || fail "$target: pageSize is not 12 (4 KiB)"
    local code_limit n_code expect
    code_limit="$(be32 "$exe" $((cd + 32)))"
    [ "$code_limit" = "$dataoff" ]                      || fail "$target: codeLimit != signature offset"
    n_code="$(be32 "$exe" $((cd + 28)))"
    expect=$(( (code_limit + 4095) / 4096 ))
    [ "$n_code" = "$expect" ]                           || fail "$target: nCodeSlots != ceil(codeLimit/4096)"

    echo "verified $target ad-hoc code signature (codeLimit=$code_limit, nCodeSlots=$n_code)"
}

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
    echo "$lc" | grep -q '__LINKEDIT'    || fail "$target: executable missing __LINKEDIT segment"
    echo "$lc" | grep -q 'LC_UNIXTHREAD' || fail "$target: executable missing LC_UNIXTHREAD"
    echo "$lc" | grep -q "$flavor"       || fail "$target: executable thread state is not $flavor"

    verify_codesig "$target" "$exe"
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

    verify_codesig "$target" "$exe"
}

# verify_filename_independent <dir> <target>
#
# builds the fixture in <dir> twice under two different `-o` output names and
# asserts the two signed images are byte-identical. the code-signing identifier is
# the artifact's product name, not the `-o` basename, so the output filename must
# not perturb a single byte - this is exactly what the darwin self-host fixpoint
# (a -> b -> c, b == c) depends on, and it is the only filename-sensitive part of
# the writer, so it is the part worth locking down.
verify_filename_independent() {
    local dir="$1" target="$2"
    echo "checking $target signed-image filename-independence in $dir"
    mkdir -p "$PWD/$dir/out"
    local a="$PWD/$dir/out/fnind_a" b="$PWD/$dir/out/fnind_b"
    "$mach" build "$dir" --target "$target" --profile debug -o "$a"
    "$mach" build "$dir" --target "$target" --profile debug -o "$b"
    cmp -s "$a" "$b" \
        || fail "$target: signed image differs under a different -o name (identifier leaks the filename)"
    echo "verified $target signed image is identical regardless of the -o name"
}

# verify_pie <target> <machine> <reloc>...
#
# cross-compiles the static fixture for a target whose main executables must be
# position-independent (arm64 Darwin: the Apple Silicon kernel refuses a non-PIE
# image), so the linker routes it through the dynamic/PIE writer even with no shared
# dependency. checks the relocatable object, then the PIE executable shape: MH_PIE,
# an LC_MAIN entry, LC_LOAD_DYLINKER, LC_BUILD_VERSION, LC_UUID, and no LC_UNIXTHREAD.
verify_pie() {
    local target="$1" machine="$2"; shift 2
    local exe="out/$target/machoprobe"

    echo "cross-compiling PIE fixture for $target with $mach"
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

    echo "verifying $target PIE executable $exe"
    file "$exe" | grep -q "Mach-O 64-bit $machine executable" || fail "$target: not a Mach-O $machine executable"
    file "$exe" | grep -q 'PIE'                               || fail "$target: executable is not MH_PIE"
    local lc
    lc="$("$otool" -l "$exe")"
    echo "$lc" | grep -q '__PAGEZERO'       || fail "$target: PIE exe missing __PAGEZERO"
    echo "$lc" | grep -q '__TEXT'           || fail "$target: PIE exe missing __TEXT segment"
    echo "$lc" | grep -q '__LINKEDIT'       || fail "$target: PIE exe missing __LINKEDIT segment"
    echo "$lc" | grep -q 'LC_MAIN'          || fail "$target: PIE exe missing LC_MAIN entry"
    echo "$lc" | grep -q 'LC_LOAD_DYLINKER' || fail "$target: PIE exe missing LC_LOAD_DYLINKER"
    echo "$lc" | grep -q '/usr/lib/dyld'    || fail "$target: PIE exe dylinker is not /usr/lib/dyld"
    echo "$lc" | grep -q 'LC_BUILD_VERSION' || fail "$target: PIE exe missing LC_BUILD_VERSION"
    echo "$lc" | grep -q 'LC_UUID'          || fail "$target: PIE exe missing LC_UUID"
    echo "$lc" | grep -q 'LC_DYLD_INFO'     || fail "$target: PIE exe missing LC_DYLD_INFO"
    if echo "$lc" | grep -q 'LC_UNIXTHREAD'; then fail "$target: PIE exe must not carry LC_UNIXTHREAD"; fi

    verify_codesig "$target" "$exe"
}

# verify_pie_dynamic <target> <machine>
#
# cross-compiles the dynamic fixture for a PIE target: it carries a libprobe.dylib
# dependency and a `probe_add` import, so the PIE writer must combine the rebase/bind
# dyld-info streams - MH_PIE + LC_MAIN with a real LC_LOAD_DYLIB, bind record, and
# import stub.
verify_pie_dynamic() {
    local target="$1" machine="$2"
    local exe="dynamic/out/$target/machodyn"

    echo "cross-compiling dynamic PIE fixture for $target with $mach"
    "$mach" build dynamic --target "$target" --profile debug

    echo "verifying $target dynamic PIE executable $exe"
    file "$exe" | grep -q "Mach-O 64-bit $machine executable" || fail "$target: not a Mach-O $machine executable"
    file "$exe" | grep -q 'PIE'                               || fail "$target: dyn PIE exe is not MH_PIE"

    local lc
    lc="$("$otool" -l "$exe")"
    echo "$lc" | grep -q 'LC_MAIN'           || fail "$target: dyn PIE exe missing LC_MAIN"
    echo "$lc" | grep -q 'LC_BUILD_VERSION'  || fail "$target: dyn PIE exe missing LC_BUILD_VERSION"
    echo "$lc" | grep -q 'LC_DYLD_INFO'      || fail "$target: dyn PIE exe missing LC_DYLD_INFO"
    echo "$lc" | grep -q 'LC_LOAD_DYLINKER'  || fail "$target: dyn PIE exe missing LC_LOAD_DYLINKER"
    echo "$lc" | grep -q 'LC_LOAD_DYLIB'     || fail "$target: dyn PIE exe missing LC_LOAD_DYLIB"
    if echo "$lc" | grep -q 'LC_UNIXTHREAD'; then fail "$target: dyn PIE exe must not carry LC_UNIXTHREAD"; fi

    "$otool" -L "$exe" | grep -q 'libprobe.dylib' || fail "$target: dyn PIE exe does not link libprobe.dylib"
    "$objdump" --macho -t "$exe"     | grep -q 'probe_add' || fail "$target: dyn PIE exe missing the probe_add import symbol"
    "$objdump" --macho --bind "$exe" | grep -q 'probe_add' || fail "$target: dyn PIE exe has no bind record for probe_add"
    "$otool" -l "$exe" | grep -q '__stubs' || fail "$target: dyn PIE exe missing the __stubs section"

    verify_codesig "$target" "$exe"
}

# verify_rebase <target>
#
# cross-compiles the rebase fixture (globals initialized to the address of other
# globals, so __DATA holds absolute pointers) for a PIE target and asserts the
# emitted LC_DYLD_INFO rebase stream names the __DATA pointer slots and resolves to
# the right addresses (llvm-objdump --rebase decodes the opcode stream).
verify_rebase() {
    local target="$1"
    local exe="rebase/out/$target/machorebase"

    echo "cross-compiling rebase fixture for $target with $mach"
    "$mach" build rebase --target "$target" --profile debug

    echo "verifying $target rebase stream in $exe"
    file "$exe" | grep -q 'PIE' || fail "$target: rebase exe is not MH_PIE"
    local rb
    rb="$("$objdump" --macho --rebase "$exe")"
    echo "$rb" | grep -q '__DATA' || fail "$target: rebase table has no __DATA entries"
    local n
    n="$(echo "$rb" | grep -c 'pointer')"
    [ "$n" -ge 2 ] || fail "$target: expected at least 2 rebased pointers, got $n"

    verify_codesig "$target" "$exe"
}

rm -rf out dynamic/out rebase/out
verify_static      darwin-x86_64  x86_64 x86_THREAD_STATE64 SIGNED
verify_pie         darwin-aarch64 arm64  PAGE21 PAGOF12 BR26
verify_dynamic     darwin-x86_64  x86_64
verify_pie_dynamic darwin-aarch64 arm64
verify_rebase      darwin-aarch64
verify_filename_independent .       darwin-aarch64
verify_filename_independent dynamic darwin-aarch64
verify_filename_independent rebase  darwin-aarch64

echo "OK: Mach-O emits correct objects, x86_64 static/dynamic execs, and arm64 PIE execs (LC_MAIN + rebase) for both Darwin triples"
