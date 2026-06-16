#!/usr/bin/env bash
# $each / $fields / v.[f] expansion integration test (#1473).
#
# `$each f in $fields(T) { ... v.[f] ... }` unrolls at compile time, once per
# field of T, binding `f` to that field's descriptor so `v.[f]` projects the
# concrete field. the app asserts the splice computes correct values for a
# homogeneous record (rvalue projection), a heterogeneous record (per-iteration
# typing), a pointer-receiver write (v.[f] lvalue), and an empty record (no
# expansion); it exits 0 only when all pass, returning the failed check's id
# otherwise. end-to-end proof that the sema + lowering unroll splice agree.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
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
    echo "FAIL eachfields: build failed" >&2
    exit 1
fi

BIN="$WORK/app/out/linux/bin/eachfields"
set +e
"$BIN"; code=$?
set -e
if [ "$code" -ne 0 ]; then
    echo "FAIL eachfields: check $code computed the wrong value" >&2
    exit 1
fi
echo "PASS eachfields: \$each over \$fields projects v.[f] correctly"
