#!/usr/bin/env bash
# SysV float-aggregate C-FFI test (#1257 BUG/L): link a Mach program against a
# C-compiled object that exchanges by-value structs containing floats, and confirm
# the values cross the boundary correctly.
#
# the System V AMD64 psABI classifies an aggregate per eightbyte: an all-float
# eightbyte is SSE class and travels in an XMM register, so `{f64,f64}` passes and
# returns in XMM0:XMM1, `{f32,f32}` in one XMM, and `{i64,f64}` in RDI:XMM0. before
# the Mach compiler implemented SSE-eightbyte classification it passed/returned
# float-bearing aggregates in GP registers, so a C function reading XMM saw garbage.
# this compiles `cffi.c` with the system `cc` (skipped when absent), links the Mach
# consumer against the object, and runs it — `main` exits 0 only when every
# float-struct call produced the right value (1=vec2_sum arg, 2/3=vec2_scale return,
# 4=f2_sum one-eightbyte, 5=mixed INTEGER:SSE).
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

if ! command -v cc >/dev/null 2>&1; then
    echo "SKIP cffi: no system 'cc' to compile the C boundary object"
    exit 0
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"

# compile the C boundary object exporting the float-struct functions.
OBJ="$WORK/cffi.o"
if ! cc -c "$HERE/cffi.c" -o "$OBJ"; then
    echo "FAIL cffi: could not compile cffi.c with cc" >&2
    exit 1
fi

# build the Mach consumer, linking the C object.
if ! ( cd "$WORK/app" && "$MACH" build . "$OBJ" ) >/dev/null 2>&1; then
    echo "FAIL cffi: build linking the C object failed" >&2
    exit 1
fi

BIN="$WORK/app/out/linux/bin/cffiapp"
test -x "$BIN" || { echo "error: binary not produced at $BIN" >&2; exit 1; }

set +e
"$BIN"
code=$?
set -e

if [ "$code" -eq 0 ]; then
    echo "PASS cffi: by-value float structs cross the C boundary in SSE registers"
    exit 0
fi

echo "FAIL cffi: float-struct C-FFI miscompiled (exit $code: 1=vec2_sum, 2/3=vec2_scale, 4=f2_sum, 5=mixed)" >&2
exit 1
