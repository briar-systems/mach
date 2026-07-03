#!/usr/bin/env bash
# verify.sh — the debug-info verification harness.
#
# usage: verify.sh [--filter <glob>] <compiler>
#
# for each debug-info fixture under dbg/fixtures/, builds it with and without `-g`
# using the given compiler and machine-checks the emitted debug info with external
# validators (llvm-dwarfdump, addr2line). the exit status is nonzero if any check
# on any fixture×target fails.
#
# WHY A SEPARATE HARNESS. the int/ golden-diff harness deliberately depends on no
# external tooling (it reads binary fields with coreutils); debug-info correctness
# can only be asserted against the format's own validators (llvm-dwarfdump --verify,
# addr2line, and — once their producers land — llvm-pdbutil / lldb). keeping those
# test-only deps here leaves int/ tool-free.
#
# WHAT IS LIVE TODAY. the DWARF-on-ELF producer emits real .debug_abbrev/.debug_info/
# .debug_line under `-g`, so the ELF lane below is live: it runs on every shipped ELF
# ISA (llvm-dwarfdump/addr2line read the target object regardless of host ISA, so one
# linux runner covers x86_64 + aarch64 + riscv64). the Mach-O runtime checks (lldb /
# codesign / dyld) and the COFF/CodeView checks (llvm-pdbutil) are STAGED — see the
# staged block at the end of this file and dbg/README.md. this is the lane every
# debug-info tier issue references in its acceptance.
set -u

here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
fixtures="$here/fixtures"

usage() {
    echo "usage: verify.sh [--filter <glob>] <compiler>" >&2
    exit 2
}

filter='*'
compiler=
while [ $# -gt 0 ]; do
    case "$1" in
        --filter) shift; [ $# -gt 0 ] || usage; filter=$1 ;;
        -h|--help) usage ;;
        -*) echo "verify.sh: unknown flag '$1'" >&2; usage ;;
        *)  [ -z "$compiler" ] || { echo "verify.sh: unexpected argument '$1'" >&2; usage; }
            compiler=$1 ;;
    esac
    shift
done
[ -n "$compiler" ] || { echo "verify.sh: a compiler path is required" >&2; usage; }

# resolve the compiler to an absolute path; fixtures build from their own dirs.
case "$compiler" in
    /*) : ;;
    *)  compiler=$(CDPATH= cd -- "$(dirname -- "$compiler")" && pwd)/$(basename -- "$compiler") ;;
esac

# the ELF/DWARF targets validated host-side. every shipped ELF ISA emits DWARF and
# the validators read the target object regardless of the host's ISA, so this one
# linux runner covers them all. macho/coff are staged (see the end of the file).
elf_targets="linux linux-arm64 linux-riscv64"

# resolve_dwarfdump — print an llvm-dwarfdump on PATH, preferring the unversioned
# name and falling back to the highest-versioned one (ubuntu ships llvm-dwarfdump-NN).
resolve_dwarfdump() {
    if command -v llvm-dwarfdump >/dev/null 2>&1; then echo llvm-dwarfdump; return 0; fi
    newest=$(compgen -c 'llvm-dwarfdump-' 2>/dev/null | sort -t- -k3 -n | tail -1)
    [ -n "$newest" ] && { echo "$newest"; return 0; }
    return 1
}

dwarfdump=$(resolve_dwarfdump) || {
    echo "verify.sh: llvm-dwarfdump not found (install the 'llvm' package)" >&2
    exit 2
}
command -v addr2line >/dev/null 2>&1 || {
    echo "verify.sh: addr2line not found (install 'binutils')" >&2
    exit 2
}

fails=0
ran=0

# cu_bounds <binary> — print "<low> <high>" (decimal) of the fixture compile unit
# (the one whose DW_AT_name is the fixture's ./src/main.mach), or nothing if absent.
cu_bounds() {
    "$dwarfdump" --debug-info "$1" 2>/dev/null | awk '
        /DW_TAG_compile_unit/{lo="";hi="";nm=0}
        $0 ~ /"\.\/src\/main\.mach"/{nm=1}
        /DW_AT_low_pc/{gsub(/[()]/,"",$2);lo=$2}
        /DW_AT_high_pc/{gsub(/[()]/,"",$2);hi=$2; if(nm){print strtonum(lo), strtonum(hi); exit}}'
}

# text_range <binary> — print "<addr> <addr+size>" (decimal) of the .text section.
text_range() {
    readelf -SW "$1" 2>/dev/null | awk '
        /\.text/ && /PROGBITS/ {
            for (i=1;i<=NF;i++) if ($i=="PROGBITS") { a=strtonum("0x"$(i+1)); s=strtonum("0x"$(i+3)); print a, a+s; exit }
        }'
}

# row_addr <binary> <low> <high> <line> — print the first line-table row address
# (0x-hex) inside [low,high) whose source line equals <line>, or nothing.
row_addr() {
    "$dwarfdump" --debug-line "$1" 2>/dev/null | awk -v lo="$2" -v hi="$3" -v ln="$4" '
        /^0x/{a=strtonum($1); if(a>=lo && a<hi && $2==ln){printf "0x%x\n", a; exit}}'
}

# distinct_lines <binary> <low> <high> — count distinct source lines the CU's rows
# reference (a multi-function fixture must reference several).
distinct_lines() {
    "$dwarfdump" --debug-line "$1" 2>/dev/null | awk -v lo="$2" -v hi="$3" '
        /^0x/{a=strtonum($1); if(a>=lo && a<hi) seen[$2]=1} END{print length(seen)}'
}

# elf_seg_identical <a> <b> — true if every PT_LOAD segment of <a> has byte-identical
# file content in <b>, after the ELF header's section-table bookkeeping (e_shoff,
# e_shnum, e_shstrndx — expected to differ once -g adds named sections) is normalized.
# this is the additive-only guard: -g must not perturb one byte of the loaded program.
elf_seg_identical() {
    an=$(mktemp); bn=$(mktemp)
    _norm_shdr_fields "$1" "$an"; _norm_shdr_fields "$2" "$bn"
    rc=0
    while read -r off fsz; do
        [ "$fsz" -eq 0 ] && continue
        if ! cmp -s \
            <(dd if="$an" bs=1M iflag=skip_bytes,count_bytes skip="$off" count="$fsz" status=none) \
            <(dd if="$bn" bs=1M iflag=skip_bytes,count_bytes skip="$off" count="$fsz" status=none); then
            rc=1; break
        fi
    done < <(readelf -lW "$1" 2>/dev/null | awk '/LOAD/{print strtonum($2), strtonum($6)}')
    rm -f "$an" "$bn"
    return $rc
}

# _norm_shdr_fields <in> <out> — copy <in> to <out> zeroing e_shoff/e_shnum/e_shstrndx.
_norm_shdr_fields() {
    cp "$1" "$2"
    printf '\0\0\0\0\0\0\0\0' | dd of="$2" bs=1 seek=40 count=8 conv=notrunc status=none
    printf '\0\0'             | dd of="$2" bs=1 seek=60 count=2 conv=notrunc status=none
    printf '\0\0'             | dd of="$2" bs=1 seek=62 count=2 conv=notrunc status=none
}

# verify_dwarf <fixture_dir> <build_target> — the live ELF/DWARF checks for one
# fixture built for one target. returns 0 on pass, 1 on any failed assertion.
verify_dwarf() {
    dir=$1; bt=$2
    label="${dir#"$here"/} [$bt]"
    tmp=$(mktemp -d)
    g="$tmp/g"; nog="$tmp/nog"
    src="$dir/src/main.mach"

    if ! (cd "$dir" && "$compiler" dep pull && "$compiler" build . --target "$bt" --profile debug -g -o "$g") >"$tmp/build.log" 2>&1; then
        echo "FAIL $label (build -g)"; sed 's/^/    /' "$tmp/build.log" >&2; rm -rf "$tmp"; return 1
    fi
    if ! (cd "$dir" && "$compiler" build . --target "$bt" --profile debug -o "$nog") >"$tmp/build2.log" 2>&1; then
        echo "FAIL $label (build no-g)"; sed 's/^/    /' "$tmp/build2.log" >&2; rm -rf "$tmp"; return 1
    fi

    # 1. the standard structural verifier accepts the whole image.
    if ! "$dwarfdump" --verify "$g" >"$tmp/verify.log" 2>&1; then
        echo "FAIL $label (llvm-dwarfdump --verify)"; tail -20 "$tmp/verify.log" | sed 's/^/    /' >&2; rm -rf "$tmp"; return 1
    fi

    # 2. the fixture compile unit exists and its PC range lies inside .text.
    read -r low high < <(cu_bounds "$g")
    if [ -z "${low:-}" ] || [ -z "${high:-}" ]; then
        echo "FAIL $label (no compile unit for ./src/main.mach)"; rm -rf "$tmp"; return 1
    fi
    read -r tlo thi < <(text_range "$g")
    if [ -z "${tlo:-}" ] || [ "$low" -lt "$tlo" ] || [ "$high" -gt "$thi" ] || [ "$low" -ge "$high" ]; then
        echo "FAIL $label (CU [$low,$high) not within .text [$tlo,$thi))"; rm -rf "$tmp"; return 1
    fi

    # 3. every helper function is represented in the line table at its own `fun` line,
    #    and a standard consumer (addr2line) round-trips one such PC back to file:line.
    rc=0
    for fn in add mul square; do
        fl=$(grep -n "^fun $fn" "$src" | head -1 | cut -d: -f1)
        if [ -z "$fl" ]; then echo "FAIL $label (fixture has no 'fun $fn')" >&2; rc=1; continue; fi
        a=$(row_addr "$g" "$low" "$high" "$fl")
        if [ -z "$a" ]; then echo "FAIL $label (no line row for 'fun $fn' at line $fl)" >&2; rc=1; continue; fi
        if [ "$fn" = add ]; then
            out=$(addr2line -e "$g" "$a" 2>/dev/null)
            file=${out%:*}; ln=${out##*:}; ln=${ln%% *}
            if [ "${file##*/}" != "main.mach" ] || [ "$ln" != "$fl" ]; then
                echo "FAIL $label (addr2line $a -> '$out', want main.mach:$fl)" >&2; rc=1
            fi
        fi
    done
    if [ "$rc" -ne 0 ]; then rm -rf "$tmp"; return 1; fi

    # 4. multi-function coverage: the CU references several distinct source lines.
    n=$(distinct_lines "$g" "$low" "$high")
    if [ "${n:-0}" -lt 3 ]; then
        echo "FAIL $label (only $n distinct source lines in the CU; expected a multi-function table)"; rm -rf "$tmp"; return 1
    fi

    # 5. additive-only: -g must not perturb the loaded program. every PT_LOAD segment
    #    is byte-identical between the -g and no-g builds (modulo the ELF header's
    #    section-table bookkeeping). a codegen change under -g fails here.
    if ! elf_seg_identical "$g" "$nog"; then
        echo "FAIL $label (-g perturbed a loadable segment vs the no-g build)"; rm -rf "$tmp"; return 1
    fi

    echo "PASS $label"
    rm -rf "$tmp"
    return 0
}

for dir in "$fixtures"/$filter; do
    [ -f "$dir/mach.toml" ] || continue
    for bt in $elf_targets; do
        ran=$((ran + 1))
        verify_dwarf "$dir" "$bt" || fails=$((fails + 1))
    done
done

# STAGED — expansion points wired but not yet live, each pending its producer. these
# consume the same fixtures; a tier issue turns one on by adding its checks here and
# its runner leg (see dbg/README.md and issue #1698's runtime-check comment).
#
#   Mach-O DWARF runtime (darwin-x86_64 / darwin-aarch64) — needs a macOS runner
#     (int-main.yml, on merge to main): `codesign --verify` covers the __DWARF bytes,
#     the image loads under dyld, and lldb hits a source-line breakpoint. the
#     structural llvm-dwarfdump --verify already works host-side and can be lifted
#     into the elf loop's shape once a macho fixture leg is added.
#   COFF / CodeView (windows) — needs the CodeView producer (#1595); `mach build -g`
#     emits no debug sections for COFF today, so the lane is dormant. once it lands:
#     llvm-pdbutil / llvm-readobj --sections assert the .debug$S/.debug$T discardable
#     sections and the COFF long-name string table, on the existing windows runner.

if [ "$ran" -eq 0 ]; then
    echo "verify.sh: no fixtures matched --filter '$filter'" >&2
    exit 1
fi
if [ "$fails" -ne 0 ]; then
    echo "dbg: $fails failure(s)"
    exit 1
fi
echo "dbg: all debug-info checks passed ($ran target build(s))"
