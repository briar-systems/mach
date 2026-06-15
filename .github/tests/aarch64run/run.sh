#!/usr/bin/env bash
# aarch64 f64 memory-load run smoke (#1459): cross-builds a tiny aarch64-linux
# program that loads f64s from an array element and a struct field — each feeding
# an FP op — and runs it under qemu-aarch64. on the pre-fix compiler those loads
# used the general-purpose register path, so the FP op read a stale vector
# register and the result was garbage; main returns 0 only when both loads read
# correctly (1 = array-element misread, 2 = struct-field misread).
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
    echo "error: vendored std not found at $STD (run: git submodule update --init)" >&2
    exit 2
fi

# the test runs an aarch64 binary; without qemu-aarch64 there is nothing to run it.
RUNNER="$(command -v qemu-aarch64 || command -v qemu-aarch64-static || true)"
if [ -z "$RUNNER" ]; then
    echo "SKIP aarch64run: 'qemu-aarch64' not available to run the aarch64 binary"
    exit 0
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"

# build at -O0 so the loads are not optimized away — the f64 memory-load path
# must actually be exercised.
if ! ( cd "$WORK/app" && "$MACH" build . -O0 ) >/dev/null 2>&1; then
    echo "FAIL aarch64run: aarch64 cross-build failed" >&2
    exit 1
fi

EXE="$WORK/app/out/linux-arm64/bin/aarch64run"
test -f "$EXE" || { echo "error: aarch64 binary not produced at $EXE" >&2; exit 1; }

set +e
"$RUNNER" "$EXE" >/dev/null 2>&1
code=$?
set -e

if [ "$code" -eq 0 ]; then
    echo "PASS aarch64run: f64 array-element and struct-field loads read correctly under qemu"
    exit 0
fi

echo "FAIL aarch64run: f64 memory-load miscompiled (exit $code: 1=array-element, 2=struct-field)" >&2
exit 1
