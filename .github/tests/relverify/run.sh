#!/usr/bin/env bash
# release-pipeline IR-verification regression (issue #1252): build the `app`
# library with `--release --verify-ir`. The release pipeline used to emit IR the
# aggregate-representation verifier rejected on almost any program (inline's
# single-return result phi over an aggregate, plus a too-strict post-inline
# reachability gate), so `mach build . --release --verify-ir` failed on a
# hello-world. The app exercises scalar and aggregate single-return inlining,
# constant propagation across an inline boundary, and a dead inlined-argument
# computation; the build must pass at both -O0 and --release under --verify-ir.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
# absolutize a path argument: the suite cds into a temp dir, so a relative
# compiler path would break after the first cd.
case "$MACH" in */*) MACH="$(cd "$(dirname "$MACH")" && pwd)/$(basename "$MACH")";; esac
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
cp -r "$HERE/app" "$WORK/"

if ! ( cd "$WORK/app" && "$MACH" build . --verify-ir ) >/dev/null 2>&1; then
    echo "FAIL relverify: -O0 --verify-ir build failed" >&2
    exit 1
fi

if ! ( cd "$WORK/app" && "$MACH" build . --release --verify-ir ) >/dev/null 2>&1; then
    echo "FAIL relverify: --release --verify-ir build failed (inline aggregate phi / post-inline gate)" >&2
    exit 1
fi

echo "PASS relverify: release pipeline passes --verify-ir on inlined aggregate/constant-propagation paths"
exit 0
