#!/usr/bin/env bash
# generic $each $fields(T) deferral integration test (#1523). two halves:
#
#   app — a generic fun instantiated on concrete records must COMPILE and
#   produce correct results for homogeneous / heterogeneous / empty records,
#   pointer-receiver zero, field counts, and name queries. exits 0 on pass.
#
#   nonrec — `$fields(T)` where T is instantiated as a non-record (e.g. u64)
#   must FAIL at compile time with a recognisable diagnostic so the error is
#   surfaced at the call site rather than silently producing wrong code.
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

# half 1: the generic fields app must build and run correctly.
cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"

if ! ( cd "$WORK/app" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "FAIL eachfieldsgeneric(app): build failed" >&2
    exit 1
fi

BIN="$WORK/app/out/linux/bin/eachfieldsgeneric"
set +e
"$BIN"; code=$?
set -e
if [ "$code" -ne 0 ]; then
    echo "FAIL eachfieldsgeneric(app): check $code returned wrong value" >&2
    exit 1
fi

# half 2: instantiating $fields(T) with a non-record must fail at compile time.
cp -r "$HERE/nonrec" "$WORK/"
mkdir -p "$WORK/nonrec/dep"
ln -s "$STD" "$WORK/nonrec/dep/mach-std"

set +e
BUILD_OUT="$( cd "$WORK/nonrec" && "$MACH" build . 2>&1 )"
code=$?
set -e
if [ "$code" -eq 0 ]; then
    echo "FAIL eachfieldsgeneric(nonrec): build should have failed but succeeded" >&2
    exit 1
fi
if ! echo "$BUILD_OUT" | grep -qi "non-record\|record type\|not a record"; then
    echo "FAIL eachfieldsgeneric(nonrec): expected a record-type diagnostic, got:" >&2
    echo "$BUILD_OUT" >&2
    exit 1
fi

echo "PASS eachfieldsgeneric: generic \$each \$fields(T) defers and unrolls correctly"
