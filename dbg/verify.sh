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

# var_loclist_ranges <binary> <name> — the number of PC ranges in the named local's
# DW_AT_location location list, or 0 when it has an inline (single-location) exprloc or
# no location. A variable that re-homes across its scope (the #1705 tier) reports its
# range count; the single-location tier reports 0.
var_loclist_ranges() {
    "$dwarfdump" --debug-info "$1" 2>/dev/null | awk -v want="$2" '
        /DW_TAG_/    { invar = ($0 ~ /DW_TAG_variable/); name=""; loc=0 }
        /DW_AT_name/ && invar { if ($0 ~ "\\(\"" want "\"\\)") name=want }
        name==want && /DW_AT_location/ { loc=1; next }
        name==want && loc && /\[0x/    { cnt++ }
        name==want && loc && !/\[0x/   { loc=0 }
        END { print cnt+0 }'
}

# var_computed <binary> <name> — "1" if the named local's DW_AT_location is a computed
# value (DW_OP_const* ... DW_OP_stack_value), else "0". A constant-folded local (the
# #1706 tier) renders this way instead of dropping its location.
var_computed() {
    "$dwarfdump" --debug-info "$1" 2>/dev/null | awk -v want="$2" '
        /DW_TAG_/    { invar = ($0 ~ /DW_TAG_variable/); name="" }
        /DW_AT_name/ && invar { if ($0 ~ "\\(\"" want "\"\\)") name=want }
        name==want && /DW_AT_location/ && /DW_OP_const/ && /DW_OP_stack_value/ { print "1"; exit }
        END { print "0" }' | head -1
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
    #    section-table bookkeeping). a codegen change under -g fails here. NOTE: this
    #    runs at the debug profile only; the release counterpart is deferred to #1944
    #    (release -g still perturbs .text on clean dev), so verify_inline_release does
    #    not yet assert it.
    if ! elf_seg_identical "$g" "$nog"; then
        echo "FAIL $label (-g perturbed a loadable segment vs the no-g build)"; rm -rf "$tmp"; return 1
    fi

    # 6. PC-ranged variable locations (#1705): `pick`'s re-homed local `x` gets a
    #    `.debug_loclists` entry with multiple ranges. under the single-location tier
    #    alone it would have no location at all, so this is the falsifiable loclists
    #    check. only asserted where the fixture has such a variable.
    if grep -q '^fun pick' "$src"; then
        if ! readelf -SW "$g" 2>/dev/null | grep -q '\.debug_loclists'; then
            echo "FAIL $label (no .debug_loclists section for a re-homed local)"; rm -rf "$tmp"; return 1
        fi
        nr=$(var_loclist_ranges "$g" x)
        if [ "${nr:-0}" -lt 2 ]; then
            echo "FAIL $label ('x' has $nr loclist ranges; expected a multi-range location list)"; rm -rf "$tmp"; return 1
        fi
    fi

    # 7. computed constant values (#1706): `konst`'s folded local `kc` renders as a
    #    DW_OP_const* ... DW_OP_stack_value computed value. under the reg/frame tiers
    #    alone it would have no location, so this is the falsifiable computed-value check.
    if grep -q '^fun konst' "$src"; then
        if [ "$(var_computed "$g" kc)" != "1" ]; then
            echo "FAIL $label ('kc' has no DW_OP_const/stack_value computed location)"; rm -rf "$tmp"; return 1
        fi
    fi

    # 8. composite type DIEs (#1919): a fixture with a record local emits a
    #    DW_TAG_structure_type with DW_TAG_member children at their byte offsets, a
    #    DW_TAG_array_type + DW_TAG_subrange_type for its array local, and a
    #    DW_TAG_pointer_type resolving the record for its typed pointer. under the
    #    primitive-only tier all of these are absent, so this is the falsifiable composite
    #    check.
    if grep -q '^rec ' "$src"; then
        di=$("$dwarfdump" --debug-info "$g" 2>/dev/null)
        for tag in DW_TAG_structure_type DW_TAG_member DW_TAG_array_type DW_TAG_subrange_type DW_TAG_pointer_type; do
            if ! printf '%s\n' "$di" | grep -q "$tag"; then
                echo "FAIL $label (no $tag DIE for aggregate/composite locals)"; rm -rf "$tmp"; return 1
            fi
        done
        if ! printf '%s\n' "$di" | grep -q 'DW_AT_data_member_location'; then
            echo "FAIL $label (struct member has no DW_AT_data_member_location)"; rm -rf "$tmp"; return 1
        fi

        # 8a. the array's element count and the pointer's pointee are VALUE-checked, not
        #     just tag-checked: llvm-dwarfdump synthesizes a type's name from its DIE, so
        #     a wrong DW_AT_count renders "i32[3]" not "i32[4]", and a pointer whose ref4
        #     resolves to the wrong pointee renders e.g. "i32 *" not "Point *". both would
        #     pass --verify (any in-CU ref4 / any count is structurally valid), so these
        #     catch value drift the tag greps miss.
        if ! printf '%s\n' "$di" | grep -q 'DW_AT_type.*"i32\[4\]"'; then
            echo "FAIL $label (array local's DW_AT_count is not 4: no i32[4] type)"; rm -rf "$tmp"; return 1
        fi
        if ! printf '%s\n' "$di" | grep -q 'DW_AT_type.*"Point \*"'; then
            echo "FAIL $label (typed pointer does not resolve to the Point record)"; rm -rf "$tmp"; return 1
        fi

        # 8b. by-reference aggregate locations never use a bare register address. An
        #     aggregate's dbg-value is its storage POINTER; describing it whole-scope as
        #     DW_OP_breg<reg>+0 (address in a reused register) is a wild pointer read that
        #     --verify accepts. The producer instead emits DW_OP_fbreg+DW_OP_deref for a
        #     frame home and OMITS a register home, so a DW_OP_breg location that is NOT a
        #     computed value (no DW_OP_stack_value) is the regression, on any variable.
        if printf '%s\n' "$di" | grep 'DW_OP_breg' | grep -qv 'DW_OP_stack_value'; then
            echo "FAIL $label (a DW_OP_breg location is a bare register address, not a computed value — the aggregate wild-read regression)"; rm -rf "$tmp"; return 1
        fi

        # NOTE (member VALUES): these checks are structural — DIE shape, DW_AT_count and
        # pointee names, and location-form. They do NOT read a member's runtime VALUE
        # (`pt.count == 100`); that needs a debugger executing the target, and the ELF lane
        # is a host-side DWARF inspector (llvm-dwarfdump / addr2line), tool-free of a
        # runtime. The member-value read is the STAGED lldb source-breakpoint check (see the
        # staged block below); until a runtime runner lands, a frame-homed aggregate's
        # member values are proven only by the PR's manual gdb transcript, not by CI.
    fi

    echo "PASS $label"
    rm -rf "$tmp"
    return 0
}

# verify_inline_release <fixture_dir> <build_target> <min_dies> <callees> — the inline
# pass fires only at release, so this leg builds one -g release binary and asserts the
# DWARF inlined_subroutine tier: llvm-dwarfdump --verify is clean, at least <min_dies>
# DW_TAG_inlined_subroutine DIEs are present, and each name in <callees> (space-
# separated) appears as an inlined instance (its DW_AT_abstract_origin resolves to it) —
# the pin that every level of a nested inline chain survived. Byte-for-byte -g additivity
# is NOT asserted here yet: release -g still perturbs .text on clean dev (a pre-existing
# -g-sensitive inlining decision, tracked in #1944 — see step 5's note). turn the
# elf_seg_identical assertion on at release once #1944 lands.
verify_inline_release() {
    dir=$1; bt=$2; min=$3; callees=$4
    label="${dir#"$here"/} [$bt release]"
    tmp=$(mktemp -d)
    g="$tmp/g"
    if ! (cd "$dir" && "$compiler" dep pull && "$compiler" build . --target "$bt" --profile release -g -o "$g") >"$tmp/b.log" 2>&1; then
        echo "FAIL $label (build release -g)"; sed 's/^/    /' "$tmp/b.log" >&2; rm -rf "$tmp"; return 1
    fi
    if ! "$dwarfdump" --verify "$g" >"$tmp/v.log" 2>&1; then
        echo "FAIL $label (llvm-dwarfdump --verify)"; tail -30 "$tmp/v.log" | sed 's/^/    /' >&2; rm -rf "$tmp"; return 1
    fi
    n=$("$dwarfdump" --debug-info "$g" 2>/dev/null | grep -c DW_TAG_inlined_subroutine)
    if [ "$n" -lt "$min" ]; then
        echo "FAIL $label (expected >=$min inlined_subroutine DIEs, found $n)"; rm -rf "$tmp"; return 1
    fi
    for callee in $callees; do
        if ! "$dwarfdump" --debug-info "$g" 2>/dev/null | grep -q "DW_AT_abstract_origin.*\"$callee\""; then
            echo "FAIL $label (no inlined_subroutine referencing $callee)"; rm -rf "$tmp"; return 1
        fi
    done
    echo "PASS $label ($n inlined_subroutine DIEs)"
    rm -rf "$tmp"
    return 0
}

# verify_no_foreign_file <fixture_dir> <build_target> — regression for #1902 (loc
# provenance leak). A std dependency object's .debug_line must never list the entry
# module (main.mach) as a source file: a synthesized instruction left with a zeroed
# {0,0} location used to read as file 0 (the first-registered = entry module) and emit
# a stray row misattributing a std address to main.mach:1. With FILE_NIL == 0 a zeroed
# loc is loc_nil and drops. Scans every std object under the fixture's obj tree.
verify_no_foreign_file() {
    dir=$1; bt=$2
    label="${dir#"$here"/} [$bt no-foreign-file]"
    tmp=$(mktemp -d)
    if ! (cd "$dir" && "$compiler" dep pull && "$compiler" build . --target "$bt" --profile debug -g -o "$tmp/g") >"$tmp/b.log" 2>&1; then
        echo "FAIL $label (build)"; sed 's/^/    /' "$tmp/b.log" >&2; rm -rf "$tmp"; return 1
    fi
    objdir="$dir/out/$bt/debug/obj"
    bad=0
    for o in $(find "$objdir" -path '*std*' -name '*.o' 2>/dev/null); do
        if "$dwarfdump" --debug-line "$o" 2>/dev/null | grep -q 'main\.mach'; then
            echo "FAIL $label (${o#"$objdir"/} attributes a .debug_line row to the entry module main.mach)" >&2
            bad=$((bad + 1))
        fi
    done
    rm -rf "$tmp"
    if [ "$bad" -ne 0 ]; then return 1; fi
    echo "PASS $label"
    return 0
}

# verify_aggregate_release <fixture_dir> <build_target> — the release counterpart of the
# composite checks. At -O2 an aggregate's storage pointer re-homes across its scope, so
# its location becomes a `.debug_loclists` list (the debug build keeps a single stable
# home and never exercises that path). This leg asserts --verify is clean and, crucially,
# that no aggregate location is a bare DW_OP_breg register address (the wild-read
# regression) even when homes move — the byref plumb-through in the per-range loclist
# emit is otherwise unexercised by CI.
verify_aggregate_release() {
    dir=$1; bt=$2
    label="${dir#"$here"/} [$bt release]"
    tmp=$(mktemp -d)
    g="$tmp/g"
    if ! (cd "$dir" && "$compiler" dep pull && "$compiler" build . --target "$bt" --profile release -g -o "$g") >"$tmp/b.log" 2>&1; then
        echo "FAIL $label (build release -g)"; sed 's/^/    /' "$tmp/b.log" >&2; rm -rf "$tmp"; return 1
    fi
    if ! "$dwarfdump" --verify "$g" >"$tmp/v.log" 2>&1; then
        echo "FAIL $label (llvm-dwarfdump --verify)"; tail -30 "$tmp/v.log" | sed 's/^/    /' >&2; rm -rf "$tmp"; return 1
    fi
    di=$("$dwarfdump" --debug-info "$g" 2>/dev/null)
    if ! printf '%s\n' "$di" | grep -q 'DW_TAG_structure_type'; then
        echo "FAIL $label (no composite type DIE at release)"; rm -rf "$tmp"; return 1
    fi
    if printf '%s\n' "$di" | grep 'DW_OP_breg' | grep -qv 'DW_OP_stack_value'; then
        echo "FAIL $label (a moving aggregate rendered a bare DW_OP_breg register address — the wild-read regression in the loclist path)"; rm -rf "$tmp"; return 1
    fi
    echo "PASS $label"
    rm -rf "$tmp"
    return 0
}

for dir in "$fixtures"/$filter; do
    [ -f "$dir/mach.toml" ] || continue
    # the inline fixtures are release-only (their callees inline away at -O0); they have
    # their own release legs below and do not fit the debug fixture's structural checks.
    case "$(basename "$dir")" in inline|inline4) continue;; esac
    for bt in $elf_targets; do
        ran=$((ran + 1))
        verify_dwarf "$dir" "$bt" || fails=$((fails + 1))
    done
done

# release legs: exercise the release-only inline pass so the inlined_subroutine tier
# (#1707) is actually verified in CI (debug builds never inline). inline is a 2-level
# chain (leaf under mid); inline4 is a 4-level chain (a>b>c>d) that pins nested-instance
# rollup recovery — every level must survive as its own inlined instance.
if [ -f "$fixtures/inline/mach.toml" ]; then
    for bt in $elf_targets; do
        ran=$((ran + 1))
        verify_inline_release "$fixtures/inline" "$bt" 2 "leaf mid" || fails=$((fails + 1))
    done
fi
if [ -f "$fixtures/inline4/mach.toml" ]; then
    for bt in $elf_targets; do
        ran=$((ran + 1))
        verify_inline_release "$fixtures/inline4" "$bt" 4 "a b c d" || fails=$((fails + 1))
    done
fi

# #1919 release leg: an aggregate re-homes at -O2, so its location becomes a loclist; this
# exercises the byref plumb-through in the per-range loclist emit that the debug build
# (single stable home) never reaches, and pins the no-bare-breg wild-read guard there.
if [ -f "$fixtures/aggregate/mach.toml" ]; then
    for bt in $elf_targets; do
        ran=$((ran + 1))
        verify_aggregate_release "$fixtures/aggregate" "$bt" || fails=$((fails + 1))
    done
fi

# #1902 regression: no std dependency object may attribute a .debug_line row to the
# entry module. The multi fixture is a [bin] main.mach pulling std, so its std objects
# are the exact surface the loc-provenance leak corrupted.
if [ -f "$fixtures/multi/mach.toml" ]; then
    for bt in $elf_targets; do
        ran=$((ran + 1))
        verify_no_foreign_file "$fixtures/multi" "$bt" || fails=$((fails + 1))
    done
fi

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
