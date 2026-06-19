#!/usr/bin/env bash
# comptime-vs-runtime comparison agreement test (issue #1272): build the `app`
# project with the freshly-built compiler and run it. the app folds u64-range and
# cross-sign integer comparisons through `$if` gates AND computes the same
# comparisons at runtime off a non-foldable zero, asserting the comptime fold and
# the runtime result agree for every boundary sentinel. it exits 0 only when every
# pair agrees; a divergence exits with the sentinel's non-zero id.
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
    echo "error: vendored std not found at $STD (run: mach dep pull)" >&2
    exit 2
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"

if ! ( cd "$WORK/app" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "FAIL ctcmp: build failed" >&2
    exit 1
fi

set +e
"$WORK/app/out/linux/bin/ctcmpapp"
code=$?
set -e

if [ "$code" -ne 0 ]; then
    echo "FAIL ctcmp: comptime \$if disagreed with runtime at check $code" >&2
    exit 1
fi

echo "PASS ctcmp: comptime \$if and runtime comparisons agree on the boundary sentinels"
exit 0
