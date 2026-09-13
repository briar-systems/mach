#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_pe_local_import <engine> <leg> <binary>
# Verify the DefinedLocalImport shape from real Clang objects. Duplicate live
# `__imp_local_add` sites share one cell even though an earlier import archive was
# selected; a losing SELECT_ANY body's `__imp_dead_local` site leaves no cell.
# provider.o's direct local_add call still addresses the function. The sole cell
# stores that function's image VA, receives the sole DIR64 base relocation, and
# produces neither an import directory nor an IAT (#2552).
produce_pe_local_import() {
    bin=$3
    elfanew=$(read_le_uint "$bin" 60 4)
    nsec=$(read_le_uint "$bin" $((elfanew + 6)) 2)
    optsize=$(read_le_uint "$bin" $((elfanew + 20)) 2)
    sec=$((elfanew + 24 + optsize))
    magic=$(read_le_uint "$bin" $((elfanew + 24)) 2)
    if [ "$magic" != "523" ]; then
        echo "link: pe-local-import: not a PE32+ image (optional-header magic $magic)" >&2
        return 2
    fi

    imports=$(bash "$(dirname "$0")/../../check/pe-imports.sh" "$@") || return $?
    iat_dir=$((elfanew + 24 + 112 + 12 * 8))
    iat_rva=$(read_le_uint "$bin" "$iat_dir" 4)
    iat_size=$(read_le_uint "$bin" $((iat_dir + 4)) 4)
    if [ "$imports" != "no-import-table" ] || [ "$iat_rva" -ne 0 ] || [ "$iat_size" -ne 0 ]; then
        echo "link: pe-local-import: local_add leaked into loader imports/IAT ($imports, $iat_rva/$iat_size)" >&2
        return 2
    fi

    first_suffix=$(find_unique_hex "$bin" ffc04883c428c3) || {
        echo "link: pe-local-import: first indirect-call suffix is not unique" >&2
        return 2
    }
    second_suffix=$(find_unique_hex "$bin" 05785634124883c428c3) || {
        echo "link: pe-local-import: second indirect-call suffix is not unique" >&2
        return 2
    }
    direct_suffix=$(find_unique_hex "$bin" 83c0024883c428c3) || {
        echo "link: pe-local-import: direct-call suffix is not unique" >&2
        return 2
    }
    local_off=$(find_unique_hex "$bin" 8d4123c36666662e0f1f840000000000) || {
        echo "link: pe-local-import: local definition marker is not unique" >&2
        return 2
    }

    first_patch=$((first_suffix - 4))
    second_patch=$((second_suffix - 4))
    direct_patch=$((direct_suffix - 4))
    if [ "$(read_le_uint "$bin" $((first_patch - 2)) 2)" -ne 5631 ] \
        || [ "$(read_le_uint "$bin" $((second_patch - 2)) 2)" -ne 5631 ] \
        || [ "$(read_le_uint "$bin" $((direct_patch - 1)) 1)" -ne 232 ]; then
        echo "link: pe-local-import: recovered displacement is not owned by the expected call opcode" >&2
        return 2
    fi

    first_rva=$(pe_off_to_rva "$bin" "$sec" "$nsec" "$first_patch") || return 2
    second_rva=$(pe_off_to_rva "$bin" "$sec" "$nsec" "$second_patch") || return 2
    direct_rva=$(pe_off_to_rva "$bin" "$sec" "$nsec" "$direct_patch") || return 2
    local_rva=$(pe_off_to_rva "$bin" "$sec" "$nsec" "$local_off") || return 2
    first_disp=$(read_le_uint "$bin" "$first_patch" 4)
    second_disp=$(read_le_uint "$bin" "$second_patch" 4)
    direct_disp=$(read_le_uint "$bin" "$direct_patch" 4)
    [ "$first_disp" -ge 2147483648 ] && first_disp=$((first_disp - 4294967296))
    [ "$second_disp" -ge 2147483648 ] && second_disp=$((second_disp - 4294967296))
    [ "$direct_disp" -ge 2147483648 ] && direct_disp=$((direct_disp - 4294967296))
    first_target=$((first_rva + 4 + first_disp))
    second_target=$((second_rva + 4 + second_disp))
    direct_target=$((direct_rva + 4 + direct_disp))
    if [ "$first_target" -ne "$second_target" ]; then
        echo "link: pe-local-import: duplicate aliases target different cells $first_target/$second_target" >&2
        return 2
    fi
    if [ "$direct_target" -ne "$local_rva" ] || [ "$first_target" -eq "$local_rva" ]; then
        echo "link: pe-local-import: direct/indirect targets do not preserve one level of indirection" >&2
        return 2
    fi

    cell_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$first_target") || {
        echo "link: pe-local-import: pointer-cell RVA $first_target is in no section" >&2
        return 2
    }
    image_base=$(read_le_uint "$bin" $((elfanew + 24 + 24)) 8)
    cell_value=$(read_le_uint "$bin" "$cell_off" 8)
    if [ "$cell_value" -ne $((image_base + local_rva)) ]; then
        echo "link: pe-local-import: cell contains $cell_value, expected local VA $((image_base + local_rva))" >&2
        return 2
    fi

    reloc_dir=$((elfanew + 24 + 112 + 5 * 8))
    reloc_rva=$(read_le_uint "$bin" "$reloc_dir" 4)
    reloc_size=$(read_le_uint "$bin" $((reloc_dir + 4)) 4)
    reloc_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$reloc_rva") || {
        echo "link: pe-local-import: base-relocation directory is missing" >&2
        return 2
    }
    cursor=0
    dir64_count=0
    dir64_rva=0
    while [ "$cursor" -lt "$reloc_size" ]; do
        block=$((reloc_off + cursor))
        page=$(read_le_uint "$bin" "$block" 4)
        block_size=$(read_le_uint "$bin" $((block + 4)) 4)
        if [ "$block_size" -lt 8 ] || [ $((block_size % 2)) -ne 0 ] \
            || [ $((cursor + block_size)) -gt "$reloc_size" ]; then
            echo "link: pe-local-import: malformed base-relocation block" >&2
            return 2
        fi
        entries=$(((block_size - 8) / 2))
        i=0
        while [ "$i" -lt "$entries" ]; do
            entry=$(read_le_uint "$bin" $((block + 8 + i * 2)) 2)
            type=$((entry >> 12))
            if [ "$type" -eq 10 ]; then
                dir64_count=$((dir64_count + 1))
                dir64_rva=$((page + (entry & 4095)))
            elif [ "$type" -ne 0 ]; then
                echo "link: pe-local-import: unexpected base-relocation type $type" >&2
                return 2
            fi
            i=$((i + 1))
        done
        cursor=$((cursor + block_size))
    done
    if [ "$cursor" -ne "$reloc_size" ] || [ "$dir64_count" -ne 1 ] \
        || [ "$dir64_rva" -ne "$first_target" ]; then
        echo "link: pe-local-import: DIR64 set is $dir64_count at $dir64_rva, expected the one cell $first_target" >&2
        return 2
    fi

    echo "imports=0"
    echo "local_cells=1"
    echo "indirect_targets=local_cell"
    echo "direct_target=local_definition"
    echo "cell_value=local_definition"
    echo "base_reloc=local_cell"
}

produce_pe_local_import "$@"
