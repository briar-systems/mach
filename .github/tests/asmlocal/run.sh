#!/usr/bin/env bash
# inline-asm numeric local label test (#1365): the x64 inline-asm grammar had no
# NASM-style numeric locals, so a block-local forward / backward branch (the
# `jc 1f` / `1:` shape dep/mach-std's darwin syscall wrappers use) could not be
# written — branch targets had to be relocatable symbols. this builds a program
# exercising a backward loop (`jnz 1b`), forward branches (`jc 1f` / `jmp 2f`),
# and redefinition of the same number in both directions, runs it natively (main
# exits 0 only when every label resolves and executes correctly), then proves an
# undefined forward reference is a hard compile error with a located message.
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

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp -r "$HERE/app" "$WORK/"
cp -r "$HERE/bad" "$WORK/"
mkdir -p "$WORK/app/dep" "$WORK/bad/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"
ln -s "$STD" "$WORK/bad/dep/mach-std"

# positive cases: backward loop, forward branches, and redefinition.
if ! ( cd "$WORK/app" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "FAIL asmlocal: native build failed (numeric local labels not encoded?)" >&2
    exit 1
fi

BIN="$WORK/app/out/linux/bin/asmlocal"
test -x "$BIN" || { echo "error: binary not produced at $BIN" >&2; exit 1; }

set +e
"$BIN"
code=$?
set -e

if [ "$code" -ne 0 ]; then
    echo "FAIL asmlocal: numeric-local-label program miscompiled (exit $code)" >&2
    exit 1
fi
echo "PASS asmlocal: backward (1b) loop, forward (1f / 2f) branches, and redefinition resolve and run"

# negative case: an undefined forward reference is a hard, located compile error.
set +e
ERR="$( cd "$WORK/bad" && "$MACH" build . 2>&1 )"
bcode=$?
set -e

if [ "$bcode" -eq 0 ]; then
    echo "FAIL asmlocal: build of an undefined local-label reference unexpectedly succeeded" >&2
    exit 1
fi
if ! printf '%s' "$ERR" | grep -q "local label"; then
    echo "FAIL asmlocal: undefined local-label error lacked a located message:" >&2
    printf '%s\n' "$ERR" >&2
    exit 1
fi
echo "PASS asmlocal: an undefined forward reference is a located compile error"
exit 0
