#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_pe_exceptions <engine> <leg> <binary>
# verify that the real clang COFF fixture in pe-foreign-unwind is represented by
# one valid RUNTIME_FUNCTION row in the published PE exception directory. the
# fixture's stable first nine instruction bytes identify qz_answer without a
# linked symbol table; its patched call displacement begins immediately after
# that signature. the row's 22-byte extent and exact UNWIND_INFO distinguish the
# foreign entry from mach-generated unwind rows in the same sorted table.
produce_pe_exceptions() {
    bin=$3
    elfanew=$(read_le_uint "$bin" 60 4)
    nsec=$(read_le_uint "$bin" $((elfanew + 6)) 2)
    optsize=$(read_le_uint "$bin" $((elfanew + 20)) 2)
    sec=$((elfanew + 24 + optsize))

    magic=$(read_le_uint "$bin" $((elfanew + 24)) 2)
    if [ "$magic" != "523" ]; then
        echo "link: pe-exceptions: not a PE32+ image (optional-header magic $magic)" >&2
        return 2
    fi

    exc_rva=$(read_le_uint "$bin" $((elfanew + 24 + 112 + 3 * 8)) 4)
    exc_size=$(read_le_uint "$bin" $((elfanew + 24 + 112 + 3 * 8 + 4)) 4)
    if [ "$exc_rva" -eq 0 ] || [ "$exc_size" -eq 0 ] || [ $((exc_size % 12)) -ne 0 ]; then
        echo "link: pe-exceptions: missing or malformed exception directory" >&2
        return 2
    fi
    exc_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$exc_rva") || {
        echo "link: pe-exceptions: exception RVA $exc_rva is in no section" >&2
        return 2
    }

    found=0
    i=0
    while [ $((i * 12)) -lt "$exc_size" ]; do
        row=$((exc_off + i * 12))
        begin=$(read_le_uint "$bin" "$row" 4)
        end=$(read_le_uint "$bin" $((row + 4)) 4)
        unwind=$(read_le_uint "$bin" $((row + 8)) 4)
        begin_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$begin") || {
            echo "link: pe-exceptions: begin RVA $begin is in no section" >&2
            return 2
        }
        sig=$(dd if="$bin" bs=1 skip="$begin_off" count=9 2>/dev/null | od -An -tx1 | tr -d ' \n')
        if [ "$sig" = "4883ec28b928000000" ]; then
            found=$((found + 1))
            if [ $((end - begin)) -ne 22 ]; then
                echo "link: pe-exceptions: qz_answer row has length $((end - begin)), want 22" >&2
                return 2
            fi
            unwind_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$unwind") || {
                echo "link: pe-exceptions: unwind RVA $unwind is in no section" >&2
                return 2
            }
            unwind_bytes=$(dd if="$bin" bs=1 skip="$unwind_off" count=8 2>/dev/null | od -An -tx1 | tr -d ' \n')
            if [ "$unwind_bytes" != "0104010004420000" ]; then
                echo "link: pe-exceptions: qz_answer UNWIND_INFO is $unwind_bytes" >&2
                return 2
            fi
        fi
        i=$((i + 1))
    done
    if [ "$found" -ne 1 ]; then
        echo "link: pe-exceptions: found $found qz_answer runtime rows, want 1" >&2
        return 2
    fi

    echo "exception_directory=present"
    echo "foreign_runtime=published"
    echo "foreign_length=22"
    echo "foreign_unwind=valid"
}

produce_pe_exceptions "$@"
