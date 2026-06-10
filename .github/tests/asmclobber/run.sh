#!/usr/bin/env bash
# inline-asm clobber regression test (#1254): regalloc modeled an inline-asm
# block only as a caller-saved call barrier, ignoring the callee-saved registers
# the body writes. a value parked in such a register was overwritten (1), and an
# asm-bearing function corrupted its caller's preserved register because the
# clobbered register never reached the prologue save mask (2). this builds a
# program exercising both shapes at -O0 (so the asm and calls are real) and runs
# it natively; main exits 0 only when both reads see the correct value.
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

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"

# build at -O0 so the asm blocks and calls are not optimized away — the clobber
# barrier must be exercised at the asm and the call boundary.
if ! ( cd "$WORK/app" && "$MACH" build . -O0 ) >/dev/null 2>&1; then
    echo "FAIL asmclobber: native build failed" >&2
    exit 1
fi

BIN="$WORK/app/out/linux/bin/asmclobber"
test -x "$BIN" || { echo "error: binary not produced at $BIN" >&2; exit 1; }

set +e
"$BIN"
code=$?
set -e

if [ "$code" -eq 0 ]; then
    echo "PASS asmclobber: a value across an asm and a caller's register survive the asm's clobbers"
    exit 0
fi

echo "FAIL asmclobber: inline-asm clobber miscompiled (exit $code: 1=value parked in clobbered reg, 2=caller corruption)" >&2
exit 1
