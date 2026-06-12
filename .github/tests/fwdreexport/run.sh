#!/usr/bin/env bash
# fwd re-export integration test: same-package sibling re-exports in a library
# entrypoint (octalide/mach-std#186, fixed by #1185 / PR #1186).
#
# proves that a standalone `mach build .` of a library whose entrypoint only
# `fwd`-re-exports its own sibling submodules succeeds — the mach-std lib.mach
# shape that used to fail with "re-exported module is not in the dep set" when
# the library was the root project (it always worked as a dependency) — and
# that a consumer can call through the entrypoint's symbol re-export.
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

cp -r "$HERE/demo" "$WORK/demo"
cp -r "$HERE/app" "$WORK/app"
mkdir -p "$WORK/app/dep"
ln -s "$WORK/demo" "$WORK/app/dep/demo"
ln -s "$STD" "$WORK/app/dep/mach-std"

# the library builds as the ROOT project: every module it contains is in its
# own dep set, so the entrypoint's sibling re-exports must resolve.
if ( cd "$WORK/demo" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "PASS fwdreexport: standalone lib build with sibling fwd re-exports"
else
    echo "FAIL fwdreexport: standalone lib build with sibling fwd re-exports — build failed" >&2
    fail=1
fi

# the same library consumed as a dependency: the consumer reaches `answer`
# only through the entrypoint's symbol re-export.
if ! ( cd "$WORK/app" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "FAIL fwdreexport: consumer build through fwd re-export — build failed" >&2
    fail=1
else
    bin="$WORK/app/out/linux/debug/bin/app"
    if [ ! -x "$bin" ]; then
        echo "FAIL fwdreexport: consumer build through fwd re-export — binary not produced at $bin" >&2
        fail=1
    else
        set +e
        "$bin"; got=$?
        set -e
        if [ "$got" -ne 7 ]; then
            echo "FAIL fwdreexport: consumer call through fwd re-export — exit $got, expected 7" >&2
            fail=1
        else
            echo "PASS fwdreexport: consumer call through fwd symbol re-export"
        fi
    fi
fi

exit "$fail"
