#!/usr/bin/env bash
. "$(dirname "$0")/common.sh"

# produce_image_sections <engine> <leg> <binary>
# the linked ELF image's own account of its data (#3362): a freestanding image
# built with `of = "elf"` must carry, in the file and not merely in its program
# headers, the section names and data symbols its objects had. #3362 found an
# image whose section table was synthesized from the LOAD segments - every
# allocated section named `.text`/`.data`/`.rodata` by segment protection, sized
# by p_filesz and typed PROGBITS - so a `#[section(...)]` name was gone, `.bss`
# appeared as a second `.data` of size 0, and the symbol table held functions
# only. a loader or debugger that finds structures by section name, and anything
# that resolves a data address to a symbol, had nothing to work with.
#
# read host-side with readelf, so the case works cross-built on any leg and never
# executes the image. normalized to names, types, sizes and symbol bindings: no
# address appears, so the same golden holds for every ISA.
produce_image_sections() {
    b=$3
    command -v readelf >/dev/null 2>&1 || {
        echo "link: image-sections: readelf not found (install the 'binutils' package)" >&2; return 2
    }

    # `index<TAB>name<TAB>type<TAB>size` per section header, the bracketed index
    # split off so a name is always one field
    sections=$(readelf -SW "$b" 2>/dev/null | sed -n 's/^ *\[ *\([0-9][0-9]*\)\] *//p' \
        | awk '{ printf "%d\t%s\t%s\t%d\n", NR - 1, $1, $2, strtonum("0x" $5) }')
    symbols=$(readelf -sW "$b" 2>/dev/null)

    sec_field() { printf '%s\n' "$sections" | awk -F'\t' -v n="$1" -v c="$2" '$2 == n { print $c; exit }'; }
    sec_by_index() { printf '%s\n' "$sections" | awk -F'\t' -v i="$1" '$1 == i { print $2; exit }'; }

    # the `#[section(".limine_requests_start")]` name reached the image, with the
    # PROGBITS type its contents require
    echo "limine_section=$(sec_field .limine_requests_start 3)"

    # .bss is NOBITS at its MEMORY size: p_memsz was always right, what was
    # missing was the section saying how much of the mapping is zeroed
    echo "bss_type=$(sec_field .bss 3)"
    echo "bss_size=$(sec_field .bss 4)"

    # nothing was renamed into the generic `.data` bucket: the fixture defines no
    # `.data` section at all, so one appearing means the names were synthesized
    echo "generic_data_sections=$(printf '%s\n' "$sections" | awk -F'\t' '$2 == ".data"' | wc -l)"

    # the data objects are in the image symbol table as OBJECT symbols with their
    # real st_size, each bound to the section it actually lives in
    for sym in requests_start pml4; do
        line=$(printf '%s\n' "$symbols" | awk -v s="$sym" '$8 == s { print; exit }')
        if [ -z "$line" ]; then
            echo "sym_${sym}=missing"
            continue
        fi
        echo "sym_${sym}=$(printf '%s\n' "$line" | awk '{ print $4 }')"
        echo "size_${sym}=$(printf '%s\n' "$line" | awk '{ print $3 }')"
        echo "section_${sym}=$(sec_by_index "$(printf '%s\n' "$line" | awk '{ print $7 }')")"
    done

    # the entry point is still a FUNC: widening the table to data symbols did not
    # reclassify what was already in it
    echo "sym_start=$(printf '%s\n' "$symbols" | awk '$8 == "_start" { print $4; exit }')"
}

produce_image_sections "$@"
