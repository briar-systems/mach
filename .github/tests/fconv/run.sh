#!/usr/bin/env bash
# integer-to-float conversion regression test (issue #1255): build the `app`
# project with the freshly-built compiler and run it. the app self-checks u32 /
# u16 / u8 / i8 / i16 / i32 / u64 -> f32 / f64 conversions across boundary values
# (0xFFFFFFFF, 0x80000000, 2^63, 2^64-1, ...) and exits 0 only when every result is
# correct; any miscompiled conversion exits with a non-zero check id.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
# absolutize a path argument: the suite cds into a temp dir, so a relative
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

cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"

if ! ( cd "$WORK/app" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "FAIL fconv: build failed" >&2
    exit 1
fi

set +e
"$WORK/app/out/linux/bin/fconvapp"
code=$?
set -e

if [ "$code" -ne 0 ]; then
    echo "FAIL fconv: conversion check $code produced a wrong result" >&2
    exit 1
fi

echo "PASS fconv: integer-to-float conversions correct across boundary values"
exit 0
