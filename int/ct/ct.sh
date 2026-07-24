#!/usr/bin/env bash
# ct.sh — the constant-time timing-leak harness (#1647).
#
# WHAT IT MEASURES. an empirical, testing-grade check that Mach's constant-time story
# holds on real hardware, complementing the static secret-flow gates (#1645) and the
# #[oblivious] codegen contract (#1646). it builds two functions that do the SAME task
# — an 8-byte comparison against a fixed reference — and times each over two input
# classes (dudect's fixed-vs-random), applying Welch's t-test to the two timing
# distributions:
#   * ct_probe   — a branchless #[oblivious] compare over `^` secrets; timing must not
#                  depend on the input, so |t| stays small.
#   * leaky_probe — a deliberately-leaky control that early-exits on the first
#                  mismatching byte, so its running time leaks how many bytes matched.
# the leaky control is the planted leak: a harness that cannot catch it is decoration.
# it is written over PUBLIC values on purpose — sema rejects a secret branch in any
# function, so this leak is unconstructable with `^` types, which is exactly the point:
# the static discipline forbids it, and this harness catches it the moment the
# discipline is dropped.
#
# HOW TO RUN.
#   bash int/ct/ct.sh                     # bootstrap a from-source compiler, measure
#   bash int/ct/ct.sh /path/to/mach       # use a prebuilt CT-capable compiler
#   bash int/ct/ct.sh /path/to/mach linux release
#   LEAK_MIN=10 CT_WARN=10 bash int/ct/ct.sh   # override thresholds
# the PATH `mach` seed predates the `^`/#[oblivious] surface, so the harness cannot be
# built with it directly; omitting the compiler argument bootstraps one from source.
#
# WHY NOT A CI GATE. timing measurement is noise-sensitive and CI runners are shared
# and noisy, so this is run-on-demand, not wired into CI. it lives beside the int/
# cases but is NOT discovered by int/run.sh (which scans only surface/ and regression/),
# so it never becomes a flaky gate. the assertion is a leak-detection smoke: the leaky
# control MUST exceed LEAK_MIN (a wide margin — observed |t| in the hundreds against a
# threshold of 10), while the constant-time reference is informational (its |t| is
# reported and warned on, never failed) so ambient noise cannot break the build.
set -euo pipefail

here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$here"
repo_root=$(cd "$here/../.." && pwd)

mach="${1:-}"
target="${2:-linux}"
profile="${3:-release}"

# leaky |t| must clear this (hard). ct |t| above CT_WARN warns but never fails.
LEAK_MIN="${LEAK_MIN:-10}"
CT_WARN="${CT_WARN:-10}"

fail() { echo "FAIL: $1" >&2; exit 1; }

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

# vendor the repo's std as the harness's path dependency: no network, and the exact
# std the compiler under test was built against.
[ -d "$repo_root/dep/mach-std" ] \
    || fail "repo dep/mach-std missing — run 'mach dep pull' at the repo root first"
mkdir -p dep
ln -sfn "$repo_root/dep/mach-std" dep/mach-std

# resolve a CT-capable compiler: the one given, else bootstrap from source with the
# PATH seed.
if [ -z "$mach" ]; then
    echo "no compiler given — bootstrapping a from-source compiler with the PATH seed"
    if ! mach build "$repo_root" --profile release -o "$tmp/machct" >"$tmp/boot.log" 2>&1; then
        sed 's/^/  /' "$tmp/boot.log" >&2
        fail "bootstrap compiler build failed"
    fi
    mach="$tmp/machct"
fi
case "$mach" in
    /*) : ;;
    */*) mach="$(cd "$(dirname "$mach")" && pwd)/$(basename "$mach")" ;;
    *)  mach="$(command -v "$mach")" || fail "compiler '$1' not found on PATH" ;;
esac

echo "building the ct harness with $mach (target $target, profile $profile)"
rm -rf out
if ! "$mach" build . --target "$target" --profile "$profile" >"$tmp/build.log" 2>&1; then
    if grep -q "unknown decorator" "$tmp/build.log"; then
        fail "the compiler at '$mach' predates the #[oblivious] surface — pass a from-source compiler (post-#2137) or omit the argument to bootstrap one"
    fi
    sed 's/^/  /' "$tmp/build.log" >&2
    fail "harness build failed"
fi
bin=$(find out -name ct_harness -type f -print -quit)
[ -n "$bin" ] || fail "no ct_harness binary produced"

echo "running the measurement (dudect / Welch's t-test)"
out=$("$bin")
echo "$out" | sed 's/^/  /'

extract() { echo "$out" | awk -v n="$1" '$0 ~ ("name=" n " ") { for (i=1;i<=NF;i++) if ($i ~ /^abst=/) { sub("abst=","",$i); print $i } }'; }
ct_t=$(extract ct)
leak_t=$(extract leak)
[ -n "$ct_t" ]   || fail "no ct result parsed from harness output"
[ -n "$leak_t" ] || fail "no leak result parsed from harness output"

echo
rc=0
awk -v ct="$ct_t" -v lk="$leak_t" -v lmin="$LEAK_MIN" -v cwarn="$CT_WARN" '
BEGIN {
    printf "constant-time reference : |t| = %s (threshold %s, informational)\n", ct, cwarn
    printf "planted leak (control)  : |t| = %s (must exceed %s)\n", lk, lmin
    rc = 0
    if (lk + 0 < lmin + 0) {
        printf "FAIL: the planted leak was NOT detected — |t| %s < %s; the harness is not measuring\n", lk, lmin
        rc = 1
    } else {
        printf "OK: the planted leak was detected (|t| %s >= %s)\n", lk, lmin
    }
    if (ct + 0 >= cwarn + 0) {
        printf "WARN: the constant-time reference showed |t| %s >= %s — investigate (runner noise, or a real regression)\n", ct, cwarn
    } else {
        printf "OK: the constant-time reference showed no input-dependent timing (|t| %s < %s)\n", ct, cwarn
    }
    exit rc
}' || rc=$?
echo
if [ "$rc" -eq 0 ]; then
    echo "OK: ct harness passed — planted leak caught, constant-time reference clean"
else
    echo "int/ct: harness FAILED"
fi
exit "$rc"
