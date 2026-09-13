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
    # the fixture's functions as defined (T) symbols: `main` (an explicit
    # `#[symbol("main")]`, unmangled) and `burn` (mangled - matched by substring,
    # not exact name, since the mangling scheme is not this case's concern).
    nm_out=$(nm "$b" 2>/dev/null)
    if printf '%s\n' "$nm_out" | grep -qE ' T main$'; then
        echo "nm_main=defined"
    else
        echo "nm_main=missing"
    fi
    if printf '%s\n' "$nm_out" | grep -qE ' T .*burn'; then
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

produce_symtab "$@"
