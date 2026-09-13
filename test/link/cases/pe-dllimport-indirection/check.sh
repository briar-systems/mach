#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_pe_dllimport <engine> <leg> <binary>
# Verify qz.o's IMAGE_REL_AMD64_REL32 sites against `__imp_Sleep` and indirect-only
# `__imp_GetTickCount` target their IAT cells. Stable byte signatures locate each
# four-byte displacement, so S = P + 4 + disp proves the recovered COFF -4 addend
# as well as the target. Mach also calls Sleep directly; an IAT size of three
# thunks (two bindings + one descriptor terminator) proves that pair deduplicated
# while the indirect-only export still receives a slot and no call stub.
produce_pe_dllimport() {
    bin=$3
    elfanew=$(read_le_uint "$bin" 60 4)
    nsec=$(read_le_uint "$bin" $((elfanew + 6)) 2)
    optsize=$(read_le_uint "$bin" $((elfanew + 20)) 2)
    sec=$((elfanew + 24 + optsize))
    magic=$(read_le_uint "$bin" $((elfanew + 24)) 2)
    if [ "$magic" != "523" ]; then
        echo "link: pe-dllimport: not a PE32+ image (optional-header magic $magic)" >&2
        return 2
    fi

    imports=$(bash "$(dirname "$0")/../../check/pe-imports.sh" "$@") || return $?
    want_imports='kernel32.dll:GetTickCount
kernel32.dll:Sleep'
    if [ "$imports" != "$want_imports" ]; then
        echo "link: pe-dllimport: imports are '$imports', want two canonical exports" >&2
        return 2
    fi

    iat_dir=$((elfanew + 24 + 112 + 12 * 8))
    iat_rva=$(read_le_uint "$bin" "$iat_dir" 4)
    iat_size=$(read_le_uint "$bin" $((iat_dir + 4)) 4)
    if [ "$iat_rva" -eq 0 ] || [ "$iat_size" -ne 24 ]; then
        echo "link: pe-dllimport: IAT is RVA=$iat_rva size=$iat_size, want two entries" >&2
        return 2
    fi

    sleep_off=$(find_unique_hex "$bin" 4883ec28b907000000ff15) || {
        echo "link: pe-dllimport: qz_indirect Sleep signature is not unique" >&2
        return 2
    }
    tick_off=$(find_unique_hex "$bin" 904883c42848ff25) || {
        echo "link: pe-dllimport: qz_indirect GetTickCount signature is not unique" >&2
        return 2
    }

    sleep_patch=$((sleep_off + 11))
    tick_patch=$((tick_off + 8))
    sleep_rva=$(pe_off_to_rva "$bin" "$sec" "$nsec" "$sleep_patch") || return 2
    tick_rva=$(pe_off_to_rva "$bin" "$sec" "$nsec" "$tick_patch") || return 2
    sleep_disp=$(read_le_uint "$bin" "$sleep_patch" 4)
    tick_disp=$(read_le_uint "$bin" "$tick_patch" 4)
    [ "$sleep_disp" -ge 2147483648 ] && sleep_disp=$((sleep_disp - 4294967296))
    [ "$tick_disp" -ge 2147483648 ] && tick_disp=$((tick_disp - 4294967296))
    sleep_target=$((sleep_rva + 4 + sleep_disp))
    tick_target=$((tick_rva + 4 + tick_disp))
    second_iat=$((iat_rva + 8))
    if ! { [ "$sleep_target" -eq "$iat_rva" ] && [ "$tick_target" -eq "$second_iat" ]; } \
       && ! { [ "$tick_target" -eq "$iat_rva" ] && [ "$sleep_target" -eq "$second_iat" ]; }; then
        echo "link: pe-dllimport: foreign targets $sleep_target/$tick_target miss IAT $iat_rva/$second_iat" >&2
        return 2
    fi

    echo "$imports"
    echo "iat_entries=2"
    echo "foreign_indirect_target=iat"
}

produce_pe_dllimport "$@"
