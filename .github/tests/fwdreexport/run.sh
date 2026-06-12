#!/usr/bin/env bash
# fwd re-export integration test: same-package sibling re-exports in a library
# entrypoint (octalide/mach-std#186, fixed by #1185 / PR #1186) and consumer
# chains through the published surface (#1343).
#
# proves that a standalone `mach build .` of a library whose entrypoint only
# `fwd`-re-exports its own sibling submodules succeeds — the mach-std lib.mach
# shape that used to fail with "re-exported module is not in the dep set" when
# the library was the root project (it always worked as a dependency) — and
# that a consumer can reach every re-export shape: a symbol re-export, module
# re-exports chained in expression position (including a nested-FQN alias),
# and a second library's fwd-of-a-fwd of both.
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
cp -r "$HERE/outer" "$WORK/outer"
cp -r "$HERE/app" "$WORK/app"
mkdir -p "$WORK/outer/dep" "$WORK/app/dep"
ln -s "$WORK/demo" "$WORK/outer/dep/demo"
ln -s "$WORK/demo" "$WORK/app/dep/demo"
ln -s "$WORK/outer" "$WORK/app/dep/outer"
ln -s "$STD" "$WORK/app/dep/mach-std"

# the library builds as the ROOT project: every module it contains is in its
# own dep set, so the entrypoint's sibling re-exports must resolve.
if ( cd "$WORK/demo" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "PASS fwdreexport: standalone lib build with sibling fwd re-exports"
else
    echo "FAIL fwdreexport: standalone lib build with sibling fwd re-exports — build failed" >&2
    fail=1
fi

# a library whose entrypoint only re-exports ANOTHER library's re-exports
# builds standalone too: each fwd hop preserves the leaf's resolution (#1343).
if ( cd "$WORK/outer" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "PASS fwdreexport: standalone lib build with fwd-of-a-fwd re-exports"
else
    echo "FAIL fwdreexport: standalone lib build with fwd-of-a-fwd re-exports — build failed" >&2
    fail=1
fi

# the libraries consumed as dependencies: the consumer reaches every target
# only through re-exports — the symbol re-export (7), the module re-exports
# chained in expression position (7 + 35), and the fwd-of-a-fwd surface
# (7 + 7) — totalling 63.
if ! ( cd "$WORK/app" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "FAIL fwdreexport: consumer build through fwd re-exports — build failed" >&2
    fail=1
else
    bin="$WORK/app/out/linux/debug/bin/app"
    if [ ! -x "$bin" ]; then
        echo "FAIL fwdreexport: consumer build through fwd re-exports — binary not produced at $bin" >&2
        fail=1
    else
        set +e
        "$bin"; got=$?
        set -e
        if [ "$got" -ne 63 ]; then
            echo "FAIL fwdreexport: consumer calls through fwd re-exports — exit $got, expected 63" >&2
            fail=1
        else
            echo "PASS fwdreexport: consumer calls through symbol, module-chain, and fwd-of-a-fwd re-exports"
        fi
    fi
fi

exit "$fail"
