#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_macho_abs_bind <engine> <leg> <binary>
#
# Walk the dyld bind rows for the fixture's two absolute 64-bit references to
# libSystem's __stdoutp/__stderrp. Each must bind in place inside __DATA (the
# cell itself is bound - no __GOT slot), one row per site, and the on-disk cell
# content must be the relocation's zero addend. On the native Intel macOS leg
# the same PIE is executed and its dereference contract compared exactly.
produce_macho_abs_bind() {
    target=$2
    bin=$3

    flags=$(read_le_uint "$bin" 24 4)
    [ $((flags & 0x200000)) -ne 0 ] || {
        echo "link: macho-abs-bind: executable is not PIE" >&2
        return 1
    }

    data_fields=$(macho_segment_fields "$bin" __DATA) || return 2
    set -- $data_fields
    data_vm=$1; data_file=$3; data_size=$4

    binds=$(macho_objdump --macho --bind "$bin" | tr 'A-F' 'a-f') || return 2

    check_slot() {
        sym=$1
        rows=$(printf '%s\n' "$binds" | grep -F "$sym" | grep -v "__got" || true)
        [ "$(printf '%s\n' "$rows" | grep -c .)" -eq 1 ] || {
            echo "link: macho-abs-bind: $sym does not have exactly one in-place bind row" >&2
            return 1
        }
        addr=$(printf '%s\n' "$rows" | awk '{for (i = 1; i <= NF; i++) if ($i ~ /^0x[0-9a-f]+$/) { print $i; exit }}')
        [ -n "$addr" ] || {
            echo "link: macho-abs-bind: $sym bind row carries no address" >&2
            return 1
        }
        [ $((addr)) -ge "$data_vm" ] && [ $((addr)) -lt $((data_vm + data_size)) ] || {
            echo "link: macho-abs-bind: $sym bind address is outside __DATA" >&2
            return 1
        }
        cell=$(read_le_uint "$bin" $((data_file + addr - data_vm)) 8)
        [ "$cell" -eq 0 ] || {
            echo "link: macho-abs-bind: $sym cell is not the zero addend on disk" >&2
            return 1
        }
        return 0
    }

    check_slot ___stdoutp || return 1
    check_slot ___stderrp || return 1

    if [ "$target" = x86_64-darwin ]; then
        out=$(mktemp)
        run_captured native "$target" "$bin" "$out" || { rm -f "$out"; return 1; }
        rm -f "$out"
        if [ "$run_status" -ne 0 ]; then
            report_run_failure "macho-abs-bind: the native PIE" "$run_status" "$run_out"
            return 1
        fi
        [ "$run_out" = "abs-bind=1" ] || {
            echo "link: macho-abs-bind: the native PIE ran to completion but computed wrong values" >&2
            diff_expected_actual "abs-bind=1" "$run_out"
            return 1
        }
    fi

    echo "PIE=1"
    echo "out_slot=libSystem-bind-in-place"
    echo "err_slot=libSystem-bind-in-place"
    echo "slots=addend-zero"
}

produce_macho_abs_bind "$@"
