#!/usr/bin/env bash
# win64 callee-saved XMM regression test (#1254): under win64 the allocator hands
# out the callee-saved vector registers XMM6–15 but the prologue never saved them.
# the FP-bank callee-save model now keeps call-crossing floats in XMM6+ and the
# prologue preserves them with 16-byte-aligned MOVAPS saves (described in .xdata as
# UWOP_SAVE_XMM128). this cross-compiles a float-heavy program (several FP values
# live across a call) and runs it under wine: a misaligned MOVAPS would fault, a
# dropped save would corrupt the result. main exits 0 only when the float math is
# correct.
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
    echo "SKIP win64fp: 'wine' not available to run the windows binary"
    exit 0
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"

# build at -O0 so the call is real and the float values genuinely live across it.
if ! ( cd "$WORK/app" && "$MACH" build . -O0 ) >/dev/null 2>&1; then
    echo "FAIL win64fp: windows cross-build failed" >&2
    exit 1
fi

EXE="$WORK/app/out/windows/bin/win64fp.exe"
test -f "$EXE" || { echo "error: windows binary not produced at $EXE" >&2; exit 1; }

set +e
WINEDEBUG=-all wine "$EXE" >/dev/null 2>&1
code=$?
set -e

if [ "$code" -eq 0 ]; then
    echo "PASS win64fp: call-crossing floats survive in callee-saved XMM6+ and the result is correct"
    exit 0
fi

echo "FAIL win64fp: win64 callee-saved XMM miscompiled or faulted (exit $code)" >&2
exit 1
