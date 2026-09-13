#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_pe_codeview <engine> <leg> <binary>
# verify that the real clang COFF fixture in pe-foreign-codeview retained its
# `.debug$S` records and that both SECREL+SECTION pairs resolve to the exact
# linked code address and final PE section. The fixture's `.text` requires 8 KiB
# alignment while PE's first section begins at RVA 0x1000, so deriving the
# expected offset from the section header (rather than linker content) exercises
# the alignment prefix that originally made the two bases differ.
produce_pe_codeview() {
    bin=$3
    elfanew=$(read_le_uint "$bin" 60 4)
    nsec=$(read_le_uint "$bin" $((elfanew + 6)) 2)
    optsize=$(read_le_uint "$bin" $((elfanew + 20)) 2)
    sec=$((elfanew + 24 + optsize))

    magic=$(read_le_uint "$bin" $((elfanew + 24)) 2)
    if [ "$magic" != "523" ]; then
        echo "link: pe-codeview: not a PE32+ image (optional-header magic $magic)" >&2
        return 2
    fi

    text_index=0
    text_rva=0
    debug_off=0
    debug_size=0
    i=0
    while [ "$i" -lt "$nsec" ]; do
        base=$((sec + i * 40))
        name=$(dd if="$bin" bs=1 skip="$base" count=8 2>/dev/null | od -An -tx1 | tr -d ' \n')
        case "$name" in
            2e74657874000000)
                text_index=$((i + 1))
                text_rva=$(read_le_uint "$bin" $((base + 12)) 4)
                ;;
            2e64656275672453)
                debug_size=$(read_le_uint "$bin" $((base + 16)) 4)
                debug_off=$(read_le_uint "$bin" $((base + 20)) 4)
                ;;
        esac
        i=$((i + 1))
    done
    if [ "$text_index" -eq 0 ] || [ "$debug_off" -eq 0 ] || [ "$debug_size" -lt 178 ]; then
        echo 'link: pe-codeview: missing .text or complete .debug$S section' >&2
        return 2
    fi

    code_off=$(find_unique_hex "$bin" 4883ec28b928000000) || {
        echo "link: pe-codeview: codeview_answer signature is not unique" >&2
        return 2
    }
    code_rva=$(pe_off_to_rva "$bin" "$sec" "$nsec" "$code_off") || {
        echo "link: pe-codeview: codeview_answer is outside the PE sections" >&2
        return 2
    }
    if [ "$code_rva" -lt "$text_rva" ]; then
        echo "link: pe-codeview: code precedes its .text section" >&2
        return 2
    fi
    expected=$((code_rva - text_rva))

    secrel_a=$(read_le_uint "$bin" $((debug_off + 0x68)) 4)
    section_a=$(read_le_uint "$bin" $((debug_off + 0x6c)) 2)
    secrel_b=$(read_le_uint "$bin" $((debug_off + 0xac)) 4)
    section_b=$(read_le_uint "$bin" $((debug_off + 0xb0)) 2)
    if [ "$secrel_a" -ne "$expected" ] || [ "$secrel_b" -ne "$expected" ]; then
        echo "link: pe-codeview: SECREL fields $secrel_a/$secrel_b, want $expected" >&2
        return 2
    fi
    if [ "$section_a" -ne "$text_index" ] || [ "$section_b" -ne "$text_index" ]; then
        echo "link: pe-codeview: SECTION fields $section_a/$section_b, want $text_index" >&2
        return 2
    fi
    if [ "$text_rva" -ne 4096 ] || [ "$code_rva" -lt 8192 ] || [ "$expected" -lt 4096 ]; then
        echo "link: pe-codeview: fixture no longer covers first-section alignment padding" >&2
        return 2
    fi

    echo "codeview_section=present"
    echo "secrel_fields=exact"
    echo "section_fields=exact"
    echo "alignment_prefix=covered"
}

produce_pe_codeview "$@"
