#!/usr/bin/env bash
# absout integration test: verify that an absolute -o path is honored verbatim
# and that a relative -o path is still resolved against the project root.
#
# regression for issue #1340: mach build . -o /abs/path produced the binary at
# <project>/abs/path (the leading slash was eaten by the unconditional join with
# the project root). an absolute -o must be used as-is; relative -o must still
# resolve under the project root.
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

cp -r "$HERE/app" "$WORK/app"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"
APP="$WORK/app"

fail=0

# absolute -o: the binary must appear at exactly the given absolute path.
ABS_OUT="$WORK/abs-out/mach-absout"
mkdir -p "$WORK/abs-out"
if ! ( cd "$APP" && "$MACH" build . -o "$ABS_OUT" ) >/dev/null 2>&1; then
    echo "FAIL absout: absolute -o build failed" >&2
    fail=1
else
    if [ -x "$ABS_OUT" ]; then
        echo "PASS absout: absolute -o writes to the exact absolute path"
    else
        echo "FAIL absout: binary not at absolute path '$ABS_OUT'" >&2
        # diagnose: show where it actually landed
        find "$APP/out" -type f 2>/dev/null | head -5 | while read -r f; do
            echo "  found: $f" >&2
        done
        fail=1
    fi
fi

# relative -o: the binary must appear under the project root.
REL_OUT="rel-out/mach-absout"
rm -rf "$APP/out" "$APP/rel-out"
if ! ( cd "$APP" && "$MACH" build . -o "$REL_OUT" ) >/dev/null 2>&1; then
    echo "FAIL absout: relative -o build failed" >&2
    fail=1
else
    if [ -x "$APP/$REL_OUT" ]; then
        echo "PASS absout: relative -o resolves under the project root"
    else
        echo "FAIL absout: binary not at relative path '$APP/$REL_OUT'" >&2
        find "$APP" -type f -name "mach-absout" 2>/dev/null | head -5 | while read -r f; do
            echo "  found: $f" >&2
        done
        fail=1
    fi
fi

exit "$fail"
