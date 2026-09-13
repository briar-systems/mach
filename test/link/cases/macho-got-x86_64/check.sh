#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_macho_got <engine> <leg> <binary>
#
# Decode the checked-in fixture's five instruction signatures from __TEXT. The
# two clang GOT_LOAD fields and the assembly GOT32/GOT64 fields independently
# resolve to one local and one imported slot. Then inspect the exact dyld rows:
# the initialized local slot must be rebased, while the zero-on-disk import slot
# must bind ___stderrp to libSystem. On the native Intel macOS leg the same PIE is
# also executed and its values compared byte-for-byte with the runtime contract.
#
# The two non-relaxable local signatures (GOT32 / GOT64) carry a `movq (%rax), %rax`
# the import ones do not: X86_64_RELOC_GOT resolves to the slot address, so reading
# the local int takes two loads where testing the imported pointer takes one (#2586).
produce_macho_got() {
    target=$2
    bin=$3

    flags=$(read_le_uint "$bin" 24 4)
    [ $((flags & 0x200000)) -ne 0 ] || {
        echo "link: macho-got: executable is not PIE" >&2
        return 1
    }

    text_fields=$(macho_segment_fields "$bin" __TEXT) || return 2
    set -- $text_fields
    text_vm=$1; text_file=$3; text_size=$4

    sites=$(od -An -v -tu1 -j "$text_file" -N "$text_size" "$bin" | awk -v vm="$text_vm" '
        function s32(i, u) {
            u = b[i] + 256*b[i+1] + 65536*b[i+2] + 16777216*b[i+3]
            return u >= 2147483648 ? u - 4294967296 : u
        }
        function s64(i, lo, hi, u) {
            lo = b[i] + 256*b[i+1] + 65536*b[i+2] + 16777216*b[i+3]
            hi = b[i+4] + 256*b[i+5] + 65536*b[i+6] + 16777216*b[i+7]
            u = lo + 4294967296*hi
            return hi >= 2147483648 ? u - 18446744073709551616 : u
        }
        { for (i = 1; i <= NF; i++) b[++n] = $i }
        END {
            nl = ni = gl = gi = g64 = 0
            for (i = 1; i <= n; i++) {
                pc = vm + i - 1
                if (i + 10 <= n && b[i] == 72 && b[i+1] == 139 && b[i+2] == 5 &&
                    b[i+7] == 139 && b[i+8] == 0 && b[i+9] == 93 && b[i+10] == 195) {
                    nl++; vl = pc + 7 + s32(i + 3)
                }
                if (i + 16 <= n && b[i] == 72 && b[i+1] == 139 && b[i+2] == 13 &&
                    b[i+7] == 49 && b[i+8] == 192 && b[i+9] == 72 && b[i+10] == 131 &&
                    b[i+11] == 57 && b[i+12] == 0 && b[i+13] == 15 && b[i+14] == 149 &&
                    b[i+15] == 192) {
                    ni++; vi = pc + 7 + s32(i + 3)
                }
                if (i + 12 <= n && b[i] == 72 && b[i+1] == 141 && b[i+2] == 5 &&
                    b[i+7] == 72 && b[i+8] == 139 && b[i+9] == 0 &&
                    b[i+10] == 139 && b[i+11] == 0 && b[i+12] == 195) {
                    gl++; vgl = pc + 7 + s32(i + 3)
                }
                if (i + 17 <= n && b[i] == 72 && b[i+1] == 141 && b[i+2] == 5 &&
                    b[i+7] == 72 && b[i+8] == 131 && b[i+9] == 56 && b[i+10] == 0 &&
                    b[i+11] == 15 && b[i+12] == 149 && b[i+13] == 192 && b[i+14] == 15 &&
                    b[i+15] == 182 && b[i+16] == 192 && b[i+17] == 195) {
                    gi++; vgi = pc + 7 + s32(i + 3)
                }
                if (i + 25 <= n && b[i] == 72 && b[i+1] == 184 && b[i+10] == 72 &&
                    b[i+11] == 141 && b[i+12] == 13 && b[i+13] == 249 && b[i+14] == 255 &&
                    b[i+15] == 255 && b[i+16] == 255 && b[i+17] == 72 && b[i+18] == 1 &&
                    b[i+19] == 200 && b[i+20] == 72 && b[i+21] == 139 && b[i+22] == 0 &&
                    b[i+23] == 139 && b[i+24] == 0 && b[i+25] == 195) {
                    g64++; vg64 = pc + 10 + s64(i + 2)
                }
            }
            if (nl != 1 || ni != 1 || gl != 1 || gi != 1 || g64 != 1) {
                print "link: macho-got: expected GOT sites 1/1/1/1/1, found " nl "/" ni "/" gl "/" gi "/" g64 > "/dev/stderr"
                exit 2
            }
            printf "%.0f %.0f %.0f %.0f %.0f\n", vl, vi, vgl, vgi, vg64
        }
    ') || return 2
    set -- $sites
    local_load=$1; import_load=$2; local_got=$3; import_got=$4; local_got64=$5

    [ "$local_load" -eq "$local_got" ] && [ "$local_load" -eq "$local_got64" ] || {
        echo "link: macho-got: local references did not deduplicate to one slot" >&2
        return 1
    }
    [ "$import_load" -eq "$import_got" ] || {
        echo "link: macho-got: import references did not deduplicate to one slot" >&2
        return 1
    }
    [ "$local_load" -ne "$import_load" ] || {
        echo "link: macho-got: local and imported symbols share a slot" >&2
        return 1
    }

    relro_fields=$(macho_segment_fields "$bin" __DATA_CONST) || return 2
    set -- $relro_fields
    relro_vm=$1; relro_file=$3; relro_size=$4
    [ "$local_load" -ge "$relro_vm" ] && [ "$local_load" -lt $((relro_vm + relro_size)) ] || {
        echo "link: macho-got: local slot is not in __DATA_CONST" >&2
        return 1
    }
    local_file=$((relro_file + local_load - relro_vm))
    local_value=$(read_le_uint "$bin" "$local_file" 8)
    [ "$local_value" -ne 0 ] || {
        echo "link: macho-got: local slot was not initialized" >&2
        return 1
    }

    got_fields=$(macho_segment_fields "$bin" __GOT) || return 2
    set -- $got_fields
    got_vm=$1; got_file=$3; got_size=$4
    [ "$import_load" -ge "$got_vm" ] && [ "$import_load" -lt $((got_vm + got_size)) ] || {
        echo "link: macho-got: import slot is not in __GOT" >&2
        return 1
    }
    import_file=$((got_file + import_load - got_vm))
    import_value=$(read_le_uint "$bin" "$import_file" 8)
    [ "$import_value" -eq 0 ] || {
        echo "link: macho-got: import slot is not zero before dyld binding" >&2
        return 1
    }

    local_hex=$(printf '0x%x' "$local_load")
    import_hex=$(printf '0x%x' "$import_load")
    rebases=$(macho_objdump --macho --rebase "$bin" | tr 'A-F' 'a-f') || return 2
    [ "$(printf '%s\n' "$rebases" | grep -F -c "$local_hex")" -eq 1 ] || {
        echo "link: macho-got: local slot has no unique rebase row" >&2
        return 1
    }
    binds=$(macho_objdump --macho --bind "$bin" | tr 'A-F' 'a-f') || return 2
    bind_row=$(printf '%s\n' "$binds" | grep -F "$import_hex" || true)
    [ "$(printf '%s\n' "$bind_row" | grep -c 'libSystem.*___stderrp')" -eq 1 ] || {
        echo "link: macho-got: import slot has no unique libSystem ___stderrp bind" >&2
        return 1
    }

    if [ "$target" = x86_64-darwin ]; then
        out=$(mktemp)
        run_captured native "$target" "$bin" "$out" || { rm -f "$out"; return 1; }
        rm -f "$out"
        expected=$(printf '%s\n' \
            'local-load=40' 'local-got=40' 'local-got64=40' \
            'import-load=1' 'import-got=1')
        if [ "$run_status" -ne 0 ]; then
            report_run_failure "macho-got: the native PIE" "$run_status" "$run_out"
            return 1
        fi
        [ "$run_out" = "$expected" ] || {
            echo "link: macho-got: the native PIE ran to completion but computed wrong values" >&2
            diff_expected_actual "$expected" "$run_out"
            return 1
        }
    fi

    echo "PIE=1"
    echo "local_slots=deduplicated"
    echo "local_slot=rebase"
    echo "import_slots=deduplicated"
    echo "import_slot=libSystem-bind"
}

produce_macho_got "$@"
