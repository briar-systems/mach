#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_foreign_unwind <engine> <leg> <binary>
# gdb stops in the innermost mach function and walks out through the C frames
# (the function names of its backtrace, outermost last). the image has one
# `.eh_frame`, the objects' descriptions folded into the image's own, and every
# function an fde in it covers has a row of `.eh_frame_hdr`'s search table, the
# two C functions in particular. the C function nothing calls is collected with
# its fde (#3410), so every fde left starts at a function the image keeps
produce_foreign_unwind() {
    b=$3
    for tool in gdb llvm-dwarfdump llvm-readobj llvm-readelf llvm-nm; do
        command -v "$tool" >/dev/null 2>&1 || { echo "link: foreign-unwind: $tool not found" >&2; return 2; }
    done

    bt=$(gdb -q -batch -nx -ex 'set debuginfod enabled off' -ex 'set pagination off' \
        -ex 'break unwind_inner' -ex 'run' -ex 'bt' "$b" 2>/dev/null \
        | sed -n 's/^#[0-9][0-9]* .* in \([A-Za-z_][A-Za-z0-9_]*\) (.*/\1/p; s/^#0  \([A-Za-z_][A-Za-z0-9_]*\) (.*/\1/p' \
        | tr '\n' ' ' | sed 's/ $//')
    echo "bt=$bt"

    fdes=$(llvm-dwarfdump --eh-frame "$b" 2>/dev/null | sed -n 's/.* FDE cie=[0-9a-f]* pc=\([0-9a-f]*\)\.\.\..*/\1/p' | sed 's/^0*//' | sort -u)
    echo "eh_frame_sections=$(llvm-readelf -SW "$b" 2>/dev/null | grep -cE '\] \.eh_frame +PROGBITS')"
    rows=$(llvm-readobj --unwind "$b" 2>/dev/null | sed -n '/EHFrameHeader {/,/^\.eh_frame section/p' \
        | sed -n 's/.*initial_location: 0x\([0-9a-f]*\).*/\1/p' | sed 's/^0*//' | sort -u)
    if [ -n "$fdes" ] && [ "$fdes" = "$rows" ]; then echo "fdes_indexed=yes"; else echo "fdes_indexed=no"; fi
    for fn in c_middle c_dwarf; do
        at=$(llvm-nm "$b" 2>/dev/null | sed -n "s/^\([0-9a-f]*\) [Tt] $fn\$/\1/p" | sed 's/^0*//')
        if [ -n "$at" ] && printf '%s\n' "$rows" | grep -qx "$at"; then echo "${fn}_indexed=yes"; else echo "${fn}_indexed=no"; fi
    done
    # the function nothing calls is collected, and so is its frame description:
    # every fde left starts at a function the image keeps, both readers take the
    # tables whole, and the object's debug info, which still names the collected
    # function, verifies
    if llvm-nm "$b" 2>/dev/null | grep -q ' c_unused$'; then echo "c_unused=kept"; else echo "c_unused=collected"; fi
    starts=$(llvm-nm "$b" 2>/dev/null | sed -n 's/^\([0-9a-f]*\) [Tt] .*/\1/p' | sed 's/^0*//' | sort -u)
    dangling=$(printf '%s\n' "$fdes" | grep -vxF -f <(printf '%s\n' "$starts") | grep -c .)
    echo "fdes_dangling=$dangling"
    if llvm-dwarfdump --eh-frame "$b" >/dev/null 2>&1 && llvm-readobj --unwind "$b" >/dev/null 2>&1 \
        && llvm-dwarfdump --verify "$b" >/dev/null 2>&1; then
        echo "tables_read=yes"
    else
        echo "tables_read=no"
    fi
}

produce_foreign_unwind "$@"
