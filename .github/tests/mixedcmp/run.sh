#!/usr/bin/env bash
# mixed-numeric comparison regression test (issues #1248 / #1251): build the
# `app` project with the freshly-built compiler and run it. the app self-checks
# integer comparisons across every signedness/width class and float-width
# comparisons, asserting each boundary case in BOTH operand orders (the original
# bug was an order-dependent miscompile), and exits 0 only when every result is
# value-correct; any wrong comparison exits with a non-zero check id.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
# absolutize a path argument: the suite cds into a temp dir, so a relative
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

if ! ( cd "$WORK/app" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "FAIL mixedcmp: build failed" >&2
    exit 1
fi

set +e
"$WORK/app/out/linux/bin/mixedcmpapp"
code=$?
set -e

if [ "$code" -ne 0 ]; then
    echo "FAIL mixedcmp: comparison check $code produced a wrong result" >&2
    exit 1
fi

echo "PASS mixedcmp: mixed-numeric comparisons value-correct and order-independent"
exit 0
