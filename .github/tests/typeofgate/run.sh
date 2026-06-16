#!/usr/bin/env bash
# $type_of type-gate integration test (#1472). two halves:
#
#   folding — `$type_of(expr)` yields a comptime TYPE value, and a type `==` / `!=`
#   inside a `$if` gate folds to an identity compare at lowering. the app asserts
#   every gate selects the arm its operand types dictate and exits 0 only when all
#   match; a wrong arm returns that gate's id. this is the end-to-end proof that
#   the lowering-time type resolver folds the gate against the real types.
#
#   rejection — a type comparison is comptime-only; used in a runtime value
#   position the build must FAIL with the teaching diagnostic, never compile.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

# the exact diagnostic sema emits for a type comparison in a value position.
REJECT_DIAG='a `$type_of` type comparison is comptime-only; it is valid only as a `$if` gate condition'

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

# build_and_run <app-dir> <label>: vendor std, build, run, require exit 0. the
# app's own main returns the misselected gate's id on a wrong arm.
build_and_run() {
    local app="$1"; local label="$2"
    local dst="$WORK/$app"
    cp -r "$HERE/$app" "$dst"
    mkdir -p "$dst/dep"
    ln -s "$STD" "$dst/dep/mach-std"
    if ! ( cd "$dst" && "$MACH" build . ) >/dev/null 2>&1; then
        echo "FAIL typeofgate/$label: build failed" >&2; fail=1; return
    fi
    local bin
    bin="$(find "$dst/out" -type f -path '*/bin/*' | head -n1)"
    if [ -z "$bin" ]; then
        echo "FAIL typeofgate/$label: no binary produced" >&2; fail=1; return
    fi
    set +e
    "$bin"; local code=$?
    set -e
    if [ "$code" -ne 0 ]; then
        echo "FAIL typeofgate/$label: gate id $code selected the wrong arm" >&2; fail=1; return
    fi
    echo "PASS typeofgate/$label: \$type_of gates fold to the right arm"
}

# expect_reject <app-dir> <label> <diagnostic>: vendor std, build, and require
# the build to FAIL with `diagnostic` present verbatim on stderr.
expect_reject() {
    local app="$1"; local label="$2"; local want="$3"
    local dst="$WORK/$app"
    cp -r "$HERE/$app" "$dst"
    mkdir -p "$dst/dep"
    ln -s "$STD" "$dst/dep/mach-std"
    local out
    if out="$( cd "$dst" && "$MACH" build . 2>&1 )"; then
        echo "FAIL typeofgate/$label: build unexpectedly succeeded for a value-position type comparison" >&2; fail=1; return
    fi
    if ! printf '%s' "$out" | grep -qF -- "$want"; then
        echo "FAIL typeofgate/$label: missing the teaching diagnostic; got:" >&2
        printf '%s\n' "$out" >&2; fail=1; return
    fi
    echo "PASS typeofgate/$label: value-position type comparison rejected with the teaching diagnostic"
}

build_and_run app    "type gate folding"
expect_reject reject "value-position type comparison" "$REJECT_DIAG"

exit "$fail"
