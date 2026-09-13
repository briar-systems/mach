#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_macho_sections <engine> <leg> <binary>
# Assert that a linked darwin image kept the (segment, section) identity of its
# inputs, which is what a name-based runtime scan needs: libobjc walks
# __DATA,__objc_classlist and reads __DATA,__objc_imageinfo, and dyld calls every
# pointer in __DATA,__mod_init_func (#2606). Merging inputs by KIND alone drops the
# names and the sections do not exist in the output at all, even though their bytes
# are still mapped somewhere inside __data.
#
# Presence is not enough, so this also reads the bytes back. Two clang objects each
# contribute one __objc_classlist pointer, and the section must be ONE contiguous
# 16-byte pointer-aligned run holding both markers in link order - not two sections
# sharing a name, which is what a per-input section would produce and what libobjc's
# single-array walk cannot read.
#
# This image is inspected and never RUN, deliberately. Its objc metadata is
# synthetic - a class list entry has to point at a real Objective-C class object,
# and building one needs the macOS SDK - so the markers that make concatenation
# order checkable here are not addresses at all. libobjc reads that list for real
# on darwin and dereferences every entry, so running this image faults before main
# (#2637). Executing a genuine __mod_init_func initializer is `macho-mod-init`,
# whose payload is valid content a runtime can act on correctly.
produce_macho_sections() {
    target=$2
    bin=$3

    fields=$(macho_section_fields "$bin" __DATA __objc_classlist) || return 2
    set -- $fields
    cl_size=$2; cl_off=$3; cl_align=$4
    [ "$cl_size" -eq 16 ] || {
        echo "link: macho-sections: __objc_classlist is $cl_size bytes, expected the two inputs concatenated into 16" >&2
        return 1
    }
    [ "$cl_align" -ge 3 ] || {
        echo "link: macho-sections: __objc_classlist is 2^$cl_align aligned, expected at least pointer alignment" >&2
        return 1
    }
    [ $((cl_off % 8)) -eq 0 ] || {
        echo "link: macho-sections: __objc_classlist lands at file offset $cl_off, not pointer-aligned" >&2
        return 1
    }
    marker_a=$(read_le_uint "$bin" "$cl_off" 8)
    marker_b=$(read_le_uint "$bin" $((cl_off + 8)) 8)
    [ "$marker_a" -eq 9734 ] && [ "$marker_b" -eq 9739 ] || {
        echo "link: macho-sections: __objc_classlist holds $marker_a,$marker_b; expected the probe-a then probe-b markers 9734,9739" >&2
        return 1
    }

    fields=$(macho_section_fields "$bin" __DATA __objc_imageinfo) || return 2
    set -- $fields
    ii_size=$2; ii_off=$3
    [ "$ii_size" -eq 8 ] || {
        echo "link: macho-sections: __objc_imageinfo is $ii_size bytes, expected 8" >&2
        return 1
    }
    ii_version=$(read_le_uint "$bin" "$ii_off" 4)
    ii_flags=$(read_le_uint "$bin" $((ii_off + 4)) 4)
    [ "$ii_version" -eq 0 ] && [ "$ii_flags" -eq 64 ] || {
        echo "link: macho-sections: __objc_imageinfo holds version=$ii_version flags=$ii_flags; libobjc validates these and the input set 0/64" >&2
        return 1
    }

    fields=$(macho_section_fields "$bin" __DATA __mod_init_func) || return 2
    set -- $fields
    mi_addr=$1; mi_size=$2; mi_align=$4; mi_flags=$5
    # the section TYPE is the low byte of the flags word, and dyld dispatches on it:
    # S_MOD_INIT_FUNC_POINTERS is 9 (#2637).
    [ $((mi_flags & 0xFF)) -eq 9 ] || {
        echo "link: macho-sections: __mod_init_func has section type $((mi_flags & 0xFF)), expected 9 (S_MOD_INIT_FUNC_POINTERS)" >&2
        return 1
    }
    [ "$mi_size" -eq 8 ] || {
        echo "link: macho-sections: __mod_init_func is $mi_size bytes, expected one 8-byte entry" >&2
        return 1
    }
    [ "$mi_align" -ge 3 ] || {
        echo "link: macho-sections: __mod_init_func is 2^$mi_align aligned, expected at least pointer alignment" >&2
        return 1
    }
    # the entry is an in-image pointer, so a PIE must slide it: without a rebase row
    # dyld would call whatever the unslid address happens to land on.
    mi_hex=$(printf '0x%X' "$mi_addr")
    rebases=$(macho_objdump --macho --rebase "$bin") || return 2
    [ "$(printf '%s\n' "$rebases" | grep -F -c "$mi_hex")" -eq 1 ] || {
        echo "link: macho-sections: __mod_init_func has no rebase row at $mi_hex" >&2
        return 1
    }

    echo "objc_classlist=concatenated-16-pointer-aligned"
    echo "objc_imageinfo=version0-flags64"
    echo "mod_init_func=present-rebased"
}

produce_macho_sections "$@"
