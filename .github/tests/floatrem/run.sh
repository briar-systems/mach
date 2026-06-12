#!/usr/bin/env bash
# float remainder semantics test (#1378): float `%` lowered through the integer
# IDIV/DIV path over the raw IEEE-754 bit patterns instead of a float-remainder
# sequence, so it returned a passthrough-below-divisor / near-zero-above shape
# rather than the truncated (C fmod) remainder (result takes the dividend's sign).
# this builds a self-checking program covering f32/f64, const-folded and runtime
# operands, lhs<rhs / lhs>rhs / exact multiples, and negative dividend / divisor,
# then runs it natively. it builds and runs the SAME source at -O0 and -O2 and
# requires both to exit 0, proving the debug and release pipelines agree on fmod.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
# absolutize a path argument: the suites cd into temp dirs, so a relative
# compiler path would break after the first cd.
case "$MACH" in */*) MACH="$(cd "$(dirname "$MACH")" && pwd)/$(basename "$MACH")";; esac
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$HERE/../../.." && pwd)"
STD="$REPO_ROOT/dep/mach-std"

if [ ! -d "$STD/src" ]; then
    echo "error: vendored std not found at $STD (run: git submodule update --init)" >&2
    exit 2
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"

for opt in O0 O2; do
    BIN="$WORK/app/floatrem-$opt"
    if ! ( cd "$WORK/app" && "$MACH" build . -$opt -o "$BIN" ) >/dev/null 2>&1; then
        echo "FAIL floatrem: native build failed at -$opt" >&2
        exit 1
    fi
    test -x "$BIN" || { echo "error: binary not produced at $BIN" >&2; exit 1; }

    set +e
    "$BIN"
    code=$?
    set -e

    if [ "$code" -ne 0 ]; then
        echo "FAIL floatrem: float % miscompiled at -$opt (case $code)" >&2
        exit 1
    fi
    echo "PASS floatrem: float % matches the truncated (fmod) remainder at -$opt"
done

exit 0
