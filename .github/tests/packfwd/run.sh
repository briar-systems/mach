#!/usr/bin/env bash
# whole-pack `va...` forward (#1475): inside a pack instance, `g(va...)` threads the
# instance's concrete pack parameters through to another pack-tailed callee, which
# is monomorphized for the forwarded type-list. this builds a program that forwards
# packs (incl. into a callee with a leading fixed param, and a forward-of-a-forward)
# and runs it; main exits 0 only when every forward delivers the right elements.
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
    echo "FAIL packfwd: build failed" >&2
    exit 1
fi

BIN="$WORK/app/out/linux/bin/packfwd"
test -x "$BIN" || { echo "error: binary not produced at $BIN" >&2; exit 1; }

set +e
"$BIN"; code=$?
set -e
if [ "$code" -ne 0 ]; then
    echo "FAIL packfwd: check $code forwarded the wrong elements" >&2
    exit 1
fi
echo "PASS packfwd: va... threads the instance's concrete pack through to a pack-tailed callee"
