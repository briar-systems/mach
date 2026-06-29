#!/usr/bin/env bash
# cross-compile the freestanding aarch64 fixture, byte-verify the result with the llvm
# tools, then run it under qemu-aarch64 and assert its exit code. proves the arm64
# backend encodes a spilled-float load/store as the scalar-FP slot form (ldr/str Sd|Dd)
# rather than aborting -- the #1737 fix on the arm64 encoder.
#
# usage: verify.sh [path-to-mach]   (defaults to `mach` on PATH)
#
# requires: llvm-readelf, llvm-objdump (unversioned or `-NN` suffixed) and qemu-aarch64
# (or qemu-aarch64-static) for the exit-code run.
set -euo pipefail

# the exit code the fixture returns: fps_in holds (i & 1) over a 40-wide f64 buffer the
# allocator must spill, so the sum is the twenty odd indices in 0..39 = 20. the qemu e2e
# asserts the exact code, so a regression in the spilled-float load/store encoding (an
# abort, or an integer ld/st of the float bits) changes or breaks it.
expect_code=20

mach="${1:-mach}"
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$here"

fail() { echo "FAIL: $1" >&2; exit 1; }

# resolve an llvm tool by its unversioned name, falling back to the highest `-NN`
# suffixed variant on PATH (ubuntu ships e.g. llvm-objdump-18).
resolve_tool() {
    local base="$1" cand
    if command -v "$base" >/dev/null 2>&1; then echo "$base"; return 0; fi
    cand="$(compgen -c "$base-" | sort -V | tail -n 1)"
    if [ -n "$cand" ] && command -v "$cand" >/dev/null 2>&1; then echo "$cand"; return 0; fi
    return 1
}

readelf="$(resolve_tool llvm-readelf)" || fail "llvm-readelf not found"
objdump="$(resolve_tool llvm-objdump)" || fail "llvm-objdump not found"

target="linux-arm64"
exe="out/$target/a64probe"

echo "cross-compiling fixture for $target with $mach"
rm -rf out
"$mach" build . --target "$target" --profile debug

echo "verifying elf header of $exe"
hdr="$("$readelf" -h "$exe")"
grep -q 'Class:.*ELF64'    <<< "$hdr" || fail "exe is not ELFCLASS64"
grep -q 'Machine:.*AArch64' <<< "$hdr" || fail "exe e_machine is not EM_AARCH64"

# the spilled float must encode as the scalar-FP slot load/store (ldr/str Sd|Dd off a
# frame base), not an integer ld/st of the float bits and not an encode abort (the build
# above would have failed). assert both an FP slot load and store appear.
echo "verifying spilled-float FP slot load/store in $exe"
dis="$("$objdump" -d "$exe")"
grep -qiE '\bldr\s+d[0-9]+,' <<< "$dis" || fail "no FP-register load (ldr Dd) found -- spilled float not encoded as scalar FP"
grep -qiE '\bstr\s+d[0-9]+,' <<< "$dis" || fail "no FP-register store (str Dd) found -- spilled float not encoded as scalar FP"

echo "running $exe under qemu-aarch64"
qemu="$(command -v qemu-aarch64 || command -v qemu-aarch64-static || true)"
[ -n "$qemu" ] || fail "qemu-aarch64 not found"
set +e
"$qemu" "$exe"
code=$?
set -e
[ "$code" -eq "$expect_code" ] || fail "exit code $code, expected $expect_code"

echo "OK: arm64 backend encodes spilled-float load/store as scalar FP and runs to exit code $expect_code"
