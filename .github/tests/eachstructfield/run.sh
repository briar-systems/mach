#!/usr/bin/env bash
# #1549 — field access (`a.x`) on a struct-typed `$each` pack element resolves to
# the element's storage slot instead of fabricating an undefined extern global. a
# struct pack element reached the lvalue path (field access lowers its receiver as
# an lvalue) which had no pack branch, so it fell through to a fabricated extern
# global named after the loop variable and failed at link. this builds a program
# that reads struct pack-element fields (and takes their address via `?a`) across
# struct-only and mixed packs, then runs it natively; main exits 0 only when every
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
    echo "FAIL eachstructfield: build failed" >&2
    exit 1
fi

BIN="$WORK/app/out/linux/bin/eachstructfield"
test -x "$BIN" || { echo "error: binary not produced at $BIN" >&2; exit 1; }

set +e
"$BIN"; code=$?
set -e
if [ "$code" -ne 0 ]; then
    echo "FAIL eachstructfield: check $code computed the wrong value" >&2
    exit 1
fi
echo "PASS eachstructfield: field access and ?a on a struct \$each pack element resolve to its slot"
