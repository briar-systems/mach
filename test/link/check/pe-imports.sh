#!/usr/bin/env bash
. "$(dirname "$0")/common.sh"

# produce_pe_imports <engine> <leg> <binary>
# emit the PE import table's symbol->DLL bindings, one `<dll>:<symbol>` line per
# imported function, sorted. this is the fact the two-level-namespace attribution
# rules decide (#2510) and the one execution cannot observe: a wrongly attributed
# import still links and still runs on the host that happens to export it, so only
# the emitted descriptor distinguishes a correct binding from a lucky one.
#
# walks the PE32+ headers with coreutils alone (no LLVM on the linux legs): the
# import data directory is entry 1 of the optional header's directory array, which
# for PE32+ begins at optional-header offset 112, i.e. e_lfanew + 24 + 112.
produce_pe_imports() {
    bin=$3
    elfanew=$(read_le_uint "$bin" 60 4)
    nsec=$(read_le_uint "$bin" $((elfanew + 6)) 2)
    optsize=$(read_le_uint "$bin" $((elfanew + 20)) 2)
    sec=$((elfanew + 24 + optsize))

    magic=$(read_le_uint "$bin" $((elfanew + 24)) 2)
    if [ "$magic" != "523" ]; then
        echo "link: pe-imports: not a PE32+ image (optional-header magic $magic)" >&2
        return 2
    fi

    imp_rva=$(read_le_uint "$bin" $((elfanew + 24 + 112 + 8)) 4)
    if [ "$imp_rva" -eq 0 ]; then
        echo "no-import-table"
        return 0
    fi
    imp_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$imp_rva") || {
        echo "link: pe-imports: import directory RVA $imp_rva is in no section" >&2
        return 2
    }

    out=$(mktemp)
    d=0
    while :; do
        desc=$((imp_off + d * 20))
        ilt_rva=$(read_le_uint "$bin" "$desc" 4)
        name_rva=$(read_le_uint "$bin" $((desc + 12)) 4)
        iat_rva=$(read_le_uint "$bin" $((desc + 16)) 4)
        # the descriptor array ends at an all-zero entry
        if [ "$ilt_rva" -eq 0 ] && [ "$name_rva" -eq 0 ] && [ "$iat_rva" -eq 0 ]; then break; fi

        name_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$name_rva") || return 2
        dll=$(read_cstr "$bin" "$name_off")

        # prefer the ILT; a linker may leave it zero and describe imports through
        # the IAT alone, which holds the same thunk encoding before the loader runs
        thunks=$ilt_rva
        [ "$thunks" -eq 0 ] && thunks=$iat_rva
        thunk_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$thunks") || return 2

        t=0
        while :; do
            thunk=$((thunk_off + t * 8))
            entry_lo=$(read_le_uint "$bin" "$thunk" 4)
            entry_hi=$(read_le_uint "$bin" $((thunk + 4)) 4)
            [ "$entry_lo" -eq 0 ] && [ "$entry_hi" -eq 0 ] && break
            # Inspect bit 63 as a byte. A by-ordinal thunk is an unsigned u64
            # above the shell's signed arithmetic range, so reading the whole
            # value into `$((...))` would overflow before the test.
            top=$(read_le_uint "$bin" $((thunk + 7)) 1)
            if [ $((top & 128)) -ne 0 ]; then
                ordinal=$(read_le_uint "$bin" "$thunk" 2)
                echo "$dll:#$ordinal" >>"$out"
            else
                hn_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$entry_lo") || return 2
                echo "$dll:$(read_cstr "$bin" $((hn_off + 2)))" >>"$out"
            fi
            t=$((t + 1))
        done
        d=$((d + 1))
    done

    LC_ALL=C sort "$out"
    rm -f "$out"
}

produce_pe_imports "$@"
