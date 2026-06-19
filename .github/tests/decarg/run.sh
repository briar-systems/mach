#!/usr/bin/env bash
# decorator-argument resolution test (#1491, fast-follow #1476): bind_decorator_args
# (resolve.mach) walks the comptime expression in each backtick decorator, so an
# unresolved identifier there must surface a name-resolution error rather than be
# silently dropped. two placements are covered:
#   unresolved — a top-level decl whose decorator names an undeclared identifier.
#   branch     — the same, on a decl inside a TAKEN `$if (...)` decl-scope branch,
#                proving the resolver descends into comptime decl branches and binds
#                the decorators it finds there.
# each build must FAIL with a located `unresolved identifier` naming the offender.
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
    echo "error: vendored std not found at $STD (run: mach dep pull)" >&2
    exit 2
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

fail=0

# vendor <app-dir>: copy the app into the temp dir and symlink the vendored std.
vendor() {
    local app="$1" dst="$WORK/$1"
    cp -r "$HERE/$app" "$dst"
    mkdir -p "$dst/dep"
    ln -s "$STD" "$dst/dep/mach-std"
}

# expect_unresolved <app-dir> <name>: build the app and require it to FAIL with a
# diagnostic that reports <name> as an `unresolved identifier`, located on main.mach.
expect_unresolved() {
    local app="$1" name="$2" out dst="$WORK/$1"
    vendor "$app"
    if out="$( cd "$dst" && "$MACH" build . 2>&1 )"; then
        echo "FAIL decarg/$app: build unexpectedly succeeded (the decorator arg was not resolved)" >&2
        fail=1; return
    fi
    if ! printf '%s' "$out" | grep -qF "unresolved identifier"; then
        echo "FAIL decarg/$app: diagnostic missing 'unresolved identifier'; got:" >&2
        printf '%s\n' "$out" >&2; fail=1; return
    fi
    if ! printf '%s' "$out" | grep -qF "$name"; then
        echo "FAIL decarg/$app: diagnostic did not name '$name'; got:" >&2
        printf '%s\n' "$out" >&2; fail=1; return
    fi
    if ! printf '%s' "$out" | grep -qE 'main\.mach:[0-9]+:[0-9]+:'; then
        echo "FAIL decarg/$app: diagnostic missing file:line:col; got:" >&2
        printf '%s\n' "$out" >&2; fail=1; return
    fi
    echo "PASS decarg/$app: an unresolved decorator-arg identifier is a located name-resolution error"
}

expect_unresolved unresolved NoSuchThing
expect_unresolved branch     NoSuchThing

exit "$fail"
