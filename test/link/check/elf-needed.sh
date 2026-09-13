#!/usr/bin/env bash
. "$(dirname "$0")/common.sh"

# produce_elf-needed <engine> <leg> <binary>
# the ELF DT_NEEDED observable: one `name` line per shared-library dependency the
# `.dynamic` section actually carries, sorted, or the literal line "none" when the
# image has no PT_DYNAMIC segment at all (a fully static link). mach's own ELF
# writer does not emit a conventional `.dynstr`/`.dynsym` section pair the way a
# hand-parsed byte reader could walk directly (`.dynamic` is the only dynamic-linking
# section it emits), so this shells out to `readelf -d` rather than reimplementing
# ELF dynamic-section parsing; the same tool dependency `symtab` already requires.
produce_elf_needed() {
    b=$3
    command -v readelf >/dev/null 2>&1 || {
        echo "link: elf-needed: readelf not found (install the 'binutils' package)" >&2; return 2
    }
    # readelf prints an explanatory sentence (exit 0) rather than empty output
    # when a static image has no PT_DYNAMIC segment at all, so the absence check
    # is on the filtered NEEDED lines, not on readelf's raw output.
    needed=$(readelf -d "$b" 2>/dev/null | awk -F'[][]' '/\(NEEDED\)/ { print $2 }' | sort)
    if [ -z "$needed" ]; then
        echo "none"
        return 0
    fi
    printf '%s\n' "$needed"
}

produce_elf_needed "$@"
