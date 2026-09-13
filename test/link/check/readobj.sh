#!/usr/bin/env bash
. "$(dirname "$0")/common.sh"

# _uuid_shape <hex32> — the first sixteen bytes of a digest with the rfc 4122
# version 4 and variant 1 bits forced, as the writers derive a 16-byte id.
_uuid_shape() {
    local h b6 b8
    h=${1:0:32}
    b6=$(( 0x${h:12:2} & 0x0F | 0x40 ))
    b8=$(( 0x${h:16:2} & 0x3F | 0x80 ))
    printf '%s%02x%s%02x%s' "${h:0:12}" "$b6" "${h:14:2}" "$b8" "${h:18:14}"
}

# _sha256_of_stream — the hex digest of stdin
_sha256_of_stream() {
    sha256sum | cut -d' ' -f1
}

# _elf_build_id_expected <binary> — the ELF build id recomputed from the file: the
# sha-256 of every PT_LOAD file extent in table order, with the header's
# section-table fields and the note descriptor as zero. handles both classes.
_elf_build_id_expected() {
    local bin class n note_off
    bin=$1
    class=$(readelf -hW "$bin" 2>/dev/null | awk '/Class:/{print $2}')
    n=$(mktemp)
    cp "$bin" "$n"
    if [ "$class" = "ELF64" ]; then
        printf '\0\0\0\0\0\0\0\0' | dd of="$n" bs=1 seek=40 count=8 conv=notrunc status=none
        printf '\0\0\0\0'             | dd of="$n" bs=1 seek=60 count=4 conv=notrunc status=none
    else
        printf '\0\0\0\0' | dd of="$n" bs=1 seek=32 count=4 conv=notrunc status=none
        printf '\0\0\0\0' | dd of="$n" bs=1 seek=48 count=4 conv=notrunc status=none
    fi
    note_off=$(readelf -lW "$bin" 2>/dev/null | awk '/^ *NOTE/{print strtonum($2); exit}')
    [ -n "$note_off" ] || { rm -f "$n"; return 1; }
    head -c 32 /dev/zero | dd of="$n" bs=1 seek=$((note_off + 16)) count=32 conv=notrunc status=none
    readelf -lW "$bin" 2>/dev/null | awk '/^ *LOAD/{print strtonum($2), strtonum($5)}' | while read -r off fsz; do
        [ "$fsz" -eq 0 ] && continue
        dd if="$n" bs=1M iflag=skip_bytes,count_bytes skip="$off" count="$fsz" status=none
    done | _sha256_of_stream
    rm -f "$n"
}

# _zeroed_prefix_sha256 <file> <hole_off> <hole_len> <limit> — the sha-256 of the
# first <limit> bytes of <file> with <hole_len> bytes at <hole_off> as zero
_zeroed_prefix_sha256() {
    {
        head -c "$2" "$1"
        head -c "$3" /dev/zero
        tail -c +$(( $2 + $3 + 1 )) "$1" | head -c $(( $4 - $2 - $3 ))
    } | _sha256_of_stream
}

# produce_build_id <readobj> <binary> — one line naming the id's presence, its
# length and whether it is the content hash the design states
produce_build_id() {
    local tool bin fmt got want guid a b c d e elfanew oh dir_rva nsec st s sh vs va rp entry raw size
    local dd_tool ncmds off i uuid_off sig_off cmd cmdsize agree
    tool=$1
    bin=$2
    fmt=$("$tool" --file-header "$bin" 2>/dev/null | awk '/^Format:/{print $2; exit}')
    got=
    want=
    case "$fmt" in
        elf*)
            got=$("$tool" --notes "$bin" 2>/dev/null | sed -n 's/^ *Build ID: //p' | head -1)
            want=$(_elf_build_id_expected "$bin")
            ;;
        COFF*)
            guid=$("$tool" --coff-debug-directory "$bin" 2>/dev/null | sed -n 's/^ *PDBGUID: {\(.*\)}$/\1/p' | head -1)
            if [ -n "$guid" ]; then
                # the printed guid spells its first three fields little-endian
                a=${guid:0:8}; b=${guid:9:4}; c=${guid:14:4}; d=${guid:19:4}; e=${guid:24:12}
                got="${a:6:2}${a:4:2}${a:2:2}${a:0:2}${b:2:2}${b:0:2}${c:2:2}${c:0:2}${d}${e}"
                got=$(printf '%s' "$got" | tr 'A-F' 'a-f')
                elfanew=$(read_le_uint "$bin" 60 4)
                oh=$((elfanew + 24))
                dir_rva=$(read_le_uint "$bin" $((oh + 112 + 6 * 8)) 4)
                nsec=$(read_le_uint "$bin" $((elfanew + 6)) 2)
                st=$((oh + 240))
                s=0
                raw=
                while [ "$s" -lt "$nsec" ]; do
                    sh=$((st + s * 40))
                    vs=$(read_le_uint "$bin" $((sh + 8)) 4); va=$(read_le_uint "$bin" $((sh + 12)) 4)
                    rp=$(read_le_uint "$bin" $((sh + 20)) 4)
                    if [ "$dir_rva" -ge "$va" ] && [ "$dir_rva" -lt $((va + vs)) ]; then
                        entry=$((rp + dir_rva - va))
                        raw=$(read_le_uint "$bin" $((entry + 24)) 4)
                        break
                    fi
                    s=$((s + 1))
                done
                if [ -n "$raw" ]; then
                    size=$(stat -c %s "$bin")
                    want=$(_uuid_shape "$(_zeroed_prefix_sha256 "$bin" $((raw + 4)) 16 "$size")")
                fi
            fi
            ;;
        Mach-O*)
            # llvm-readobj prints no LC_UUID; llvm-dwarfdump (pinned beside it) does
            dd_tool=$(resolve_dwarfdump) || { echo "link: readobj: llvm-dwarfdump is required for the Mach-O uuid" >&2; return 2; }
            got=$("$dd_tool" --uuid "$bin" 2>/dev/null | sed -n 's/^UUID: \([0-9A-Fa-f-]*\) .*/\1/p' | head -1 | tr -d '-' | tr 'A-F' 'a-f')
            ncmds=$(read_le_uint "$bin" 16 4)
            off=32; i=0; uuid_off=; sig_off=
            while [ "$i" -lt "$ncmds" ]; do
                cmd=$(read_le_uint "$bin" "$off" 4)
                cmdsize=$(read_le_uint "$bin" $((off + 4)) 4)
                [ "$cmd" = 27 ] && uuid_off=$((off + 8))
                [ "$cmd" = 29 ] && sig_off=$(read_le_uint "$bin" $((off + 8)) 4)
                [ "$cmdsize" -ge 8 ] || break
                off=$((off + cmdsize)); i=$((i + 1))
            done
            if [ -n "$uuid_off" ] && [ -n "$sig_off" ]; then
                want=$(_uuid_shape "$(_zeroed_prefix_sha256 "$bin" "$uuid_off" 16 "$sig_off")")
            fi
            ;;
    esac
    if [ -z "$got" ]; then
        echo "build-id absent"
        return 0
    fi
    if [ -n "$want" ] && [ "$got" = "$want" ]; then agree=yes; else agree=no; fi
    echo "build-id len=$(( ${#got} / 2 )) content-derived=$agree"
}

# produce_readobj <engine> <leg> <binary>
# an independent decoder over everything the writer published for the case: the
# linked image and every relocatable object under the case's out/ tree are each
# parsed by llvm-readobj (--all: headers, sections, segments, symbols, relocations
# and every format-specific table), and any diagnostic the reader prints is a
# failure. the writer's own tiling checks prove that its plan and its bytes agree
# with each other; this proves the bytes are the format (#3113). the golden then
# names the image's sections and their alignments, which is what the file plan
# decides, without offsets or sizes so that a dependency bump does not rebless it.
produce_readobj() {
    bin=$3
    profile=$5
    tool=$(resolve_readobj) || {
        echo "link: readobj: llvm-readobj is required" >&2; return 2
    }
    casedir=$(dirname "$(dirname "$(dirname "$bin")")")
    files=$(mktemp)
    printf '%s\n' "$bin" >"$files"
    # this profile's objects only: the other profile's tree may still be present
    find "$casedir"/out/*/"$profile" -type f \( -name '*.o' -o -name '*.obj' \) 2>/dev/null | LC_ALL=C sort >>"$files"
    n=0
    failed=0
    while IFS= read -r f; do
        n=$((n + 1))
        rc=0
        errs=$("$tool" --all "$f" 2>&1 >/dev/null) || rc=$?
        if [ "$rc" -ne 0 ] || [ -n "$errs" ]; then
            failed=$((failed + 1))
            echo "readobj: FAIL ${f#"$casedir"/}: $(printf '%s' "$errs" | head -1)"
        fi
    done <"$files"
    rm -f "$files"
    # the object count follows the dependency, so it is reported, not recorded
    echo "readobj: objects $n failed $failed" >&2
    if [ "$failed" -ne 0 ]; then return 2; fi
    if [ "$n" -lt 2 ]; then echo "readobj: no relocatable object was found beside the image" >&2; return 2; fi
    echo "readobj: image and objects parse"

    # the image's sections as the reader names them: one row per section with its
    # alignment, and for a mapped image its segments' kinds. ELF, COFF and Mach-O
    # each print these under different keys, so the awk keys on all three.
    "$tool" --sections "$bin" 2>/dev/null | awk '
        /^ *Name: /             { name = $2; sub(/ \(.*/, "", name) }
        /^ *Segment: /          { seg = $2 }
        /^ *AddressAlignment: / { print "section " name " align=" $2 }
        /^ *Alignment: /        { print "section " seg "," name " align=" $2 }
        /^ *Characteristics \[/ { print "section " name }
    '
    "$tool" --segments "$bin" 2>/dev/null | awk '
        /^ *Type: /  { t = $2 }
        /^ *Flags \[/ { print "segment " t }
    '
    "$tool" --macho-segment "$bin" 2>/dev/null | awk '
        /^ *Name: /  { print "segment " $2 }
    '

    # the build id every linked image carries (#3221), read by an external tool and
    # checked against an independent recomputation over the file, which is the
    # content-derived property: the value itself is not recorded because it
    # changes with every dependency bump, the length and the agreement do not.
    produce_build_id "$tool" "$bin"
    return 0
}

produce_readobj "$@"
