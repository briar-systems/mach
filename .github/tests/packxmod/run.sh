#!/usr/bin/env bash
# cross-module pack bodies (#1475): a `va: ...` function whose `$each a in va` body
# calls functions in ANOTHER module. monomorphizing it re-types the body per
# element, which must resolve the cross-module callees' signatures from the
# all-modules typing-snapshot set the re-inference assembles. this builds a
# two-module program (main + helper) and runs it; main exits 0 only when every
# instance computes the right value.
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
    echo "FAIL packxmod: build failed" >&2
    exit 1
fi

BIN="$WORK/app/out/linux/bin/packxmod"
test -x "$BIN" || { echo "error: binary not produced at $BIN" >&2; exit 1; }

set +e
"$BIN"; code=$?
set -e
if [ "$code" -ne 0 ]; then
    echo "FAIL packxmod: check $code computed the wrong value" >&2
    exit 1
fi
echo "PASS packxmod: pack bodies resolve cross-module calls at per-element re-inference"
