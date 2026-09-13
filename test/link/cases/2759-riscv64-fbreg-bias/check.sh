#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_varloc_fbreg <engine> <leg> <nog_binary> <g_binary>
# the FRAME-SLOT VARIABLE LOCATION observable (#2759): does a `DW_OP_fbreg` offset name
# an address the emitted code actually uses for that slot.
#
# the offset alone is not evidence. the producer and the encoder each turn one layout
# fact - the bytes the prologue reserves between the frame pointer and the slot region -
# into an address, and a producer that derives it a second way is perfectly
# self-consistent while pointing at the wrong bytes. it agreed with the encoder on
# x86-64 and aarch64, whose reservation is 0, and was wrong by 16 to 200 bytes on every
# riscv64 function, which is why nothing saw it. so the observable crosses the two:
# for each subprogram, every `DW_OP_fbreg` offset must appear as a displacement in some
# instruction of that same function that names the register `DW_AT_frame_base` names.
#
# the two counts are ISA-independent by construction, so the golden is shared:
#   checked=<n>   offsets crossed. zero would make `unbacked` vacuous, so it is stated
#   unbacked=<n>  offsets no emitted access backs. the invariant is 0 on every target
#
# immediate displacements are read from instructions naming the frame-base register.
# arm64 large displacements are reconstructed from constant register definitions and
# actual indexed accesses, with state cleared at control-flow joins and unknown effects.
# the register tracking runs in bash because its arithmetic is 64-bit: a movn/movk
# construction of a negative offset does not survive awk's doubles.
produce_varloc_fbreg() {
    g=$4
    dd_tool=$(resolve_dwarfdump) || {
        echo "link: varloc-fbreg: llvm-dwarfdump not found (install the 'llvm' package)" >&2; return 2
    }
    od_tool=$(resolve_objdump) || {
        echo "link: varloc-fbreg: llvm-objdump not found (install the 'llvm' package)" >&2; return 2
    }

    # one record per subprogram that has a machine range and at least one fbreg
    # variable: "lo hi framebase-register off,off,..."
    "$dd_tool" --debug-info "$g" 2>/dev/null | awk '
        function flush(   i) {
            if (lo != "" && hi != "" && fb != "" && offs != "") { print lo, hi, fb, offs }
            lo = ""; hi = ""; fb = ""; offs = ""
        }
        /DW_TAG_subprogram/ { flush(); next }
        # only the subprogram own range: an inlined-subroutine or lexical-block DIE
        # nested inside it carries a low_pc too, and it is printed after the frame base.
        /DW_AT_low_pc/  { if (fb == "" && match($0, /0x[0-9a-f]+/)) { lo = substr($0, RSTART, RLENGTH) } next }
        /DW_AT_high_pc/ { if (fb == "" && match($0, /0x[0-9a-f]+/)) { hi = substr($0, RSTART, RLENGTH) } next }
        # the DWARF register NUMBER, not the name llvm-dwarfdump prints beside it: it
        # spells the aarch64 frame pointer W29 while the disassembler spells it x29.
        /DW_AT_frame_base/ { if (match($0, /DW_OP_reg[0-9]+/)) { fb = substr($0, RSTART + 9, RLENGTH - 9) } next }
        /DW_OP_fbreg/ {
            s = $0
            while (match(s, /DW_OP_fbreg [+-]?[0-9]+/)) {
                o = substr(s, RSTART, RLENGTH); sub(/^DW_OP_fbreg /, "", o); sub(/^\+/, "", o)
                offs = (offs == "") ? o : offs "," o
                s = substr(s, RSTART + RLENGTH)
            }
            next
        }
        END { flush() }
    ' > "$g.fns" || return 1

    "$od_tool" -d --no-show-raw-insn "$g" 2>/dev/null > "$g.dis" || return 1

    case "$2" in
        *riscv64*) fb_isa=riscv64 ;;
        *arm64*|*aarch64*) fb_isa=aarch64 ;;
        *) fb_isa=x86_64 ;;
    esac

    fbreg_cross "$g.fns" "$g.dis" "$fb_isa"
    rc=$?
    rm -f "$g.fns" "$g.dis"
    return $rc
}

# gp <operand>: "number width" for a general-purpose register operand xN / wN
gp() {
    [[ $1 =~ ^([xw])([0-9]+)$ ]] || return 1
    [ "${BASH_REMATCH[2]}" -lt 31 ] || return 1
    echo "${BASH_REMATCH[2]} $([ "${BASH_REMATCH[1]}" = x ] && echo 64 || echo 32)"
}

# constant <opcode> <operands...>: track a mov/movz/movn/movk into `known`
constant() {
    op=$1; shift
    d=$(gp "${1:-}") || return 0
    number=${d% *}; width=${d#* }
    if [ "$width" -eq 64 ]; then mask=-1; else mask=4294967295; fi
    previous=${known[$number]:-}
    unset "known[$number]"
    [ $# -ge 2 ] || return 0
    if src=$(gp "$2") && [ "$op" = mov ]; then
        snum=${src% *}; swidth=${src#* }
        if [ "$snum" = "$number" ]; then value=$previous; else value=${known[$snum]:-}; fi
        [ -n "$value" ] || return 0
        if [ "$swidth" -eq 64 ]; then smask=-1; else smask=4294967295; fi
        known[$number]=$(( value & smask & mask ))
        return 0
    fi
    imm=${2#\#}
    [[ $imm =~ ^-?(0x[0-9a-fA-F]+|[0-9]+)$ ]] || return 0
    value=$(( imm ))
    shift_by=0
    if [ $# -eq 3 ]; then
        [[ $3 =~ ^lsl\ #([0-9]+)$ ]] || return 0
        shift_by=${BASH_REMATCH[1]}
    fi
    [ $((shift_by % 16)) -eq 0 ] && [ "$shift_by" -lt "$width" ] || return 0
    if [ "$op" = movk ]; then
        [ -n "$previous" ] || return 0
        value=$(( (previous & ~(65535 << shift_by)) | ((value & 65535) << shift_by) ))
    else
        value=$(( value << shift_by ))
        [ "$op" = movn ] && value=$(( ~value ))
    fi
    known[$number]=$(( value & mask ))
}

# fbreg_cross <ranges> <disassembly> <isa>: cross every fbreg offset of each
# subprogram against the displacements its own instructions use from the frame base
fbreg_cross() {
    ranges=$1; dis=$2; isa=$3
    case "$isa" in
        riscv64) reg8=s0; reg2=sp ;;
        aarch64) reg29=x29; reg31=sp ;;
        *) reg6=%rbp; reg7=%rsp ;;
    esac
    ldst='^(ld|st)(r|ur|p)[a-z]*$'
    declare -A targets known seen
    if [ "$isa" = aarch64 ]; then
        while read -r t; do targets[$t]=1; done < <(awk '
            /^[[:space:]]*[0-9a-f]+:[[:space:]]/ {
                op = $2
                if (op == "b" || op ~ /^b\./ || op == "cbz" || op == "cbnz" || op == "tbz" || op == "tbnz") {
                    n = split($0, f, ","); t = f[n]; sub(/^[[:space:]]+/, "", t); sub(/[[:space:]].*$/, "", t)
                    if (t ~ /^0x[0-9a-f]+$/) print hex(t)
                }
            }
            function hex(s,   i, c, v, p) {
                sub(/^0x/, "", s); v = 0
                for (i = 1; i <= length(s); i++) { p = index("0123456789abcdef", tolower(substr(s, i, 1))); if (p == 0) return v; v = v * 16 + (p - 1) }
                return v
            }' "$dis")
    fi
    checked=0; unbacked=0
    # one stream: "R <register> <offsets>" opens a subprogram, "I <opcode> <operands>"
    # is an instruction inside it, "E" closes it. the range assignment is awk's work.
    while IFS='|' read -r kind a b addr; do
        case "$kind" in
            R)  eval "base=\${reg$a:-}"
                known=(); seen=()
                offs=$b ;;
            I)  [ -n "$base" ] || continue
                [ -n "${targets[$addr]:-}" ] && known=()
                op=$a; operands=$b
                line="$op $operands"
                if [[ $line =~ (^|[^A-Za-z0-9_])$base([^A-Za-z0-9_]|$) ]]; then
                    rest=$line
                    while [[ $rest =~ (-?0x[0-9a-f]+)(.*) ]]; do
                        seen[$(( ${BASH_REMATCH[1]} ))]=1; rest=${BASH_REMATCH[2]}
                    done
                    if [ "$isa" = aarch64 ] && [[ $op =~ $ldst ]] && [[ $operands =~ \[$base,\ *(x[0-9]+)\] ]]; then
                        if r=$(gp "${BASH_REMATCH[1]}"); then
                            v=${known[${r% *}]:-}
                            [ -n "$v" ] && seen[$v]=1
                        fi
                    fi
                fi
                [ "$isa" = aarch64 ] || continue
                IFS=',' read -r -a parts <<<"$operands"
                for i in "${!parts[@]}"; do parts[$i]=$(echo "${parts[$i]}" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//'); done
                case "$op" in
                    mov|movz|movn|movk) constant "$op" "${parts[@]}" ;;
                    cmp|cmn|tst|ccmp|ccmn|nop) ;;
                    *)
                        if [[ $op =~ $ldst ]]; then
                            if [[ $op == ld* ]]; then
                                n=1; [[ $op == ldp* ]] && n=2
                                for ((i = 0; i < n && i < ${#parts[@]}; i++)); do
                                    if d=$(gp "${parts[$i]}"); then unset "known[${d% *}]"; fi
                                done
                            fi
                            if [[ $operands == *']!'* ]] || [[ $operands == *'],'* ]]; then
                                if [[ $operands =~ \[(x[0-9]+) ]] && d=$(gp "${BASH_REMATCH[1]}"); then unset "known[${d% *}]"; fi
                            fi
                        else
                            known=()
                        fi ;;
                esac ;;
            E)  [ -n "$base" ] || continue
                IFS=',' read -r -a offs_a <<<"$offs"
                for o in "${offs_a[@]}"; do
                    checked=$((checked + 1))
                    [ -n "${seen[$((o))]:-}" ] || unbacked=$((unbacked + 1))
                done ;;
        esac
    done < <(awk -v ranges="$ranges" -v targets_isa="$isa" '
        function hex(s,   i, c, v, p) {
            sub(/^0x/, "", s); v = 0
            for (i = 1; i <= length(s); i++) { p = index("0123456789abcdef", tolower(substr(s, i, 1))); if (p == 0) return v; v = v * 16 + (p - 1) }
            return v
        }
        BEGIN {
            while ((getline l < ranges) > 0) { split(l, f, " "); n++; lo[n] = hex(f[1]); hi[n] = hex(f[2]); fb[n] = f[3]; offs[n] = f[4] }
            cur = 0
        }
        /^[[:space:]]*[0-9a-f]+:[[:space:]]/ {
            addr = hex(substr($1, 1, length($1) - 1))
            if (cur && addr >= hi[cur]) { print "E||"; cur = 0 }
            if (!cur) { for (i = 1; i <= n; i++) if (addr >= lo[i] && addr < hi[i]) { cur = i; print "R|" fb[i] "|" offs[i]; break } }
            if (!cur) next
            line = $0; sub(/^[[:space:]]*[0-9a-f]+:[[:space:]]*/, "", line); sub(/[[:space:]]*\/\/.*$/, "", line)
            op = line; sub(/[[:space:]].*$/, "", op)
            ops = line; sub(/^[^[:space:]]*[[:space:]]*/, "", ops)
            if (op == line) ops = ""
            print "I|" op "|" ops "|" addr
        }
        END { if (cur) print "E||" }' "$dis")
    echo "varloc_fbreg_checked=$([ "$checked" -gt 0 ] && echo nonzero || echo zero)"
    echo "varloc_fbreg_unbacked=$unbacked"
}

produce_varloc_fbreg "$@"
