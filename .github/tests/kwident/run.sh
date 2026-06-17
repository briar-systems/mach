#!/usr/bin/env bash
# Contextual statement keywords used as identifiers (#1458). `cnt`/`brk` are the
# continue/break keywords only in their bare `cnt;`/`brk;` form; the same word
# used as a name (`var cnt`, `cnt = g()`, `ret cnt`) must compile and run as an
# ordinary identifier. before the fix `cnt = g();` misparsed as a continue
# statement and the build failed with "expected ';' after 'cnt'". the app exits
# 0 only when every position agrees and bare cnt/brk still drive control flow.
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
        echo "FAIL kwident/$app: build failed" >&2; fail=1; return
    fi
    local bin
    bin="$(find "$dst/out" -type f -path '*/bin/*' | head -n1)"
    if [ -z "$bin" ]; then
        echo "FAIL kwident/$app: no binary produced" >&2; fail=1; return
    fi
    set +e
    "$bin"; local code=$?
    set -e
    if [ "$code" -ne "$want" ]; then
        echo "FAIL kwident/$app: ran with exit $code, expected $want" >&2; fail=1; return
    fi
    echo "PASS kwident/$app: cnt/brk used as identifiers compile and run; bare cnt;/brk; still control flow"
}

build_and_run prog 0

exit "$fail"
