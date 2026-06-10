#!/usr/bin/env bash
# lowering regression suite: each app exercises a lowering bug from the 2026-06
# audit (issue #1251) that produced a compiler crash, a link-time failure, or a
# silently wrong instance body. every app must build cleanly and run to exit 0.
#
#   comptime-str: a comptime `*u8`/str value parameter referenced in the body
#                 folds to a string constant in expression position — used to
#                 panic codegen with "unexpected IR value kind in lower_value".
#   xmod-generic: a generic body whose `$if` gates on the defining module's
#                 `pub val`, instantiated from another module — used to fail with
#                 "identifier is not a comptime constant in scope".
#   fnptr-table:  variable-index function-pointer-table dispatch `table[i](..)`
#                 — used to misparse as a generic call and fail in resolve (#1270).
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
# absolutize a path argument: the suite cds into temp dirs, so a relative
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

fail=0

# build_and_run <app-dir-name> <expected-exit>: copy the app into a temp dir,
# vendor std, build it, run the binary, and check its exit code.
build_and_run() {
    local app="$1"; local want="$2"
    local dst="$WORK/$app"
    cp -r "$HERE/$app" "$dst"
    mkdir -p "$dst/dep"
    ln -s "$STD" "$dst/dep/mach-std"
    if ! ( cd "$dst" && "$MACH" build . ) >/dev/null 2>&1; then
        echo "FAIL lower/$app: build failed" >&2; fail=1; return
    fi
    local bin
    bin="$(find "$dst/out" -type f -path '*/bin/*' | head -n1)"
    if [ -z "$bin" ]; then
        echo "FAIL lower/$app: no binary produced" >&2; fail=1; return
    fi
    set +e
    "$bin"
    local code=$?
    set -e
    if [ "$code" -ne "$want" ]; then
        echo "FAIL lower/$app: ran with exit $code, expected $want" >&2; fail=1; return
    fi
    echo "PASS lower/$app"
}

build_and_run comptime-str 0
build_and_run xmod-generic 0
build_and_run fnptr-table 0

exit "$fail"
