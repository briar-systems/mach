#!/usr/bin/env bash
# comptime roots integration test (#1217, #1249). two halves:
#
#   folding — the `$project.*` / `$project.target.*` / `$bin.*` roots must fold to each
#   manifest's declared values. two apps with distinct declared values each
#   assert every root against their own manifest and exit 0 only when all match;
#   a mismatch returns the drifted root's id. this is the end-to-end proof that
#   the driver feeds the roots from the manifest model and the selected build
#   unit, not from hardcoded values.
#
#   rejection — a standalone bare `$ident` is none of the closed comptime
#   channel shapes (#1249); the build must FAIL with the one teaching diagnostic,
#   never fold. asserted in both a `$if` gate and value position to prove sema's
#   two surfaces defer to the single rule owned by comptime.eval.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

# the exact teaching diagnostic comptime.eval emits for a bare `$ident`
# (comptime.COMPTIME_BARE_IDENT_MSG); both rejection apps must surface it verbatim.
BARE_IDENT_DIAG='comptime parameters are referenced without `$`; comptime paths are rooted: `$mach`, `$project`, `$target`, `$bin`'

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

# build_and_run <app-dir> <label>: vendor std, build, run, require exit 0. the
# app's own main returns the drifted root's id on a fold mismatch.
build_and_run() {
    local app="$1"; local label="$2"
    local dst="$WORK/$app"
    cp -r "$HERE/$app" "$dst"
    mkdir -p "$dst/dep"
    ln -s "$STD" "$dst/dep/mach-std"
    if ! ( cd "$dst" && "$MACH" build . ) >/dev/null 2>&1; then
        echo "FAIL comptimeroots/$label: build failed" >&2; fail=1; return
    fi
    local bin
    bin="$(find "$dst/out" -type f -path '*/bin/*' | head -n1)"
    if [ -z "$bin" ]; then
        echo "FAIL comptimeroots/$label: no binary produced" >&2; fail=1; return
    fi
    set +e
    "$bin"; local code=$?
    set -e
    if [ "$code" -ne 0 ]; then
        echo "FAIL comptimeroots/$label: root id $code folded to the wrong value" >&2; fail=1; return
    fi
    echo "PASS comptimeroots/$label: \$project/\$target/\$bin roots fold to the manifest values"
}

# expect_reject <app-dir> <label> <diagnostic>: vendor std, build, and require
# the build to FAIL with `diagnostic` present verbatim on stderr. proves a bare
# `$ident` is rejected by the one comptime rule rather than folded or compiled.
expect_reject() {
    local app="$1"; local label="$2"; local want="$3"
    local dst="$WORK/$app"
    cp -r "$HERE/$app" "$dst"
    mkdir -p "$dst/dep"
    ln -s "$STD" "$dst/dep/mach-std"
    local out
    if out="$( cd "$dst" && "$MACH" build . 2>&1 )"; then
        echo "FAIL comptimeroots/$label: build unexpectedly succeeded for a bare \$ident" >&2; fail=1; return
    fi
    if ! printf '%s' "$out" | grep -qF -- "$want"; then
        echo "FAIL comptimeroots/$label: missing the teaching diagnostic; got:" >&2
        printf '%s\n' "$out" >&2; fail=1; return
    fi
    echo "PASS comptimeroots/$label: bare \$ident rejected with the teaching diagnostic"
}

build_and_run app     "manifest with profiles"
build_and_run altapp  "alternate declared values"
build_and_run machabi "\$mach.abi tag namespace"

expect_reject bareident_gate  "bare \$ident gate"  "$BARE_IDENT_DIAG"
expect_reject bareident_value "bare \$ident value" "$BARE_IDENT_DIAG"

exit "$fail"
