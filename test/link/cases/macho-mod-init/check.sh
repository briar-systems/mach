#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_macho_mod_init <engine> <leg> <binary>
# Assert that a __DATA,__mod_init_func section reaches the image as one dyld will
# actually run, then on a darwin runner run it and require that dyld did.
#
# Presence and even a correct pointer are not enough. dyld finds an image's
# initializers by scanning for sections whose TYPE is S_MOD_INIT_FUNC_POINTERS, so
# a section carrying the right name, the right alignment, and a correctly rebased
# pointer at the right address is still never called if the link emitted the
# default S_REGULAR type - which is exactly what #2637 was. Every structural fact
# below held while the initializer silently did not run, so the structural half
# constrains the image and the darwin run is what settles it.
produce_macho_mod_init() {
    engine=$1
    target=$2
    bin=$3

    fields=$(macho_section_fields "$bin" __DATA __mod_init_func) || return 2
    set -- $fields
    mi_addr=$1; mi_size=$2; mi_align=$4; mi_flags=$5

    [ $((mi_flags & 0xFF)) -eq 9 ] || {
        echo "link: macho-mod-init: __mod_init_func has section type $((mi_flags & 0xFF)), expected 9 (S_MOD_INIT_FUNC_POINTERS); dyld dispatches on the type and never runs any other" >&2
        return 1
    }
    [ "$mi_size" -eq 8 ] || {
        echo "link: macho-mod-init: __mod_init_func is $mi_size bytes, expected one 8-byte entry" >&2
        return 1
    }
    [ "$mi_align" -ge 3 ] || {
        echo "link: macho-mod-init: __mod_init_func is 2^$mi_align aligned, expected at least pointer alignment" >&2
        return 1
    }

    # the entry is an in-image pointer, so a PIE must slide it: with no rebase row
    # dyld would call whatever the unslid address happens to land on.
    mi_hex=$(printf '0x%X' "$mi_addr")
    rebases=$(macho_objdump --macho --rebase "$bin") || return 2
    [ "$(printf '%s\n' "$rebases" | grep -F -c "$mi_hex")" -eq 1 ] || {
        echo "link: macho-mod-init: __mod_init_func has no rebase row at $mi_hex" >&2
        return 1
    }

    if [ "$target" = x86_64-darwin ]; then
        out=$(mktemp)
        run_captured native "$target" "$bin" "$out" || { rm -f "$out"; return 1; }
        rm -f "$out"
        if [ "$run_status" -ne 0 ]; then
            report_run_failure "macho-mod-init: the native PIE" "$run_status" "$run_out"
            return 1
        fi
        [ "$run_out" = "init ok" ] || {
            echo "link: macho-mod-init: dyld did not run the __mod_init_func entry" >&2
            diff_expected_actual "init ok" "$run_out"
            return 1
        }
    fi

    echo "mod_init_func=type9-rebased"
}

produce_macho_mod_init "$@"
