#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# _norm_shdr_fields <in> <out> — copy <in> to <out> zeroing the ELF header's
# section-table bookkeeping (e_shoff @40 8B, e_shnum @60 2B, e_shstrndx @62 2B), which
# legitimately differs once `-g` adds named debug sections. everything else — every
# loadable byte — must stay identical.
_norm_shdr_fields() {
    cp "$1" "$2"
    printf '\0\0\0\0\0\0\0\0' | dd of="$2" bs=1 seek=40 count=8 conv=notrunc status=none
    printf '\0\0'             | dd of="$2" bs=1 seek=60 count=2 conv=notrunc status=none
    printf '\0\0'             | dd of="$2" bs=1 seek=62 count=2 conv=notrunc status=none
}

# elf_seg_identical <g> <nog> — 0 when every PT_LOAD segment of the `-g` image has
# byte-identical file content in the no-`-g` image (after normalizing the header
# section-table fields), else 1. the additive-only guard: `-g` must not perturb one
# byte of the loaded program. PT_LOAD file extents come from `readelf -lW` (offset,
# filesz); a p_filesz of 0 (a pure .bss LOAD) carries no file bytes to compare.
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
    done < <(readelf -lW "$1" 2>/dev/null | awk '/LOAD/{print strtonum($2), strtonum($5)}')
    rm -f "$an" "$bn"
    return $rc
}

# produce_debuginfo <engine> <leg> <nog_binary> <g_binary>
# the binary-inspection producer for the debuginfo case kind (#2039): asserts, purely
# host-side over the artifacts run.sh built with and without `-g`, that (1) the
# standard structural validator accepts the whole `-g` image and which warning classes
# it reports while doing so, (1b) a real consumer
# (addr2line, i.e. libbfd) decodes the line table without a diagnostic and resolves the
# entry point to a name, (2) `-g` is loadable-byte additive, and (3) duplicate generic,
# comptime-value, and pack instances retain
# one live, symbolizable DIE while each discarded copy carries DWARF's dead-code address,
# has no line-table sequence at the winner, and has no location list at the winner.
# the facts are ISA-independent, so the golden is shared. requires llvm-dwarfdump,
# llvm-symbolizer, readelf, and addr2line on the leg; a missing tool is a hard error.
produce_debuginfo() {
    nog=$3
    g=$4
    dd_tool=$(resolve_dwarfdump) || {
        echo "link: debuginfo: llvm-dwarfdump not found (install the 'llvm' package)" >&2; return 2
    }
    sym_tool=$(resolve_symbolizer) || {
        echo "link: debuginfo: llvm-symbolizer not found (install the 'llvm' package)" >&2; return 2
    }
    command -v readelf >/dev/null 2>&1 || {
        echo "link: debuginfo: readelf not found (install 'binutils')" >&2; return 2
    }
    command -v addr2line >/dev/null 2>&1 || {
        echo "link: debuginfo: addr2line not found (install 'binutils')" >&2; return 2
    }

    # --verify EXITS ZERO ON WARNINGS (#2755), so reading only its status collapsed "no
    # diagnostics at all" onto "no errors, and a wall of warnings". That is how a
    # validator stops validating: the next real warning lands in a stream nobody reads.
    # The observable is therefore the stream. `errors` is reported first because an
    # error subsumes a warning, and each residual warning TEXT is listed - sorted,
    # deduplicated, with the per-CU `[0x...]` offset elided - so a failure names itself
    # in the diff instead of reading `warnings`.
    #
    # ONE warning class is expected and filtered, with its reason: DWARF 5 §6.2.4
    # numbers file entries from 0 while §6.2.2 still gives the line state machine's
    # `file` register an initial value of 1, and binutils resolves that in favour of
    # §6.2.2. So a single-file CU must declare its source at slot 0 AND at slot 1 or
    # libbfd rejects the whole section ("mangled line number section (bad file number)",
    # #2582) - measured on binutils 2.47 against clang's own single-entry `-gdwarf-5`
    # output as well as ours. gcc emits the duplicate and llvm-dwarfdump warns on gcc's
    # output identically. It is also validator-version dependent: llvm-dwarfdump 18 does
    # not report it and 22 does, so leaving it in the stream would make the golden a
    # statement about the runner's llvm package. Every OTHER warning, of any class,
    # still fails the case.
    dd_expected='\.debug_line\[.*\]\.prologue\.file_names\[1\] is a duplicate of file_names\[0\]'
    dd_out=$("$dd_tool" --verify "$g" 2>&1)
    # `|| true` because an empty result is the expected outcome of a filter rather than
    # a failure, and run.sh runs under `set -e`.
    dd_warn=$(printf '%s\n' "$dd_out" | sed -n 's/^warning: //p' \
        | { grep -v -E "$dd_expected" || true; } \
        | sed -e 's/\[0x[0-9a-fA-F]*\]/[]/g' | sort -u)
    if printf '%s\n' "$dd_out" | grep -q '^error:'; then
        echo "dwarfdump_verify=errors"
    elif [ -n "$dd_warn" ]; then
        echo "dwarfdump_verify=warnings"
    else
        echo "dwarfdump_verify=clean"
    fi
    if [ -n "$dd_warn" ]; then
        printf '%s\n' "$dd_warn" | while IFS= read -r w; do echo "dwarfdump_warning=$w"; done
    fi

    # CONSUMER-SIDE DECODE (#2582). --verify above checks structural and reference
    # integrity; it does NOT check that the line program decodes against the file table
    # the way a consumer reads it, so it accepted a .debug_line that binutils rejected
    # outright ("mangled line number section (bad file number)") on every CU. addr2line
    # is the cheapest standard consumer of that decode - the same libbfd path perf, gdb,
    # and most crash symbolizers reach - so its stderr is the observable, verbatim when
    # non-empty so a regression names itself in the diff rather than reading `errors`.
    # the entry point is the address because every leg's image has one at a known place.
    entry=$(readelf -hW "$g" 2>/dev/null | awk '/Entry point address:/{print $NF}')
    a2l_err=$(addr2line -f -e "$g" "$entry" 2>&1 >/dev/null | sed -n '1p')
    a2l_fn=$(addr2line -f -e "$g" "$entry" 2>/dev/null | sed -n '1p')
    if [ -n "$a2l_err" ]; then
        echo "addr2line_stderr=$a2l_err"
    else
        echo "addr2line_stderr=clean"
    fi
    if [ -n "$a2l_fn" ] && [ "$a2l_fn" != "??" ]; then
        echo "addr2line_entry=resolved"
    else
        echo "addr2line_entry=unresolved"
    fi

    if elf_seg_identical "$g" "$nog"; then
        echo "g_additive=yes"
    else
        echo "g_additive=no"
    fi

    # helper and main instantiate all three weak template forms. each winner must
    # symbolize by source name while the losing atom's DIE retains a dead low_pc.
    info=$("$dd_tool" --debug-info "$g") || return 1
    lines=$("$dd_tool" --debug-line "$g") || return 1
    locations=$("$dd_tool" --debug-loclists "$g") || return 1
    for spec in ident:ident value:add_n pack:pack_sum; do
        label=${spec%%:*}
        want=${spec#*:}
        counts=$(printf '%s\n' "$info" | awk -v want="$want" '
            index($0, "DW_AT_name") && index($0, "(\"" want "\")") {
                getline
                if ($0 ~ /dead code/) { dead++ }
                else if ($0 ~ /DW_AT_low_pc.*0x/) {
                    live++
                    if (addr == "" && match($0, /0x[0-9a-fA-F]+/)) {
                        addr = substr($0, RSTART, RLENGTH)
                    }
                }
            }
            END { printf "%d %d %s", live, dead, addr }
        ')
        set -- $counts
        printf 'weak_%s_dies=live:%s,dead:%s\n' "$label" "$1" "$2"
        # a substring match, not equality: once a `.symtab` exists (#2772) a real
        # symbolizer prefers the ELF symbol table's linkage name over DWARF's
        # DW_AT_name for the function-name field, so the resolved text is the
        # mangled form (e.g. `_M7dbgcase7genericN11identI3i64E`), not the bare
        # source identifier - and the mangling scheme itself is due to change
        # (the dotted-name rewrite). either way the source identifier is still
        # IN there, so that is the fact this asserts, printed back as the
        # semantic label rather than the raw resolved text so the golden names
        # what was checked instead of freezing today's mangling spelling.
        symbol=missing
        if [ -n "$3" ]; then
            resolved=$("$sym_tool" --obj="$g" "$3" | sed -n '1p')
            case "$resolved" in *"$want"*) symbol=$want ;; esac
        fi
        printf 'weak_%s_symbol=%s\n' "$label" "$symbol"

        # every live template has exactly one sequence beginning at its entry. a
        # losing weak set_address that resolves to the winner creates a second
        # prologue_end row at that address while remaining validator-clean.
        line_starts=$(printf '%s\n' "$lines" | awk -v addr="$3" '
            $1 == addr && /prologue_end/ { n++ }
            END { print n + 0 }
        ')
        line_state="count:$line_starts"
        [ "$line_starts" -eq 1 ] && line_state=unique
        printf 'weak_%s_lines=%s\n' "$label" "$line_state"

        # pack_sum's changing accumulator home gives it a location list in debug
        # builds. release may optimize that list away, so the invariant is that at
        # most one list starts at the winner; a losing base_address alias makes two.
        if [ "$label" = pack ]; then
            loc_starts=$(printf '%s\n' "$locations" | awk -v addr="$3" '
                index($0, "[" addr ",") { n++ }
                END { print n + 0 }
            ')
            loc_state="aliased:$loc_starts"
            [ "$loc_starts" -le 1 ] && loc_state=not-aliased
            printf 'weak_pack_locations=%s\n' "$loc_state"
        fi
    done
}

produce_debuginfo "$@"
