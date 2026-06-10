#!/usr/bin/env bash
# win64 variadic regression test (#1253): the home-area variadic model had two
# confirmed miscompiles — a variadic float was never duplicated into the matching
# integer register (so va_arg[f64] read garbage from the home slot), and va_start
# seeded the cursor past only the four register slots, so a function with more than
# four named parameters returned a named argument from its first va_arg. va_list is
# also a single pointer (8 bytes) under this model, not the 24-byte SysV tag. this
# cross-compiles a win64 program exercising all three and runs it under wine; main
# exits 0 only when every variadic read is correct (1=float dup, 2=va_list size,
# 3=va_start past stack-passed named params, 4=mixed int/float).
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
    echo "error: vendored std not found at $STD (run: git submodule update --init)" >&2
    exit 2
fi

# the test runs a windows binary; without wine there is nothing to execute it.
if ! command -v wine >/dev/null 2>&1; then
    echo "SKIP win64va: 'wine' not available to run the windows binary"
    exit 0
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"

# build at -O0 so the variadic calls are real, not folded.
if ! ( cd "$WORK/app" && "$MACH" build . -O0 ) >/dev/null 2>&1; then
    echo "FAIL win64va: windows cross-build failed" >&2
    exit 1
fi

EXE="$WORK/app/out/windows/bin/win64va.exe"
test -f "$EXE" || { echo "error: windows binary not produced at $EXE" >&2; exit 1; }

set +e
WINEDEBUG=-all wine "$EXE" >/dev/null 2>&1
code=$?
set -e

if [ "$code" -eq 0 ]; then
    echo "PASS win64va: variadic float dup, va_start slot seeding, and va_list pointer are correct"
    exit 0
fi

echo "FAIL win64va: win64 variadic miscompiled (exit $code: 1=float dup, 2=va_list size, 3=va_start slots, 4=mixed)" >&2
exit 1
