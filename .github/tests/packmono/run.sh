#!/usr/bin/env bash
# comptime variadic-pack monomorphization (#1475): a pack-tailed function (`va: ...`)
# is monomorphized once per distinct call-site arg-type-list, the pack expanded to N
# concrete parameters and `$each a in va` unrolled over them. this builds a program
# that sums packs of several arities (and one with a leading fixed parameter), then
# runs it natively; main exits 0 only when every instance computes the right value.
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
    echo "FAIL packmono: build failed" >&2
    exit 1
fi

BIN="$WORK/app/out/linux/bin/packmono"
test -x "$BIN" || { echo "error: binary not produced at $BIN" >&2; exit 1; }

set +e
"$BIN"; code=$?
set -e
if [ "$code" -ne 0 ]; then
    echo "FAIL packmono: check $code computed the wrong value" >&2
    exit 1
fi
echo "PASS packmono: pack-tailed fns monomorphize per type-list and \$each a in va unrolls"
