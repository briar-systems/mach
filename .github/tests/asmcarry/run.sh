#!/usr/bin/env bash
# inline-asm carry-flag mnemonic test (#1359): the x64 inline-asm parser only
# dispatched je/jz/jne/jnz, so jc/jnc/setc failed with "unknown x86_64 inline-asm
# instruction" and std had to hand-encode carry handling via `.byte`. this builds
# a program that uses setc (== setb) to materialize the carry flag and jc/jnc
# (== jb/jae/jnb) to branch on it, then runs it natively; main exits 0 only when
# every new mnemonic encodes and executes as its carry-flag form.
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

if ! ( cd "$WORK/app" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "FAIL asmcarry: native build failed (jc/jnc/setc not encoded?)" >&2
    exit 1
fi

BIN="$WORK/app/out/linux/bin/asmcarry"
test -x "$BIN" || { echo "error: binary not produced at $BIN" >&2; exit 1; }

set +e
"$BIN"
code=$?
set -e

if [ "$code" -eq 0 ]; then
    echo "PASS asmcarry: setc / jc / jnc (and the jb/jae/jnb/setb/setae/setnb aliases) encode and run as the carry-flag forms"
    exit 0
fi

echo "FAIL asmcarry: carry-flag inline-asm mnemonic miscompiled (exit $code)" >&2
exit 1
