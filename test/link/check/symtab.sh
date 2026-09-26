#!/usr/bin/env bash
. "$(dirname "$0")/common.sh"

# produce_symtab <engine> <leg> <binary>
# the ELF `.symtab` observable (#2772): a PLAIN build (no `-g`, no special flag -
# the shape a shipped release binary actually has) now carries a real function
# symbol table, which test/link/cases/debuginfo cannot speak to at all (DWARF is a
# `-g`-only concern). requires nm, readelf, and addr2line; a missing tool is a
# hard error, the same contract produce_debuginfo already uses for its own
# validators.
produce_symtab() {
    b=$3
    command -v nm >/dev/null 2>&1 || {
        echo "link: symtab: nm not found (install the 'binutils' package)" >&2; return 2
    }
    command -v readelf >/dev/null 2>&1 || {
        echo "link: symtab: readelf not found (install the 'binutils' package)" >&2; return 2
    }
    command -v addr2line >/dev/null 2>&1 || {
        echo "link: symtab: addr2line not found (install the 'binutils' package)" >&2; return 2
    }

    sh_out=$(readelf -SW "$b" 2>/dev/null)
    if printf '%s\n' "$sh_out" | grep -qE '\.symtab +SYMTAB'; then
        echo "symtab_present=yes"
    else
        echo "symtab_present=no"
    fi
    if printf '%s\n' "$sh_out" | grep -qE '\.strtab +STRTAB'; then
        echo "strtab_present=yes"
    else
        echo "strtab_present=no"
    fi

    # `nm` (the standard "does this binary have symbols at all" tool) finds both
    # the fixture's functions as defined symbols: `main` (an explicit
    # `#[symbol("main")]`, unmangled) and `burn` (mangled - matched by substring,
    # not exact name, since the mangling scheme is not this case's concern).
    #
    # either binding counts (T or t): #3412 made a definition that no image
    # exports hidden, and a hidden definition binds locally in the image it lands
    # in, so a non-`pub` function is `t` here. that this case asserts nothing
    # about binding is the point - the fact under test is that the table exists
    # and its entries are usable, which holds for both.
    nm_out=$(nm "$b" 2>/dev/null)
    if printf '%s\n' "$nm_out" | grep -qE ' [Tt] main$'; then
        echo "nm_main=defined"
    else
        echo "nm_main=missing"
    fi
    if printf '%s\n' "$nm_out" | grep -qE ' [Tt] .*burn'; then
        echo "nm_burn=defined"
    else
        echo "nm_burn=missing"
    fi

    # mid-function resolution: the address at burn's st_value PLUS HALF of its
    # st_size must still resolve to burn - the check `st_size` is right, not just
    # present, and the one most likely to be skipped (a symbol table with entries
    # and no real sizes looks fine under `nm` and only fails a profiler later).
    # readelf -sW dumps EVERY symbol table in the file, and a shared object's
    # `.dynsym` (#2807) carries the same global function with `st_size` always 0
    # (that table has never had a size writer - out of this case's scope) ahead
    # of `.symtab` in the listing; matching the first hit anywhere would silently
    # grab the wrong table's zero-size entry and read as "no symbol" rather than
    # the real fact under test, so the scan is confined to the `.symtab` table by
    # its own "Symbol table '.symtab'" banner line.
    burn_line=$(readelf -sW "$b" 2>/dev/null | awk -v want="'.symtab'" '
        /^Symbol table / { insym = ($0 ~ want); next }
        insym && /burn/ && / FUNC / { print; exit }
    ')
    burn_val=$(printf '%s\n' "$burn_line" | awk '{ print $2 }')
    burn_size=$(printf '%s\n' "$burn_line" | awk '{ print $3 }')
    if [ -n "$burn_val" ] && [ -n "$burn_size" ] && [ "$burn_size" -gt 0 ]; then
        mid=$(( 0x$burn_val + burn_size / 2 ))
        resolved=$(addr2line -f -e "$b" "$(printf '0x%x' "$mid")" 2>/dev/null | sed -n '1p')
        case "$resolved" in
            *burn*) echo "midfunc_resolve=burn" ;;
            *)      echo "midfunc_resolve=other" ;;
        esac
    else
        echo "midfunc_resolve=no-symbol"
    fi

    # byte-additivity (the property test/link/cases/debuginfo's elf_seg_identical
    # proves for DWARF, here read directly off the load segments rather than
    # from a before/after diff, since this symbol table is unconditional - there
    # is no "before" build to compare against): every PT_LOAD's file extent must
    # end at or before .symtab's file offset, so a loader - which reads only
    # PT_LOAD - never sees a byte the symbol table touched.
    # strip the section index before splitting fields, since [ 9] and [10] differ.
    symtab_off_hex=$(printf '%s\n' "$sh_out" | awk '
        /^[[:space:]]*\[[[:space:]]*[0-9]+\]/ {
            sub(/^[[:space:]]*\[[[:space:]]*[0-9]+\][[:space:]]*/, "")
            if ($1 == ".symtab" && $2 == "SYMTAB") { print $4; exit }
        }
    ')
    last_load_end=0
    while read -r typ off _va _pa filesz _memsz _flg _align; do
        [ "$typ" = "LOAD" ] || continue
        seg_end=$(( off + filesz ))
        if [ "$seg_end" -gt "$last_load_end" ]; then last_load_end=$seg_end; fi
    done <<PHDRS
$(readelf -lW "$b" 2>/dev/null | awk '/^  LOAD/ { print }')
PHDRS
    if [ -n "$symtab_off_hex" ] && [ "$last_load_end" -le "$(( 0x$symtab_off_hex ))" ]; then
        echo "symtab_after_loadable=yes"
    else
        echo "symtab_after_loadable=no"
    fi
}

# the unwind observable (#4013): the image maps a PT_GNU_EH_FRAME search table,
# every function mach compiled has a frame description starting at it, and the
# rules read right. only `_start` goes without one: a frameless function whose
# body is inline assembly states no frame the compiler can vouch for. the rules are
# read through llvm-dwarfdump and named by role, so the facts hold on every isa:
# past its prologue `main` finds its frame from its frame pointer with the return
# address in the slot below it, and the frameless leaf `burn` keeps the frame its
# call left
produce_unwind() {
    b=$1
    leg=$2
    command -v llvm-dwarfdump >/dev/null 2>&1 || {
        echo "link: symtab: llvm-dwarfdump not found (install the 'llvm' package)" >&2; return 2
    }
    if readelf -lW "$b" 2>/dev/null | grep -q 'GNU_EH_FRAME' \
        && readelf -SW "$b" 2>/dev/null | grep -qE '\.eh_frame_hdr +PROGBITS'; then
        echo "eh_frame_hdr=yes"
    else
        echo "eh_frame_hdr=no"
    fi

    frames=$(llvm-dwarfdump --eh-frame "$b" 2>/dev/null)
    # each fde as `<start> <last row>`: the rule that holds once the prologue ran
    fdes=$(printf '%s\n' "$frames" | awk '
        / FDE / { if (start != "") print start, last; split($0, a, "pc="); split(a[2], r, "\\.\\.\\."); start = r[1]; last = ""; next }
        /^  0x/ && start != "" { sub(/^  0x[0-9a-f]+: /, ""); last = $0 }
        END { if (start != "") print start, last }
    ' | awk '{ s = $1; sub(/^0+/, "", s); $1 = s; print }')

    syms=$(readelf -sW "$b" 2>/dev/null | awk -v want="'.symtab'" '
        /^Symbol table / { insym = ($0 ~ want); next }
        insym && / FUNC / { v = $2; sub(/^0+/, "", v); print v, $8 }
    ')
    # std's own inline-assembly leaves differ by isa and profile, so the listing
    # names the program's functions and the entry
    missing=$(printf '%s\n' "$syms" | while read -r v n; do
        [ -n "$v" ] || continue
        case "$n" in std.*) continue ;; esac
        printf '%s\n' "$fdes" | grep -q "^$v " || echo "$n"
    done | sort -u | paste -sd, -)
    echo "fde_missing=${missing:-none}"

    role() {
        case "$1" in
            RBP|W29|X29|FP|X8|S0) echo fp ;;
            RSP|WSP|SP|X2)        echo sp ;;
            *)                    echo "$1" ;;
        esac
    }
    # `CFA=<reg>[+<off>]` and the return address rule of one function's fde
    cfa_of() {
        row=$(printf '%s\n' "$fdes" | awk -v v="$1" '$1 == v { $1 = ""; print; exit }')
        cfa=$(printf '%s\n' "$row" | sed -n 's/.*CFA=\([A-Z0-9]*\)\(+[0-9]*\)\{0,1\}.*/\1 \2/p')
        reg=${cfa%% *}
        off=${cfa#* }
        [ -n "$reg" ] || { echo "none"; return; }
        echo "$(role "$reg")${off:-+0}"
    }
    # the rule's register alone: a frame pointer sits at a different distance
    # from the frame on every isa
    cfa_role_of() {
        c=$(cfa_of "$1")
        echo "${c%%+*}"
    }
    ra_of() {
        row=$(printf '%s\n' "$fdes" | awk -v v="$1" '$1 == v { $1 = ""; print; exit }')
        case "$row" in
            *RIP=\[CFA-8\]*|*W30=\[CFA-8\]*|*X30=\[CFA-8\]*|*LR=\[CFA-8\]*|*X1=\[CFA-8\]*|*RA=\[CFA-8\]*) echo "cfa-8" ;;
            *) echo "unsaved" ;;
        esac
    }
    sym_addr() {
        printf '%s\n' "$syms" | awk -v n="$1" '$2 == n { print $1; exit }'
    }
    main_v=$(sym_addr main)
    burn_v=$(printf '%s\n' "$syms" | awk '$2 ~ /burn/ { print $1; exit }')
    echo "fde_main_cfa=$(cfa_role_of "$main_v")"
    echo "fde_main_ra=$(ra_of "$main_v")"
    entry=sp+0
    case "$leg" in x86_64-*) entry=sp+8 ;; esac
    burn_cfa=$(cfa_of "$burn_v")
    if [ "$burn_cfa" = "$entry" ]; then
        echo "fde_burn_cfa=entry"
    else
        echo "fde_burn_cfa=$burn_cfa"
    fi
}

produce_symtab "$@"
# the unwind tables are an executable's, which the entry `_start` marks; a
# shared object carries none
if nm "$3" 2>/dev/null | grep -qE ' [Tt] _start$'; then
    produce_unwind "$3" "$2"
fi
