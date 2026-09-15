#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_debuginfo_coff <engine> <leg> <nog_binary> <g_binary>
# host-side over the windows images run.sh built with and without -g: the DWARF verifier's
# verdict, a symbolizer resolving `square` from its DWARF address, and byte-additivity of
# every section a loader maps.
produce_debuginfo_coff() {
    nog=$3
    g=$4
    dd_tool=$(resolve_dwarfdump) || {
        echo "link: debuginfo-coff: llvm-dwarfdump not found (install the 'llvm' package)" >&2; return 2
    }
    sym_tool=$(resolve_symbolizer) || {
        echo "link: debuginfo-coff: llvm-symbolizer not found (install the 'llvm' package)" >&2; return 2
    }
    command -v llvm-objcopy >/dev/null 2>&1 || {
        echo "link: debuginfo-coff: llvm-objcopy not found (install the 'llvm' package)" >&2; return 2
    }

    # the one expected warning class is the DWARF 5 file-slot duplicate the debuginfo case
    # documents; any other warning or error names itself in the diff
    dd_expected='\.debug_line\[.*\]\.prologue\.file_names\[1\] is a duplicate of file_names\[0\]'
    dd_out=$("$dd_tool" --verify "$g" 2>&1)
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

    # the address comes from the DIE, so a string or range offset gone wrong loses the
    # name before the symbolizer is ever asked
    addr=$("$dd_tool" --debug-info "$g" | awk '
        index($0, "DW_AT_name") && index($0, "(\"square\")") {
            getline
            if (match($0, /0x[0-9a-fA-F]+/)) { print substr($0, RSTART, RLENGTH); exit }
        }')
    if [ -z "$addr" ]; then
        echo "square_die=missing"
    else
        echo "square_die=present"
        resolved=$("$sym_tool" --obj="$g" "$addr" 2>&1)
        fn=$(printf '%s\n' "$resolved" | sed -n '1p')
        loc=$(printf '%s\n' "$resolved" | sed -n '2p' | sed -e 's#.*/##')
        echo "square_symbol=$fn"
        echo "square_line=$loc"
    fi

    # .buildid hashes the whole file, so it legitimately moves when debug sections join it
    additive=yes
    tmp=$(mktemp -d)
    for sec in $(llvm-objdump -h "$nog" | awk '$1 ~ /^[0-9]+$/ { print $2 }'); do
        [ "$sec" = .buildid ] && continue
        llvm-objcopy --dump-section "$sec=$tmp/n" "$nog" /dev/null 2>/dev/null || continue
        llvm-objcopy --dump-section "$sec=$tmp/g" "$g" /dev/null 2>/dev/null || { additive=no; break; }
        cmp -s "$tmp/n" "$tmp/g" || { additive=no; break; }
    done
    rm -rf "$tmp"
    echo "g_additive=$additive"
}

produce_debuginfo_coff "$@"
