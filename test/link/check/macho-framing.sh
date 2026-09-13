#!/usr/bin/env bash
. "$(dirname "$0")/common.sh"

# produce_macho_framing <engine> <leg> <binary>
# Walk a Mach-O executable's load commands and report how the image is FRAMED: the
# __PAGEZERO span, __TEXT's base, whether __PAGEZERO ends exactly where __TEXT
# begins, then one line per LC_SEGMENT_64 naming its section commands, and finally
# which entry command the image carries.
#
# The framing is what a static (LC_UNIXTHREAD) and a dyld-loaded (LC_MAIN) image
# must have IN COMMON - the entry command is the only line that may differ between
# the two goldens (#2599, where the non-PIE image was based a page below
# DARWIN_BASE_ADDR, carried a __PAGEZERO one page short of 4 GiB, and emitted no
# section commands at all, leaving `llvm-objdump -d` with nothing to disassemble).
# Addresses other than the base are deliberately left out: they move with the
# program's size, and the fact under test is the framing, not the layout.
produce_macho_framing() {
    bin=$3
    magic=$(read_le_uint "$bin" 0 4)
    [ "$magic" = "4277009103" ] || { echo "link: macho-framing: not a 64-bit mach-o (magic $magic)" >&2; return 2; }
    ncmds=$(read_le_uint "$bin" 16 4)

    pagezero_size=
    text_vmaddr=
    entry=none
    segs=$(mktemp)
    off=32
    i=0
    while [ "$i" -lt "$ncmds" ]; do
        cmd=$(read_le_uint "$bin" "$off" 4)
        cmdsize=$(read_le_uint "$bin" $((off + 4)) 4)
        case "$cmd" in
            25)   # LC_SEGMENT_64
                name=$(dd if="$bin" bs=1 skip=$((off + 8)) count=16 2>/dev/null | tr '\0' '\n' | head -n 1)
                vmaddr=$(read_le_uint "$bin" $((off + 24)) 8)
                vmsize=$(read_le_uint "$bin" $((off + 32)) 8)
                nsects=$(read_le_uint "$bin" $((off + 64)) 4)
                [ "$name" = "__PAGEZERO" ] && pagezero_size=$vmsize
                [ "$name" = "__TEXT" ] && text_vmaddr=$vmaddr
                sects=
                k=0
                while [ "$k" -lt "$nsects" ]; do
                    sh=$((off + 72 + k * 80))
                    sname=$(dd if="$bin" bs=1 skip="$sh" count=16 2>/dev/null | tr '\0' '\n' | head -n 1)
                    if [ -z "$sects" ]; then sects=$sname; else sects="$sects,$sname"; fi
                    k=$((k + 1))
                done
                [ -n "$sects" ] || sects=-
                printf 'seg %s nsects=%s sects=%s\n' "$name" "$nsects" "$sects" >>"$segs"
                ;;
            5)          entry=LC_UNIXTHREAD ;;   # LC_UNIXTHREAD
            2147483688) entry=LC_MAIN ;;         # LC_MAIN (0x80000028)
        esac
        [ "$cmdsize" -ge 8 ] || { rm -f "$segs"; echo "link: macho-framing: zero-size load command" >&2; return 2; }
        off=$((off + cmdsize))
        i=$((i + 1))
    done

    [ -n "$pagezero_size" ] || { rm -f "$segs"; echo "link: macho-framing: no __PAGEZERO" >&2; return 2; }
    [ -n "$text_vmaddr" ]   || { rm -f "$segs"; echo "link: macho-framing: no __TEXT" >&2; return 2; }

    printf 'pagezero_vmsize=0x%x\n' "$pagezero_size"
    printf 'text_vmaddr=0x%x\n' "$text_vmaddr"
    printf 'pagezero_abuts_text=%s\n' "$(( pagezero_size == text_vmaddr ))"
    cat "$segs"
    rm -f "$segs"
    printf 'entry=%s\n' "$entry"
}

produce_macho_framing "$@"
