#!/usr/bin/env bash
# Mach compiler test runner.
#
# Usage: run.sh [path-to-compiler]
# Compiles each tests/NNN_*.mach, runs it, and checks the expected exit
# code declared on line 1 (`# expect: exit N`).

set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
MACHC="${1:-${MACHC:-$HERE/../out/bin/machc}}"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

pass=0
fail=0
skip=0

have_compiler=1
if [ ! -x "$MACHC" ]; then
    have_compiler=0
    echo "no compiler at $MACHC — build it first (M1). tests pending."
    echo "---"
fi

for src in "$HERE"/[0-9]*.mach; do
    [ -e "$src" ] || continue
    name="$(basename "$src" .mach)"
    want="$(sed -n '1s/.*expect: exit \([0-9]*\).*/\1/p' "$src")"

    if [ "$have_compiler" = 0 ]; then
        printf 'SKIP %s\n' "$name"
        skip=$((skip + 1))
        continue
    fi

    bin="$WORK/$name"
    if ! "$MACHC" "$src" -o "$bin" > "$WORK/$name.log" 2>&1; then
        printf 'FAIL %s (compile)\n' "$name"
        sed 's/^/     | /' "$WORK/$name.log"
        fail=$((fail + 1))
        continue
    fi

    "$bin"
    got=$?
    if [ "$got" = "$want" ]; then
        printf 'PASS %s\n' "$name"
        pass=$((pass + 1))
    else
        printf 'FAIL %s (exit: got %s, want %s)\n' "$name" "$got" "$want"
        fail=$((fail + 1))
    fi
done

echo "---"
echo "pass=$pass fail=$fail skip=$skip"
[ "$fail" = 0 ]
