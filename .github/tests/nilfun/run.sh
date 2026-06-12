#!/usr/bin/env bash
# Explicit nil-init of fun-typed globals (#1369). nil is the null address and now
# coerces to any pointer-like type, function types included, in every position.
# Two halves:
#
#   fold — a fun-typed global initialised from nil, both bare (`= nil`) and
#   through the cast spelling (`= nil::F`), must compile and start null, and the
#   global must round-trip through a real function assignment, a call, and a
#   clear back to null. before #1369 the bare form failed sema with
#   "type mismatch: expected fun(u32), found *u8" and the cast form failed
#   lowering with "global initialiser must be a constant expression"; only
#   default-init worked. The app exits 0 only when every assertion holds.
#
#   reject — nil into a non-pointer global (a `u32`) must still fail the build
#   with a located type-mismatch diagnostic, proving the relaxation admits only
#   pointer-like targets, not arbitrary scalars.
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
        echo "FAIL nilfun/$app: build failed" >&2; fail=1; return
    fi
    local bin
    bin="$(find "$dst/out" -type f -path '*/bin/*' | head -n1)"
    if [ -z "$bin" ]; then
        echo "FAIL nilfun/$app: no binary produced" >&2; fail=1; return
    fi
    set +e
    "$bin"; local code=$?
    set -e
    if [ "$code" -ne "$want" ]; then
        echo "FAIL nilfun/$app: ran with exit $code, expected $want" >&2; fail=1; return
    fi
    echo "PASS nilfun/$app: fun-typed global nil-inits (bare and nil::F) fold, start null, and round-trip"
}

# expect_reject <app-dir>: build the app and require the build to FAIL with a
# located type-mismatch diagnostic (file:line:col on main.mach).
expect_reject() {
    local app="$1"
    vendor "$app"
    local dst="$WORK/$app"
    local out
    if out="$( cd "$dst" && "$MACH" build . 2>&1 )"; then
        echo "FAIL nilfun/$app: build unexpectedly succeeded for nil into a non-pointer global" >&2; fail=1; return
    fi
    if ! printf '%s' "$out" | grep -qF -- 'type mismatch'; then
        echo "FAIL nilfun/$app: diagnostic missing 'type mismatch'; got:" >&2
        printf '%s\n' "$out" >&2; fail=1; return
    fi
    if ! printf '%s' "$out" | grep -qE 'main\.mach:[0-9]+:[0-9]+:'; then
        echo "FAIL nilfun/$app: diagnostic missing file:line:col; got:" >&2
        printf '%s\n' "$out" >&2; fail=1; return
    fi
    echo "PASS nilfun/$app: nil into a non-pointer global is rejected with a located diagnostic"
}

build_and_run fold 0
expect_reject reject

exit "$fail"
