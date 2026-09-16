#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_prologue_lines <engine> <leg> <nog_binary> <g_binary> <profile>
# the binary-inspection producer for #3394. every fact is read from the `-g` artifact
# with llvm-dwarfdump, the same decode a symbolizer performs:
#
#   main_entry               what the function's entry address resolves to. this is the
#                            row covering the prologue - the frame setup, the callee
#                            saves - and it must name this fixture's `fun main` line.
#                            before the fix it named the first body instruction's
#                            location, an inlined std file at -O2.
#   main_decl                DW_AT_decl_file/decl_line of the subprogram, which had the
#                            same origin and so the same defect.
#   prologue_end_after_entry that the prologue is a region: the row flagged
#                            prologue_end sits at a later address than the entry.
#   body_starts              whether the first row after the prologue is in this file
#                            or another one. at -O2 it must be another one, or the
#                            case proved nothing about inlining; at -O0 it must be
#                            this one.
#
# the line number is asserted against the fixture's source rather than frozen as a
# number, so editing the fixture cannot silently make the golden a statement about
# nothing. requires llvm-dwarfdump on the runner; a missing tool is a hard error.
produce_prologue_lines() {
    g=$4
    src=$(dirname "$0")/src/main.mach
    dd_tool=$(resolve_dwarfdump) || {
        echo "link: 3394-prologue-lines: llvm-dwarfdump not found (install the 'llvm' package)" >&2; return 2
    }

    want_line=$(grep -n '^fun main' "$src" | cut -d: -f1)
    want_file=src/main.mach
    [ -n "$want_line" ] || { echo "link: 3394-prologue-lines: the fixture has no 'fun main' line" >&2; return 2; }

    # main's DIE: its address range and its declaration attributes
    read -r low high decl_file decl_line <<EOF
$("$dd_tool" --debug-info "$g" | awk '
    /DW_TAG_subprogram/ { name = ""; low = ""; high = ""; df = ""; dl = "" }
    /DW_AT_name/ && match($0, /\("[^"]*"\)/) { name = substr($0, RSTART + 2, RLENGTH - 4) }
    /DW_AT_low_pc/ && match($0, /0x[0-9a-fA-F]+/) { low = substr($0, RSTART, RLENGTH) }
    /DW_AT_high_pc/ && match($0, /0x[0-9a-fA-F]+/) { high = substr($0, RSTART, RLENGTH) }
    /DW_AT_decl_file/ && match($0, /\("[^"]*"\)/) { df = substr($0, RSTART + 2, RLENGTH - 4) }
    /DW_AT_decl_line/ && match($0, /\([0-9]+\)/) { dl = substr($0, RSTART + 1, RLENGTH - 2) }
    /^$/ { if (name == "main" && low != "") { print low, high, df, dl; exit } }
')
EOF
    [ -n "$low" ] || { echo "link: 3394-prologue-lines: no 'main' subprogram with a low_pc in $g" >&2; return 2; }

    # lookup_file <address> — the source file a consumer resolves an address to
    lookup_file() {
        "$dd_tool" --lookup="$1" "$g" | sed -n "s/^Line info: file '\([^']*\)'.*/\1/p"
    }
    # lookup_line <address> — and its line
    lookup_line() {
        "$dd_tool" --lookup="$1" "$g" | sed -n "s/^Line info: file '[^']*', line \([0-9]*\).*/\1/p"
    }

    entry_file=$(lookup_file "$low")
    entry_line=$(lookup_line "$low")
    if [ "$entry_file" = "$want_file" ] && [ "$entry_line" = "$want_line" ]; then
        echo "main_entry=fun-main"
    else
        echo "main_entry=$entry_file:$entry_line"
    fi

    if [ "$decl_file" = "./$want_file" ] && [ "$decl_line" = "$want_line" ]; then
        echo "main_decl=fun-main"
    else
        echo "main_decl=$decl_file:$decl_line"
    fi

    # the first prologue_end row inside main's range, from the line table itself:
    # --lookup reports the row covering an address, not where a flag was set.
    pe=$("$dd_tool" --debug-line "$g" | awk -v lo="$low" -v hi="$high" '
        /prologue_end/ && $1 ~ /^0x[0-9a-fA-F]+$/ {
            a = strtonum($1)
            if (a >= strtonum(lo) && a < strtonum(hi)) { print $1; exit }
        }
    ')
    [ -n "$pe" ] || { echo "link: 3394-prologue-lines: main has no prologue_end row" >&2; return 2; }

    if [ "$(printf '%d' "$pe")" -gt "$(printf '%d' "$low")" ]; then
        echo "prologue_end_after_entry=yes"
    else
        echo "prologue_end_after_entry=no"
    fi

    body_file=$(lookup_file "$pe")
    if [ "$body_file" = "$want_file" ]; then
        echo "body_starts=same-file"
    else
        echo "body_starts=other-file"
    fi
}

produce_prologue_lines "$@"
