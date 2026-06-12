#!/usr/bin/env bash
# module-resolution integration test: the `[project] module` bare project-id
# import surface (#1326).
#
# proves end to end that a one-segment `use`/`fwd` path equal to a resolvable
# project id resolves to that project's declared module — and that the loud
# failures fire: a bare import of a module-less project, a declared-but-dangling
# module path (imported or not), and a project module that re-exports its own id.
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

fail=0

# stage <name> — copy a fixture into the work dir and link the vendored std.
stage() {
    cp -r "$HERE/$1" "$WORK/$1"
    if grep -q 'mach-std' "$WORK/$1/mach.toml"; then
        mkdir -p "$WORK/$1/dep"
        ln -s "$STD" "$WORK/$1/dep/mach-std"
    fi
}

# expect_run <desc> <expected-exit> <project> — build and run a bin, checking exit.
expect_run() {
    local desc="$1"; local want="$2"; local proj="$3"
    if ! ( cd "$WORK/$proj" && "$MACH" build . ) >/dev/null 2>&1; then
        echo "FAIL module: $desc — build failed" >&2; fail=1; return
    fi
    local bin="$WORK/$proj/out/linux/debug/bin/$proj"
    if [ ! -x "$bin" ]; then
        echo "FAIL module: $desc — binary not produced at $bin" >&2; fail=1; return
    fi
    set +e
    "$bin"; local got=$?
    set -e
    if [ "$got" -ne "$want" ]; then
        echo "FAIL module: $desc — exit $got, expected $want" >&2; fail=1; return
    fi
    echo "PASS module: $desc"
}

# expect_build_err <desc> <project> <substring> — the build must fail and its
# stderr must contain <substring>.
expect_build_err() {
    local desc="$1"; local proj="$2"; local want="$3"
    set +e
    local out
    out="$( cd "$WORK/$proj" && "$MACH" build . 2>&1 )"
    local rc=$?
    set -e
    if [ "$rc" -eq 0 ]; then
        echo "FAIL module: $desc — build unexpectedly succeeded" >&2; fail=1; return
    fi
    if ! printf '%s' "$out" | grep -qF "$want"; then
        echo "FAIL module: $desc — message missing '$want'; got: $out" >&2; fail=1; return
    fi
    echo "PASS module: $desc"
}

stage ok
stage nomodule
stage dangling
stage selfimport

# a bare `use gfx;` resolves to gfx's [project].module (value() -> 7); `use shelf;`
# loads shelf, whose bare-id `fwd gfx;` must also resolve or the build fails.
expect_run "bare-id use + fwd resolve a project module" 7 ok

# a bare import of a module-less dependency names the fix.
expect_build_err "module-less project import errors" nomodule \
    "project 'plain' declares no module"

# a declared-but-dangling module fails at build start though nothing imports it.
expect_build_err "dangling [project].module fails at build start" dangling \
    "[project].module 'ghost.mach' names no file"

# a project module re-exporting its own id trips circular-module detection.
expect_build_err "self-fwd of the project id is circular" selfimport \
    "circular module dependency"

exit "$fail"
