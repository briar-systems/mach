#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# riscv_pcrel_label_scan — read a `-d -r` object disassembly and report whether any
# `R_RISCV_PCREL_LO12_*` in it uses the psABI's label spelling.
#
# clang names the low half `%pcrel_lo(.Lpcrel_hiK)` - a label AT the paired auipc,
# LLVM's fixed prefix for it - where mach's own back end names the target on both
# halves. so the prefix is exactly the discriminator, and no cross-referencing of
# relocation sites is needed to tell the two spellings apart.
riscv_pcrel_label_scan() {
    awk '
    /R_RISCV_PCREL_LO12/ && /\.Lpcrel_hi/ { found = 1 }
    END { print "label-pairs=" (found ? "yes" : "no") }
    '
}

# riscv_pcrel_image_scan — read a linked-image disassembly and report two independent
# properties of every load reached through an `auipc`-materialized base.
#
#   misaligned      the resolved address is not a multiple of the access width. this
#                   is the one that CATCHES the #2797 defect: a low half resolved
#                   against the wrong pc lands an arbitrary byte count off its symbol,
#                   and the fixture's own repros report 8 (debug) and 4 (release)
#   text-relative   the resolved address lands inside the text range. a float or data
#                   constant is never in .text, so a correct link answers 0 - but so
#                   does the #2797 defect, which resolves to a wrong address in .data
#                   rather than onto code. it is kept as a true invariant that fails
#                   closed for a DIFFERENT mistake (a pair resolved onto instructions,
#                   which is what a bad relaxation or a wrong section base produces),
#                   and it is reported honestly as a second number rather than being
#                   described as the one doing the work
#
# a group is tracked per destination register and ends when that register is written
# again, which is the whole lifetime a hi/lo pair has. hex is parsed by hand rather
# than through gawk's strtonum: the runners' `awk` is mawk, which has no such
# function, and every other scan here is written to the same constraint.
riscv_pcrel_image_scan() {
    awk '
    function hex(s,   i, c, v, p) {
        sub(/^0x/, "", s)
        v = 0
        for (i = 1; i <= length(s); i++) {
            c = substr(s, i, 1)
            p = index("0123456789abcdef", tolower(c))
            if (p == 0) { return v }
            v = v * 16 + (p - 1)
        }
        return v
    }
    # a disassembled immediate, printed as an already-signed `-0x2bc` or `0x18`
    function signed(s) {
        if (substr(s, 1, 1) == "-") { return -hex(substr(s, 2)) }
        return hex(s)
    }
    /^[[:space:]]*[0-9a-f]+:/ {
        addr = hex(substr($1, 1, length($1) - 1))
        if (seen == 0 || addr < lo_addr) { lo_addr = addr }
        if (addr > hi_addr) { hi_addr = addr }
        seen = 1
        op = $2
        if (op == "auipc") {
            rd = $3; sub(/,$/, "", rd)
            base[rd] = addr + hex($4) * 4096
            live[rd] = 1
            next
        }
        # `addi a1, a1, 0x18` / `mv a1, a1` - the low half that finishes a base
        # POINTER. the pair materializes an address here rather than completing an
        # access, and every load off the result belongs to the same pair.
        if (op == "addi" || op == "mv") {
            rd = $3; sub(/,$/, "", rd)
            rs = $4; sub(/,$/, "", rs)
            if (live[rs] == 1) {
                d = (op == "mv") ? "0" : $5
                base[rd] = base[rs] + signed(d)
                live[rd] = 1
                next
            }
            if (rd in live) { live[rd] = 0 }
            next
        }
        # any load reached through such a base: the pair resolves to where it reads
        if (op == "fld" || op == "ld" || op == "flw" || op == "lw" || op == "lwu") {
            m = $4
            if (match(m, /\(.*\)$/)) {
                r = substr(m, RSTART + 1, RLENGTH - 2)
                d = substr(m, 1, RSTART - 1)
                if (live[r] == 1) {
                    a = base[r] + signed(d)
                    w = (op == "fld" || op == "ld") ? 8 : 4
                    np++
                    if (a % w != 0) { bad++ }
                    resolved[np] = a
                }
            }
            rd = $3; sub(/,$/, "", rd)
            if (rd in live) { live[rd] = 0 }
            next
        }
        # anything else that writes a register ends its group. the destination is the
        # first operand of every RV64 form that writes one; a store or a branch names
        # a source there instead, which only ends a group early and never extends one.
        rd = $3
        sub(/,$/, "", rd)
        if (rd in live) { live[rd] = 0 }
        next
    }
    END {
        intext = 0
        for (i = 1; i <= np; i++) {
            if (resolved[i] >= lo_addr && resolved[i] <= hi_addr) { intext++ }
        }
        print "pcrel-loads=" (np + 0 > 0 ? "yes" : "no")
        print "misaligned=" bad + 0
        print "text-relative=" intext
    }
    '
}

# produce_riscv_pcrel <engine> <leg> <binary>
# the riscv64 pc-relative HI/LO PAIR observable (#2797): the values a clang-built
# object loads through psABI hi/lo pairs, plus two facts about what was emitted.
#
# the values first, because the defect's signature is a believable zero: a pooled
# scale factor read from the wrong address is 0.0, and every product is then exactly
# 0. a case that only checked the exit status was green against it, which is how this
# survived as an unexplained CI-only failure of two other cases.
#
# then two emitted facts, each answering a way the value check could go quiet:
#
#   label-pairs=yes|no  whether the case's OWN C object still spells a low half the
#                       psABI way (`%pcrel_lo` naming a local label at the auipc,
#                       rather than naming the target the way mach's back end does).
#                       clang folds a `const` array into immediates and emits no pair
#                       at all; without this the fixture could stop containing the
#                       shape it exists to test and stay green forever
#   text-relative=<n>   over the WHOLE linked image: float loads reached through an
#                       `auipc`-materialized base whose resolved address lands inside
#                       the text range. a float constant is never in .text, so the
#                       answer is 0 for any correct link, and it is a property rather
#                       than an assertion about these three calls - it keeps meaning
#                       the same thing under whatever the next relaxation change does.
#                       the pre-fix linker resolved these pairs a few bytes off their
#                       own auipc, which is squarely inside .text
produce_riscv_pcrel() {
    engine=$1
    target=$2
    bin=$3
    out=$(mktemp)
    err=$(mktemp)
    run_captured "$engine" "$target" "$bin" "$out" "$err" || { rm -f "$out" "$err"; return 1; }
    if [ "$run_status" -ne 0 ]; then
        report_run_failure "riscv-pcrel" "$run_status" "$run_out"
        [ -s "$err" ] && sed 's/^/    /' "$err" >&2
        rm -f "$out" "$err"
        return "$run_status"
    fi
    cat "$err" >&2
    cat "$out"
    rm -f "$out" "$err"

    tool=$(resolve_objdump) || {
        echo "link: riscv-pcrel: llvm-objdump not found (install the 'llvm' package)" >&2
        return 2
    }
    dir=$(dirname "$(dirname "$(dirname "$bin")")")
    probe="$dir/out/link/build/obj/probe.o"
    if [ ! -f "$probe" ]; then
        echo "link: riscv-pcrel: no probe object at $probe" >&2; return 2
    fi
    "$tool" -d -r --no-show-raw-insn "$probe" | riscv_pcrel_label_scan
    # no --mattr override: the image now describes itself completely enough for the
    # disassembler to configure itself. it needed one twice over - e_flags did not
    # advertise the compressed extension the image contained (mach#2813), and the
    # `.riscv.attributes` arch string was derived from a per-ISA constant describing
    # what mach's own encoder emits rather than from what the linked image ended up
    # holding, which is what the compressed FLOAT loads (`c.fld`) read (mach#2828).
    # both are now derived from the image, so an override here would be hiding whether
    # they still are. without a correct answer from one of them those loads decode as
    # `<unknown>` and the property silently measures nothing
    "$tool" -d --no-show-raw-insn "$bin" | riscv_pcrel_image_scan
}

produce_riscv_pcrel "$@"
