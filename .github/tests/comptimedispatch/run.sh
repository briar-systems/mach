#!/usr/bin/env bash
# type-directed comptime dispatch integration test (#1511). two halves:
#
#   clean — the no-cast `$type_of` dispatch skeleton must COMPILE: each arm uses
#   the loop variable at its own concrete type with no per-arm cast, and sema
#   prunes the provably-dead arms at monomorphization. the app exits 0 only when
#   the dispatch routed each element to its own arm (42 + 1 == 43).
#
#   unhandled — a statement-position `$error` in the unhandled-type `$or {}` arm
#   must FAIL the build with its message verbatim, so a type-directed dispatch
#   fails at COMPILE time on an unhandled type instead of a runtime fallback.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

# the exact diagnostic the unhandled app's `$error` directive emits.
ERROR_DIAG='show: unsupported argument type'

MACH="${1:-mach}"
# absolutize a path argument: the suite cds into temp dirs, so a relative
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

fail=0

# build_and_run <app-dir> <label>: vendor std, build, run, require exit 0.
build_and_run() {
    local app="$1"; local label="$2"
    local dst="$WORK/$app"
    cp -r "$HERE/$app" "$dst"
    mkdir -p "$dst/dep"
    ln -s "$STD" "$dst/dep/mach-std"
    if ! ( cd "$dst" && "$MACH" build . ) >/dev/null 2>&1; then
        echo "FAIL comptimedispatch/$label: build failed" >&2; fail=1; return
    fi
    local bin
    bin="$(find "$dst/out" -type f -path '*/bin/*' | head -n1)"
    if [ -z "$bin" ]; then
        echo "FAIL comptimedispatch/$label: no binary produced" >&2; fail=1; return
    fi
    set +e
    "$bin"; local code=$?
    set -e
    if [ "$code" -ne 0 ]; then
        echo "FAIL comptimedispatch/$label: dispatch routed wrong (exit $code)" >&2; fail=1; return
    fi
    echo "PASS comptimedispatch/$label: clean no-cast \$type_of dispatch compiles and routes per arm"
}

# expect_error <app-dir> <label> <diagnostic>: vendor std, build, require the
# build to FAIL with `diagnostic` present verbatim on stderr.
expect_error() {
    local app="$1"; local label="$2"; local want="$3"
    local dst="$WORK/$app"
    cp -r "$HERE/$app" "$dst"
    mkdir -p "$dst/dep"
    ln -s "$STD" "$dst/dep/mach-std"
    local out
    if out="$( cd "$dst" && "$MACH" build . 2>&1 )"; then
        echo "FAIL comptimedispatch/$label: build unexpectedly succeeded on an unhandled type" >&2; fail=1; return
    fi
    if ! printf '%s' "$out" | grep -qF -- "$want"; then
        echo "FAIL comptimedispatch/$label: missing the \$error diagnostic; got:" >&2
        printf '%s\n' "$out" >&2; fail=1; return
    fi
    echo "PASS comptimedispatch/$label: statement-position \$error fails the build with its message"
}

build_and_run clean     "clean no-cast dispatch"
expect_error  unhandled "statement-position \$error" "$ERROR_DIAG"

exit "$fail"
