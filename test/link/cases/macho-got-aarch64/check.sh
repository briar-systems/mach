#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_macho_got_aarch64 <engine> <leg> <binary> [<g-binary>] [<profile>]
#
# The aarch64 `ext val` read is an adrp/ldr pair carrying the GOT kinds, which
# Mach-O spells ARM64_RELOC_GOT_LOAD_PAGE21 / GOT_LOAD_PAGEOFF12 (#3675). Three
# facts, each read by an external decoder or straight from the bytes:
#
#  - main.o carries matched PAGE21/PAGEOFF12 pairs for every imported cell,
#    symbol based, addressed to the cell's C spelling;
#  - the two cells provider.o defines are reached through linker-owned slots in
#    __DATA_CONST,__got, each rebased and each holding the address of bytes in
#    __DATA,__data whose values are the cells' initializers (7 and 35);
#  - libSystem's `__stderrp` is reached through one __GOT slot dyld binds.
#
# On a native arm64 macOS leg the image is also executed and its output compared
# with the runtime contract, so the structural facts and the loaded values are
# proven by one fixture.
produce_macho_got_aarch64() {
    target=$2
    bin=$3
    profile=$5
    casedir=$(dirname "$(dirname "$(dirname "$bin")")")
    tool=$(resolve_readobj) || { echo "link: macho-got-aarch64: llvm-readobj is required" >&2; return 2; }

    obj=$casedir/out/aarch64-darwin/$profile/obj/case/main.o
    [ -f "$obj" ] || { echo "link: macho-got-aarch64: no main.o at ${obj#"$casedir"/}" >&2; return 2; }
    relocs=$("$tool" -r "$obj") || return 2
    for sym in _cell8 _cell64 ___stderrp; do
        page=$(printf '%s\n' "$relocs" | grep -c " 1 2 1 ARM64_RELOC_GOT_LOAD_PAGE21 0 $sym\$")
        off=$(printf '%s\n' "$relocs" | grep -c " 0 2 1 ARM64_RELOC_GOT_LOAD_PAGEOFF12 0 $sym\$")
        [ "$page" -ge 1 ] && [ "$off" -eq "$page" ] || {
            echo "link: macho-got-aarch64: $sym has $page GOT_LOAD_PAGE21 and $off GOT_LOAD_PAGEOFF12 records, expected matched pairs" >&2
            printf '%s\n' "$relocs" | sed 's/^/    /' >&2
            return 1
        }
    done
    echo "object=got_load_pairs_per_import"

    data_fields=$(macho_segment_fields "$bin" __DATA) || return 2
    set -- $data_fields
    data_vm=$1; data_size=$2; data_file=$3
    relro_fields=$(macho_segment_fields "$bin" __DATA_CONST) || return 2
    set -- $relro_fields
    relro_vm=$1; relro_file=$3
    got_fields=$(macho_section_fields "$bin" __DATA_CONST __got) || return 2
    set -- $got_fields
    got_addr=$1; got_size=$2
    [ "$got_size" -eq 16 ] || {
        echo "link: macho-got-aarch64: __DATA_CONST,__got holds $got_size bytes, expected two 8-byte slots" >&2
        return 1
    }

    rebases=$(macho_objdump --macho --rebase "$bin") || return 2
    seen=
    slot=0
    while [ "$slot" -lt 2 ]; do
        slot_va=$((got_addr + slot * 8))
        slot_hex=$(printf '0x%X' "$slot_va")
        [ "$(printf '%s\n' "$rebases" | grep -c "__DATA_CONST *__got *$slot_hex ")" -eq 1 ] || {
            echo "link: macho-got-aarch64: slot $slot_hex has no unique rebase row" >&2
            printf '%s\n' "$rebases" | sed 's/^/    /' >&2
            return 1
        }
        value=$(read_le_uint "$bin" $((relro_file + slot_va - relro_vm)) 8)
        [ "$value" -ge "$data_vm" ] && [ "$value" -lt $((data_vm + data_size)) ] || {
            echo "link: macho-got-aarch64: slot $slot_hex points outside __DATA" >&2
            return 1
        }
        cell_file=$((data_file + value - data_vm))
        case "$(read_le_uint "$bin" "$cell_file" 8)" in
            7)  seen="$seen cell8" ;;
            35) seen="$seen cell64" ;;
            *)  echo "link: macho-got-aarch64: slot $slot_hex points at neither cell's bytes" >&2; return 1 ;;
        esac
        slot=$((slot + 1))
    done
    case "$seen" in *cell8*cell64*|*cell64*cell8*) ;; *)
        echo "link: macho-got-aarch64: the two slots do not cover both cells:$seen" >&2; return 1 ;;
    esac
    echo "local_slots=rebased_to_cells"

    binds=$(macho_objdump --macho --bind "$bin") || return 2
    [ "$(printf '%s\n' "$binds" | grep -c '^__GOT .*libSystem .*___stderrp$')" -eq 1 ] || {
        echo "link: macho-got-aarch64: no unique __GOT bind of ___stderrp to libSystem" >&2
        printf '%s\n' "$binds" | sed 's/^/    /' >&2
        return 1
    }
    echo "import_slot=libSystem-bind"

    if [ "$target" = aarch64-darwin ]; then
        out=$(mktemp)
        run_captured native "$target" "$bin" "$out" || { rm -f "$out"; return 1; }
        rm -f "$out"
        expected=$(printf '%s\n' 'cell8=7' 'cell64=35' 'stderrp=1')
        if [ "$run_status" -ne 0 ]; then
            report_run_failure "macho-got-aarch64: the native image" "$run_status" "$run_out"
            return 1
        fi
        [ "$run_out" = "$expected" ] || {
            echo "link: macho-got-aarch64: the native image ran to completion but computed wrong values" >&2
            diff_expected_actual "$expected" "$run_out"
            return 1
        }
    fi
}

produce_macho_got_aarch64 "$@"
