#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_macho_signed <engine> <leg> <binary>
# Locate the checked-in clang fixture's initialized target bytes in __DATA, then
# decode the linked disp32 fields of its byte/word/dword immediate stores in
# __TEXT. The instruction-tail constants (7, 9, 10 bytes from instruction start)
# independently embody SIGNED_1/_2/_4's P+4+N convention. Exact equality with
# the three marker VAs proves more than a successful cross-link: every relocation
# selected the intended target byte.
produce_macho_signed() {
    bin=$3
    text_fields=$(macho_segment_fields "$bin" __TEXT) || return 2
    set -- $text_fields
    text_vm=$1; text_file=$3; text_size=$4
    data_fields=$(macho_segment_fields "$bin" __DATA) || return 2
    set -- $data_fields
    data_vm=$1; data_file=$3; data_size=$4

    marker_file=$(od -An -v -tu1 -j "$data_file" -N "$data_size" "$bin" | awk -v base="$data_file" '
        { for (i = 1; i <= NF; i++) b[++n] = $i }
        END {
            found = 0
            for (i = 1; i + 7 <= n; i++) {
                if (b[i] == 165 && b[i+1] == 0 && b[i+2] == 239 && b[i+3] == 190 &&
                    b[i+4] == 190 && b[i+5] == 186 && b[i+6] == 254 && b[i+7] == 202) {
                    found++; pos = base + i - 1
                }
            }
            if (found != 1) {
                print "link: macho-signed: expected one initialized target marker, found " found > "/dev/stderr"
                exit 2
            }
            printf "%.0f\n", pos
        }
    ') || return 2
    target1=$((data_vm + marker_file - data_file))
    target2=$((target1 + 2))
    target4=$((target1 + 4))

    stores=$(od -An -v -tu1 -j "$text_file" -N "$text_size" "$bin" | awk -v vm="$text_vm" '
        function disp32(i, u) {
            u = b[i] + 256*b[i+1] + 65536*b[i+2] + 16777216*b[i+3]
            return u >= 2147483648 ? u - 4294967296 : u
        }
        { for (i = 1; i <= NF; i++) b[++n] = $i }
        END {
            n1 = n2 = n4 = 0
            for (i = 1; i <= n; i++) {
                insn = vm + i - 1
                if (i + 6 <= n && b[i] == 198 && b[i+1] == 5 && b[i+6] == 1) {
                    n1++; v1 = insn + 7 + disp32(i + 2)
                }
                if (i + 8 <= n && b[i] == 102 && b[i+1] == 199 && b[i+2] == 5 && b[i+7] == 52 && b[i+8] == 18) {
                    n2++; v2 = insn + 9 + disp32(i + 3)
                }
                if (i + 9 <= n && b[i] == 199 && b[i+1] == 5 && b[i+6] == 120 && b[i+7] == 86 && b[i+8] == 52 && b[i+9] == 18) {
                    n4++; v4 = insn + 10 + disp32(i + 2)
                }
            }
            if (n1 != 1 || n2 != 1 || n4 != 1) {
                print "link: macho-signed: expected one store of each width, found " n1 "/" n2 "/" n4 > "/dev/stderr"
                exit 2
            }
            printf "SIGNED_1 %.0f SIGNED_2 %.0f SIGNED_4 %.0f\n", v1, v2, v4
        }
    ') || return 2
    set -- $stores
    [ "$1" = SIGNED_1 ] && [ "$3" = SIGNED_2 ] && [ "$5" = SIGNED_4 ] || return 2

    [ "$2" -eq "$target1" ] || { echo "link: macho-signed: SIGNED_1 resolved to $2, expected $target1" >&2; return 1; }
    [ "$4" -eq "$target2" ] || { echo "link: macho-signed: SIGNED_2 resolved to $4, expected $target2" >&2; return 1; }
    [ "$6" -eq "$target4" ] || { echo "link: macho-signed: SIGNED_4 resolved to $6, expected $target4" >&2; return 1; }
    echo "SIGNED_1=exact"
    echo "SIGNED_2=exact"
    echo "SIGNED_4=exact"
}

produce_macho_signed "$@"
