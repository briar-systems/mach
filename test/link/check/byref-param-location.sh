#!/usr/bin/env bash
. "$(dirname "$0")/common.sh"

# produce_byref_param_location <engine> <leg> <nog_binary> <g_binary>
# the BY-REFERENCE PARAMETER LOCATION observable (#3461): the aggregate parameter `w`
# of `probe` has a DW_AT_location, and every expression in it is a memory location
# reached through the incoming pointer.
#
# aapcs64, lp64d and win64 pass a 32-byte record as the address of a caller-owned
# copy, and sysv64 leaves it in the caller's outgoing area that the callee addresses
# through a register too, so the parameter's home is a pointer wherever it is. the
# producer used to reject a register home for an aggregate outright, which on those
# conventions left the parameter with no location at all. what a debugger needs is a
# location of MEMORY class based on the pointer: `DW_OP_breg<n> <k>` while the pointer
# is in a register, `DW_OP_fbreg <slot>, DW_OP_deref` once it is spilled. a register
# class (`DW_OP_reg<n>`, which would make the debugger read the pointer's bytes as the
# record) or a value class (`DW_OP_stack_value`) is wrong for it and counts as such.
#
# the register number and the slot offset are the allocator's and differ per ISA and
# profile, so only the class is stated and one golden serves every arm:
#   located=<yes|no>   the parameter DIE carries a DW_AT_location
#   exprs=<nonzero|zero>  expressions found (a loclist may carry several)
#   nonmemory=<n>      expressions not of memory class; the invariant is 0
produce_byref_param_location() {
    g=$4
    dd_tool=$(resolve_dwarfdump) || {
        echo "link: byref-param-location: llvm-dwarfdump not found (install the 'llvm' package)" >&2; return 2
    }

    # the expressions of `w` under the `probe` subprogram, one per line
    exprs=$("$dd_tool" --debug-info "$g" 2>/dev/null | awk '
        /DW_TAG_subprogram/ { in_probe = 0; in_w = 0; next }
        /DW_AT_name/ && /"probe"/ { in_probe = 1; next }
        in_probe && /DW_TAG_/ { in_w = 0 }
        in_probe && /DW_TAG_formal_parameter/ { in_param = 1; next }
        in_probe && in_param && /DW_AT_name/ && /"w"/ { in_w = 1; in_param = 0; next }
        in_probe && in_param && /DW_AT_name/ { in_param = 0; next }
        in_w && /DW_AT_location/ {
            located = 1
            s = $0; sub(/^.*DW_AT_location[ \t]*\(/, "", s)
            if (s ~ /^0x[0-9a-f]+:/) { in_list = 1; next }
            sub(/\)$/, "", s); print s; next
        }
        in_w && in_list && /^[ \t]*\[0x[0-9a-f]+, 0x[0-9a-f]+\): / {
            s = $0; sub(/^[ \t]*\[0x[0-9a-f]+, 0x[0-9a-f]+\): /, "", s)
            if (s ~ /\)$/) { in_list = 0; sub(/\)$/, "", s) }
            print s; next
        }
        in_w && in_list && /^[ \t]*$/ { in_list = 0 }
        END { if (!located) print "NONE" }
    ')

    if [ "$exprs" = "NONE" ]; then
        echo "byref_param_located=no"
        echo "byref_param_exprs=zero"
        echo "byref_param_nonmemory=0"
        return 0
    fi
    count=0; nonmemory=0
    while IFS= read -r e; do
        [ -n "$e" ] || continue
        count=$((count + 1))
        case "$e" in
            "DW_OP_breg"[0-9]*" "*[+-][0-9]*) ;;
            "DW_OP_bregx "*) ;;
            "DW_OP_fbreg "[+-]*[0-9]", DW_OP_deref") ;;
            "DW_OP_fbreg "[+-]*[0-9]", DW_OP_deref, DW_OP_plus_uconst "*) ;;
            "DW_OP_fbreg "[+-]*[0-9]", DW_OP_deref, DW_OP_constu "*) ;;
            "DW_OP_fbreg "[+-]*[0-9]) ;;
            *) nonmemory=$((nonmemory + 1)); echo "link: byref-param-location: non-memory expression: $e" >&2 ;;
        esac
    done <<<"$exprs"
    echo "byref_param_located=yes"
    echo "byref_param_exprs=$([ "$count" -gt 0 ] && echo nonzero || echo zero)"
    echo "byref_param_nonmemory=$nonmemory"
}

produce_byref_param_location "$@"
