#!/usr/bin/env bash
# win64 erased function-pointer cast regression test (#1224): a function value
# cast to a type-erased `*u8` slot and recalled with a `*u8::Fn` cast must keep
# its full 64-bit code address. `scalar_bits` omitted the function type, so
# `lower_cast` treated `fn::*u8` as a zero-extension instead of a same-size
# bitcast; on x86-64 that encodes as a 32-bit `mov ecx, eax`, zeroing the high
# 32 bits of the address. harmless on linux (addresses < 2GB) but on win64
# (ImageBase 0x140000000) the stored vtable pointer faults the first time it is
# called through — the deterministic execute-access fault that blocked a native
# windows mach.exe.
#
# cross-compiles a tiny windows program that round-trips `inc` through an erased
# `*u8` slot and calls it under wine. main returns 0 only when the recalled
# pointer is intact (1 = the truncated pointer miscomputed; a hard fault is a
# nonzero exit either way).
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$HERE/../../.." && pwd)"
STD="$REPO_ROOT/dep/mach-std"

if [ ! -d "$STD/src" ]; then
    echo "error: vendored std not found at $STD (run: mach dep pull)" >&2
    exit 2
fi

# the test runs a windows binary; without wine there is nothing to execute it.
if ! command -v wine >/dev/null 2>&1; then
    echo "SKIP win64fnptr: 'wine' not available to run the windows binary"
    exit 0
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"

# build at -O0 so the cast is not folded away — the `fn::*u8` lowering must
# actually be exercised.
if ! ( cd "$WORK/app" && "$MACH" build . -O0 ) >/dev/null 2>&1; then
    echo "FAIL win64fnptr: windows cross-build failed" >&2
    exit 1
fi

EXE="$WORK/app/out/windows/bin/win64fnptr.exe"
test -f "$EXE" || { echo "error: windows binary not produced at $EXE" >&2; exit 1; }

# bound the run with `timeout`: a regressed binary faults, and wine's crash
# handler (winedbg --auto) can hang rather than exit, which would stall the
# integration job — the timeout turns that hang into a fast nonzero exit (124).
set +e
WINEDEBUG=-all timeout 60 wine "$EXE" >/dev/null 2>&1
code=$?
set -e

if [ "$code" -eq 0 ]; then
    echo "PASS win64fnptr: erased function pointer round-trips its full 64-bit address"
    exit 0
fi

echo "FAIL win64fnptr: recalled function pointer faulted or miscomputed (exit $code)" >&2
exit 1
