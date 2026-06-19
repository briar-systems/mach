#!/usr/bin/env bash
# IEEE-754 NaN float-comparison regression test (issue #1446): build the `app`
# project with the freshly-built compiler and run it. the app asserts that every
# ordered float relation (`<` `<=` `>` `>=` `==`) is false when an operand is NaN and
# that `!=` is true — for f32 and f64, in both operand orders — exiting 0 only when
# every result is value-correct (a wrong comparison exits with its check id).
#
# the pre-fix x86_64 backend lowered a float compare to one unsigned SETcc after
# UCOMISD and so miscompiled the unordered case; the native (x86_64) half therefore
# fails before the fix and passes after. the aarch64 half cross-builds the same app
# and runs it under qemu to prove the two architectures agree on the IEEE-754 result;
# it is skipped when qemu-aarch64 is unavailable (mirroring aarch64run).
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

# x86_64 (native): the architecture the fix changes — must build, run, and pass.
if ! ( cd "$WORK/app" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "FAIL nancmp: x86_64 build failed" >&2
    exit 1
fi

set +e
"$WORK/app/out/linux/bin/nancmp"
code=$?
set -e

if [ "$code" -ne 0 ]; then
    echo "FAIL nancmp: x86_64 NaN comparison check $code produced a non-IEEE-754 result" >&2
    exit 1
fi
echo "PASS nancmp: x86_64 NaN comparisons are IEEE-754-correct"

# aarch64 (under qemu): proves the two architectures agree on the IEEE-754 result.
# built at -O0 so the comparisons are not optimized away. skipped without qemu.
RUNNER="$(command -v qemu-aarch64 || command -v qemu-aarch64-static || true)"
if [ -z "$RUNNER" ]; then
    echo "SKIP nancmp: 'qemu-aarch64' not available to run the aarch64 binary"
    exit 0
fi

if ! ( cd "$WORK/app" && "$MACH" build . --target linux-arm64 -O0 ) >/dev/null 2>&1; then
    echo "FAIL nancmp: aarch64 cross-build failed" >&2
    exit 1
fi

EXE="$WORK/app/out/linux-arm64/bin/nancmp"
test -f "$EXE" || { echo "error: aarch64 binary not produced at $EXE" >&2; exit 1; }

set +e
"$RUNNER" "$EXE" >/dev/null 2>&1
code=$?
set -e

if [ "$code" -ne 0 ]; then
    echo "FAIL nancmp: aarch64 NaN comparison check $code disagrees with IEEE-754" >&2
    exit 1
fi
echo "PASS nancmp: aarch64 NaN comparisons agree with x86_64 (IEEE-754)"
exit 0
