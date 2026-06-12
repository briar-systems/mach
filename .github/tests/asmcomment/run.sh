#!/usr/bin/env bash
# inline-asm comment regression test (#1297): a comment inside an `asm { }` block
# was not opaque to the instruction parser. the body was split on `;` and scanned
# for `{name}` bindings before `#` comments were stripped, so a comment holding a
# `;` became a phantom instruction and one holding `{...}` became a bogus local
# binding — either aborted the build. this builds a program whose asm carries a
# kitchen-sink comment (`;`, `{...}`, `%`, backtick, quotes) and runs it; main
# exits 0 only when the asm encoded and computed the right value.
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
    echo "FAIL asmcomment: native build failed (a comment's bytes leaked into the instruction parser)" >&2
    exit 1
fi

BIN="$WORK/app/out/linux/bin/asmcomment"
test -x "$BIN" || { echo "error: binary not produced at $BIN" >&2; exit 1; }

set +e
"$BIN"
code=$?
set -e

if [ "$code" -eq 0 ]; then
    echo "PASS asmcomment: a kitchen-sink asm comment is opaque to the instruction parser"
    exit 0
fi

echo "FAIL asmcomment: inline-asm comment miscompiled (exit $code)" >&2
exit 1
