#!/usr/bin/env bash
# AAPCS64 HFA/HVA float-aggregate passing (#1174): passes structs of 1–4 same-typed
# floats by value through non-inlined calls and returns them, including >16-byte HFAs
# (3×/4× f64) that ride V registers and the all-or-memory spill when the V bank cannot
# fit the run. the aarch64 cross-build is the case the HFA classifier targets — run
# under qemu-aarch64, main exits 0 only when every placement round-trips. the SAME
# source is also built and run natively for x86_64, where these float structs ride the
# SysV SSE-eightbyte path: a cross-check that the shared lowering and x64 emission are
# unperturbed by the aarch64-only change.
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

# x86_64 cross-check: build natively at -O0 (so the float-struct calls are real, not
# inlined) and run on the host. these structs are not AAPCS64 HFAs on SysV — they ride
# the SSE-eightbyte path — so a correct exit proves the shared lowering still places
# them right under the unchanged x64 emitter.
if ! ( cd "$WORK/app" && "$MACH" build . --target linux -O0 ) >/dev/null 2>&1; then
    echo "FAIL aarch64hfa: x86_64 native build failed" >&2
    exit 1
fi
X64BIN="$WORK/app/out/linux/bin/aarch64hfa"
test -x "$X64BIN" || { echo "error: x86_64 binary not produced at $X64BIN" >&2; exit 1; }
set +e
"$X64BIN" >/dev/null 2>&1
x64code=$?
set -e
if [ "$x64code" -ne 0 ]; then
    echo "FAIL aarch64hfa: x86_64 SysV float-struct passing miscompiled (exit $x64code)" >&2
    exit 1
fi

# aarch64: cross-build at -O0 so the HFA placement at the call boundary is exercised.
if ! ( cd "$WORK/app" && "$MACH" build . --target linux-arm64 -O0 ) >/dev/null 2>&1; then
    echo "FAIL aarch64hfa: aarch64 cross-build failed" >&2
    exit 1
fi
ARMBIN="$WORK/app/out/linux-arm64/bin/aarch64hfa"
test -f "$ARMBIN" || { echo "error: aarch64 binary not produced at $ARMBIN" >&2; exit 1; }

# the HFA placement is only observable by executing the aarch64 binary; without
# qemu-aarch64 there is nothing to run it, so the run is skipped (the cross-build
# above still verifies the emitter accepts the HFA path).
RUNNER="$(command -v qemu-aarch64 || command -v qemu-aarch64-static || true)"
if [ -z "$RUNNER" ]; then
    echo "PASS aarch64hfa: x86_64 SysV float-struct cross-check passed; aarch64 cross-build OK (SKIP qemu run: 'qemu-aarch64' not available)"
    exit 0
fi

set +e
"$RUNNER" "$ARMBIN" >/dev/null 2>&1
code=$?
set -e

if [ "$code" -eq 0 ]; then
    echo "PASS aarch64hfa: HFA/HVA float aggregates pass and return in V registers under qemu (x86_64 SysV cross-check also passed)"
    exit 0
fi

echo "FAIL aarch64hfa: AAPCS64 HFA passing miscompiled under qemu (exit $code; see main.mach for the per-check codes)" >&2
exit 1
