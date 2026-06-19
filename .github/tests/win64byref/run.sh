#!/usr/bin/env bash
# win64 byref 5th-argument regression test (#1228): a >8-byte or sub-pointer
# aggregate passed as the 5th+ win64 argument is passed by hidden reference, so
# its register slot being exhausted spills the 8-byte pointer to the stack
# (CLASS_STACK_BYREF). the caller must STORE that pointer and the callee must
# LOAD it; before #1228's fix the callee LEA'd the slot as if it held the
# aggregate's bytes, handing itself garbage.
#
# cross-compiles a tiny windows program that calls two such functions — one
# taking a 3-byte aggregate, one a 12-byte aggregate, each as the 5th argument —
# and runs it under wine. main returns 0 only when both callees read the spilled
# pointer correctly (1 = 3-byte misread, 2 = 12-byte misread).
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

# the test runs a windows binary; without wine there is nothing to execute it.
if ! command -v wine >/dev/null 2>&1; then
    echo "SKIP win64byref: 'wine' not available to run the windows binary"
    exit 0
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"

# build at -O0 so the calls are not inlined away — the ABI byref-spill path must
# actually be exercised at the call boundary.
if ! ( cd "$WORK/app" && "$MACH" build . -O0 ) >/dev/null 2>&1; then
    echo "FAIL win64byref: windows cross-build failed" >&2
    exit 1
fi

EXE="$WORK/app/out/windows/bin/win64byref.exe"
test -f "$EXE" || { echo "error: windows binary not produced at $EXE" >&2; exit 1; }

set +e
WINEDEBUG=-all wine "$EXE" >/dev/null 2>&1
code=$?
set -e

if [ "$code" -eq 0 ]; then
    echo "PASS win64byref: 3-byte and 12-byte byref aggregates read correctly as the 5th argument"
    exit 0
fi

echo "FAIL win64byref: callee misread the byref-spilled 5th argument (exit $code: 1=3-byte, 2=12-byte)" >&2
exit 1
