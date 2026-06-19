#!/usr/bin/env bash
# FP-bank interference regression test (#1254): the precolored-register
# reservation and param-pinning interference machinery was GP-only, so an FP
# value live across a float argument-setup move could be colored onto the XMM
# register that move writes. swapped float arguments then collapsed to one value
# (take(b, a) returned b - b == 0). this builds a float-heavy program at -O0 (so
# the calls are real, not inlined) and runs it natively; main exits 0 only when
# every float forwarding produced the right value (1 = 2-arg swap, 2 = 3-arg
# reverse, 3 = call-crossing float).
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
    echo "error: vendored std not found at $STD (run: mach dep pull)" >&2
    exit 2
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"

# build at -O0 so the calls are not inlined away — the FP argument-setup path
# must actually be exercised at the call boundary.
if ! ( cd "$WORK/app" && "$MACH" build . -O0 ) >/dev/null 2>&1; then
    echo "FAIL fpargs: native build failed" >&2
    exit 1
fi

BIN="$WORK/app/out/linux/bin/fpargs"
test -x "$BIN" || { echo "error: binary not produced at $BIN" >&2; exit 1; }

set +e
"$BIN"
code=$?
set -e

if [ "$code" -eq 0 ]; then
    echo "PASS fpargs: swapped and call-crossing float arguments forward correctly"
    exit 0
fi

echo "FAIL fpargs: float argument forwarding miscompiled (exit $code: 1=swap, 2=reverse, 3=call-crossing)" >&2
exit 1
