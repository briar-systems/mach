#!/usr/bin/env bash
# module-qualified constant paths in global initialisers (#1290) and the span on
# their rejection (#1289). two halves:
#
#   fold — a global folded from aliased `alias.CONST` member paths must compile.
#   the `fold` app reproduces the exact mach-std THREAD_CLONE_FLAGS shape (a
#   global OR-ed from clone-flag constants reached through a module alias) plus
#   chained constants (an aliased member that is itself built from other
#   constants), and asserts every folded value at runtime, exiting 0 only when
#   all match. before #1290 this failed with "a global initialiser must be a
#   constant expression"; mach-std worked around it with unaliased imports.
#
#   reject — a module-qualified member that does NOT name a module-level constant
#   (a pub `var`) must still be rejected, and the diagnostic must carry the file,
#   line, and symbol name (#1289), never the old fileless abort. the `reject`
#   app's build must fail with a located, named diagnostic.
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

# vendor <app-dir>: copy the app into the temp dir and symlink the vendored std.
vendor() {
    local app="$1"
    local dst="$WORK/$app"
    cp -r "$HERE/$app" "$dst"
    mkdir -p "$dst/dep"
    ln -s "$STD" "$dst/dep/mach-std"
}

# build_and_run <app-dir> <expected-exit>: build the app, run the binary, and
# require the build to succeed and the run to exit with the expected code.
build_and_run() {
    local app="$1"; local want="$2"
    vendor "$app"
    local dst="$WORK/$app"
    if ! ( cd "$dst" && "$MACH" build . ) >/dev/null 2>&1; then
        echo "FAIL globalmember/$app: build failed" >&2; fail=1; return
    fi
    local bin
    bin="$(find "$dst/out" -type f -path '*/bin/*' | head -n1)"
    if [ -z "$bin" ]; then
        echo "FAIL globalmember/$app: no binary produced" >&2; fail=1; return
    fi
    set +e
    "$bin"; local code=$?
    set -e
    if [ "$code" -ne "$want" ]; then
        echo "FAIL globalmember/$app: ran with exit $code, expected $want" >&2; fail=1; return
    fi
    echo "PASS globalmember/$app: aliased module-qualified constants fold in global initialisers"
}

# expect_reject <app-dir>: build the app and require the build to FAIL with a
# diagnostic that is located on the binding (file:line:col) AND names the symbol
# (#1289), never the old fileless "a global initialiser must be a constant
# expression" abort.
expect_reject() {
    local app="$1"
    vendor "$app"
    local dst="$WORK/$app"
    local out
    if out="$( cd "$dst" && "$MACH" build . 2>&1 )"; then
        echo "FAIL globalmember/$app: build unexpectedly succeeded for a non-constant member path" >&2; fail=1; return
    fi
    if ! printf '%s' "$out" | grep -qF -- 'global initialiser of `NOT_CONST` must be a constant expression'; then
        echo "FAIL globalmember/$app: diagnostic missing the symbol name; got:" >&2
        printf '%s\n' "$out" >&2; fail=1; return
    fi
    if ! printf '%s' "$out" | grep -qE 'main\.mach:[0-9]+:[0-9]+:'; then
        echo "FAIL globalmember/$app: diagnostic missing file:line:col; got:" >&2
        printf '%s\n' "$out" >&2; fail=1; return
    fi
    echo "PASS globalmember/$app: a non-constant member path is rejected with a located, named diagnostic"
}

build_and_run fold 0
expect_reject reject

exit "$fail"
