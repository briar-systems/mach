#!/usr/bin/env bash
. "$(dirname "$0")/common.sh"

# produce_field <engine> <leg> <binary>
# emits the canonical structural fact for the artifact's format, dispatched on its
# leading magic bytes so the reader is independent of the leg the case ran on.
produce_field() {
    bin=$3
    magic=$(dd if="$bin" bs=1 count=4 2>/dev/null | od -An -tx1 | tr -d ' \n')
    case "$magic" in
        7f454c46)  field_elf "$bin" ;;      # 0x7F 'E' 'L' 'F' -> ELF
        4d5a*)     field_pe "$bin" ;;       # 'MZ' DOS stub -> PE/COFF
        cffaedfe*) field_macho "$bin" ;;    # MH_MAGIC_64 (little-endian)
        *) echo "link: field: unrecognized binary format (magic $magic)" >&2; return 2 ;;
    esac
}

produce_field "$@"
