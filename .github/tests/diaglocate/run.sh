#!/usr/bin/env bash
# Diagnostic-location regression (#1046). Several middle-end lowering errors used
# to propagate a span-less string with a `lower:` breadcrumb and no file:line:col.
# They now file a structured diagnostic at the offending span and return a
# `lower:reported` sentinel the driver suppresses, so the user sees exactly one
# located, name-bearing message.
#
#   asmlocal — an inline-asm `{name}` reference to a name with no in-scope stack
#   local. the build must fail with a diagnostic that (a) names the offending
#   local and (b) is located on the asm body (main.mach:line:col), never the old
#   span-less "lower: unknown local in inline asm '{name}' reference".
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
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

# vendor <app-dir>: copy the app into the temp dir and symlink the vendored std.
vendor() {
    local app="$1"
    local dst="$WORK/$app"
    cp -r "$HERE/$app" "$dst"
    mkdir -p "$dst/dep"
    ln -s "$STD" "$dst/dep/mach-std"
}

# expect_located <app-dir> <substr>: build the app and require the build to FAIL
# with a diagnostic that contains <substr> AND is located on main.mach with a
# file:line:col prefix.
expect_located() {
    local app="$1"; local want="$2"
    vendor "$app"
    local dst="$WORK/$app"
    local out
    if out="$( cd "$dst" && "$MACH" build . 2>&1 )"; then
        echo "FAIL diaglocate/$app: build unexpectedly succeeded" >&2; fail=1; return
    fi
    if ! printf '%s' "$out" | grep -qF -- "$want"; then
        echo "FAIL diaglocate/$app: diagnostic missing '$want'; got:" >&2
        printf '%s\n' "$out" >&2; fail=1; return
    fi
    if ! printf '%s' "$out" | grep -qE 'main\.mach:[0-9]+:[0-9]+:'; then
        echo "FAIL diaglocate/$app: diagnostic missing file:line:col; got:" >&2
        printf '%s\n' "$out" >&2; fail=1; return
    fi
    echo "PASS diaglocate/$app: a formerly span-less lowering error is located and named"
}

expect_located asmlocal "unknown local 'nosuch' in inline asm"

exit "$fail"
