#!/usr/bin/env bash
# Never-value teaching diagnostics (#1348, the #1343 silent-poison class). A
# symbol that resolves cleanly but can never carry a value type — a record,
# union, or def type name (local or imported), a generic type parameter, a
# module alias — and a member access on a void expression used to pass sema
# with ZERO diagnostics, surfacing only as link `undefined symbol` or span-less
# `lower:` errors. Each misuse must now fail the build with a located teaching
# diagnostic, and every adjacent legitimate construct must keep building.
#
#   local    — local rec/uni/def names, a generic parameter, and a void-object
#              member access, all in value position in one module.
#   imported — a module alias, an imported record, and an imported def alias in
#              value position, reached through `use plib.lib`.
#   ok       — the positives: record literal, function reference, `$size_of(T)`,
#              and qualified value access still build and run (exit 37).
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

cp -r "$HERE/plib" "$WORK/plib"

# vendor <app-dir>: copy the app into the temp dir and symlink its deps.
vendor() {
    local app="$1"
    local dst="$WORK/$app"
    cp -r "$HERE/$app" "$dst"
    mkdir -p "$dst/dep"
    ln -s "$STD" "$dst/dep/mach-std"
    ln -s "$WORK/plib" "$dst/dep/plib"
}

# expect_located <app-dir> <substr>: the app's build output (a failing build)
# must contain <substr> located on main.mach with a file:line:col prefix.
expect_located() {
    local app="$1"; local want="$2"
    local out="$3"
    if ! printf '%s' "$out" | grep -qF -- "$want"; then
        echo "FAIL valuepos/$app: diagnostic missing '$want'; got:" >&2
        printf '%s\n' "$out" >&2; fail=1; return
    fi
    if ! printf '%s' "$out" | grep -F -- "$want" | grep -qE 'main\.mach:[0-9]+:[0-9]+:'; then
        echo "FAIL valuepos/$app: diagnostic '$want' missing file:line:col; got:" >&2
        printf '%s\n' "$out" >&2; fail=1; return
    fi
    echo "PASS valuepos/$app: located '$want'"
}

# build_expect_fail <app-dir>: build the app, requiring failure; echoes output.
# runs in a command substitution, so the caller must set fail=1 on a non-zero
# return (a build that unexpectedly succeeded).
build_expect_fail() {
    local app="$1"
    vendor "$app"
    local out
    if out="$( cd "$WORK/$app" && "$MACH" build . 2>&1 )"; then
        echo "FAIL valuepos/$app: build unexpectedly succeeded" >&2
        return 1
    fi
    printf '%s' "$out"
}

if out="$(build_expect_fail local)"; then
    expect_located local "is a record type, not a value" "$out"
    expect_located local "is a union type, not a value" "$out"
    expect_located local "is a type alias, not a value" "$out"
    expect_located local "is a generic type parameter, not a value" "$out"
    expect_located local "a void expression has no members" "$out"
else
    fail=1
fi

if out="$(build_expect_fail imported)"; then
    expect_located imported "a module alias is not a value" "$out"
    expect_located imported "is a record type, not a value" "$out"
    expect_located imported "is a type alias, not a value" "$out"
else
    fail=1
fi

vendor ok
if ! ( cd "$WORK/ok" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "FAIL valuepos/ok: positive build failed" >&2
    fail=1
else
    bin="$WORK/ok/out/linux/bin/ok"
    [ -x "$bin" ] || bin="$WORK/ok/out/linux/debug/bin/ok"
    if [ ! -x "$bin" ]; then
        echo "FAIL valuepos/ok: binary not produced" >&2
        fail=1
    else
        set +e
        "$bin"; got=$?
        set -e
        if [ "$got" -ne 37 ]; then
            echo "FAIL valuepos/ok: exit $got, expected 37" >&2
            fail=1
        else
            echo "PASS valuepos/ok: record literal, fn reference, \$size_of(T), and qualified value access all build and run"
        fi
    fi
fi

exit "$fail"
