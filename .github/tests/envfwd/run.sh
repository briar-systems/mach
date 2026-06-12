#!/usr/bin/env bash
# envfwd integration test: `mach test` and `mach run` forward the parent
# environment to spawned user programs (mach-std#197).
#
# the app's test and main both assert MACH_ENVFWD_PROBE arrives with the
# value exported here; under a nil-envp harness both would fail.
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

cp -r "$HERE/app" "$WORK/app"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"
APP="$WORK/app"

export MACH_ENVFWD_PROBE=octarine
fail=0

if ( cd "$APP" && "$MACH" test . --filter envfwd ) >"$WORK/test.out" 2>&1; then
    echo "PASS envfwd: mach test forwards the environment"
else
    echo "FAIL envfwd: mach test child did not inherit MACH_ENVFWD_PROBE" >&2
    cat "$WORK/test.out" >&2
    fail=1
fi

if ( cd "$APP" && "$MACH" run . ) >"$WORK/run.out" 2>&1; then
    echo "PASS envfwd: mach run forwards the environment"
else
    echo "FAIL envfwd: mach run child did not inherit MACH_ENVFWD_PROBE" >&2
    cat "$WORK/run.out" >&2
    fail=1
fi

exit "$fail"
