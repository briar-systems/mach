#!/usr/bin/env bash
. "$(dirname "$0")/common.sh"

# produce_macho_header_span <engine> <leg> <binary>
# Assert that a darwin image whose load commands exceed one page linked, and that
# its header agrees with the span the linker reserved below the first segment.
#
# The Mach-O header carries an 80-byte section_64 per named section and an
# LC_LOAD_DYLIB per dependency, so it grows with the image; `sizeofcmds` is a
# uint32 and the format caps it nowhere near a page. A fixed one-page reservation
# refused the link outright (#2690).
#
# Size alone would be a weak observable. The reservation is what places __TEXT:
# the header occupies the gap between the image base and the first segment, so a
# writer that widened the gap on its own would slide __TEXT, and __PAGEZERO with
# it, below the base - the defect of #2599, which every size check would still
# pass. The framing is therefore asserted alongside: __PAGEZERO spans the whole low
# region, __TEXT is based exactly where __PAGEZERO ends, and the header sits at
# file offset 0 inside it.
#
# The page size is read from the image's own cputype rather than passed in, since
# the leg here is linux and darwin maps 16 KiB pages on arm64 and 4 KiB on x86_64.
produce_macho_header_span() {
    bin=$3
    magic=$(read_le_uint "$bin" 0 4)
    [ "$magic" = "4277009103" ] || { echo "link: macho-header-span: not a 64-bit mach-o (magic $magic)" >&2; return 2; }

    cputype=$(read_le_uint "$bin" 4 4)
    case "$cputype" in
        16777223) page=4096  ;;   # CPU_TYPE_X86_64
        16777228) page=16384 ;;   # CPU_TYPE_ARM64
        *) echo "link: macho-header-span: unexpected cputype $cputype" >&2; return 2 ;;
    esac

    ncmds=$(read_le_uint "$bin" 16 4)
    sizeofcmds=$(read_le_uint "$bin" 20 4)

    pagezero_size=
    text_vmaddr=
    text_fileoff=
    sect_addr=
    sect_fileoff=
    dylibs=0
    off=32
    i=0
    while [ "$i" -lt "$ncmds" ]; do
        cmd=$(read_le_uint "$bin" "$off" 4)
        cmdsize=$(read_le_uint "$bin" $((off + 4)) 4)
        case "$cmd" in
            25)   # LC_SEGMENT_64
                name=$(dd if="$bin" bs=1 skip=$((off + 8)) count=16 2>/dev/null | tr '\0' '\n' | head -n 1)
                if [ "$name" = "__PAGEZERO" ]; then
                    pagezero_size=$(read_le_uint "$bin" $((off + 32)) 8)
                fi
                if [ "$name" = "__TEXT" ]; then
                    text_vmaddr=$(read_le_uint "$bin" $((off + 24)) 8)
                    text_fileoff=$(read_le_uint "$bin" $((off + 40)) 8)
                    nsects=$(read_le_uint "$bin" $((off + 64)) 4)
                    [ "$nsects" -gt 0 ] || { echo "link: macho-header-span: __TEXT carries no section" >&2; return 2; }
                    sect_addr=$(read_le_uint "$bin" $((off + 72 + 32)) 8)
                    sect_fileoff=$(read_le_uint "$bin" $((off + 72 + 48)) 4)
                fi
                ;;
            12) dylibs=$((dylibs + 1)) ;;   # LC_LOAD_DYLIB
        esac
        [ "$cmdsize" -ge 8 ] || { echo "link: macho-header-span: zero-size load command" >&2; return 2; }
        off=$((off + cmdsize))
        i=$((i + 1))
    done

    [ -n "$pagezero_size" ] || { echo "link: macho-header-span: no __PAGEZERO" >&2; return 2; }
    [ -n "$text_vmaddr" ]   || { echo "link: macho-header-span: no __TEXT" >&2; return 2; }

    # the reservation is the gap the header fills: from __TEXT's start to its first
    # section's content.
    reservation=$((sect_addr - text_vmaddr))

    printf 'sizeofcmds_over_one_page=%s\n' "$(( sizeofcmds > page ))"
    printf 'multiple_dylib_loads=%s\n' "$(( dylibs > 1 ))"
    printf 'reservation_whole_pages=%s\n' "$(( reservation > 0 && reservation % page == 0 ))"
    printf 'header_fits_reservation=%s\n' "$(( 32 + sizeofcmds <= reservation ))"
    printf 'first_section_fileoff_is_reservation=%s\n' "$(( sect_fileoff == reservation ))"
    printf 'pagezero_vmsize=0x%x\n' "$pagezero_size"
    printf 'text_vmaddr=0x%x\n' "$text_vmaddr"
    printf 'text_fileoff=%s\n' "$text_fileoff"
    printf 'pagezero_abuts_text=%s\n' "$(( pagezero_size == text_vmaddr ))"
}

produce_macho_header_span "$@"
