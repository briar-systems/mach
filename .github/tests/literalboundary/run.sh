#!/usr/bin/env bash
# integer literal boundary regression test (issue #1242 A9): build the `app`
# project with the freshly-built compiler and run it. the app assigns the
# max-valid literal to a variable of each integer type, then checks that the
# stored value matches the expected magnitude, exiting 0 only when every check
# passes. a mismatch exits with a non-zero check id.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
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

if ! ( cd "$WORK/app" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "FAIL literalboundary: build failed" >&2
    exit 1
fi

set +e
"$WORK/app/out/linux/bin/literalboundary"
code=$?
set -e

if [ "$code" -ne 0 ]; then
    echo "FAIL literalboundary: check $code produced a wrong value" >&2
    exit 1
fi

echo "PASS literalboundary: max-valid integer literals accepted and round-trip correctly"
exit 0
