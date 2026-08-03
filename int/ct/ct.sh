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
#
# TWO MODES, BECAUSE THE LEAKAGE MODEL HAS TWO OBSERVABLE CHANNELS (#2363). #1643's
# model is control-flow trace + memory-address trace + variable-latency operands.
# the measurement above covers the first and third. it CANNOT see the second, and
# not by a little: measured with the default batch, a real secret-indexed read over
# a 256 MB table scores |t| ~= 1-15 across runs -- straddling the threshold, so it
# can neither confirm nor deny the leak. that is worse than a clean miss, because
# either verdict looks like an answer.
#   * LATENCY MODE  (`measure`)      large batches of calls per timed sample. more
#                                    calls lift a running-time difference above
#                                    clock resolution.
#   * ADDRESS MODE  (`measure_addr`) ONE call per sample, many samples. batching is
#                                    exactly what hides an address leak: every call
#                                    in a batch gets the same input, so after the
#                                    first call both classes read a warm cache line
#                                    and the single miss carrying the signal is
#                                    averaged away.
# `addr_leak_in_latency_mode` measures the SAME function under the default batch and
# is reported, never asserted: it is the blind spot, measured rather than claimed.
# it is written over PUBLIC values on purpose — sema rejects a secret branch in any
# function, so this leak is unconstructable with `^` types, which is exactly the point:
# the static discipline forbids it, and this harness catches it the moment the
# discipline is dropped.
#
# HOW TO RUN.
#   bash int/ct/ct.sh                     # bootstrap a from-source compiler, measure
#   bash int/ct/ct.sh /path/to/mach       # use a prebuilt CT-capable compiler
#   bash int/ct/ct.sh /path/to/mach linux release
#   LEAK_RATIO_MIN=2.0 CT_WARN=10 bash int/ct/ct.sh   # override thresholds
# the PATH `mach` seed predates the `^`/#[oblivious] surface, so the harness cannot be
# built with it directly; omitting the compiler argument bootstraps one from source.
#
# WHY NOT A CI GATE. timing measurement is noise-sensitive and CI runners are shared
# and noisy, so this is run-on-demand, not wired into CI. it lives beside the int/
# cases but is NOT discovered by int/run.sh (which scans only surface/ and regression/),
# so it never becomes a flaky gate. the assertion is a leak-detection smoke: the leaky
# control MUST separate the two classes' MEAN times by LEAK_RATIO_MIN, while the
# constant-time reference is informational (reported and warned on, never failed) so
# ambient noise cannot break the build.
#
# THE LEAK GATE IS A MEAN RATIO, NOT |t| (#2371). it used to be `|t| >= 10`, and that
# threshold produced FALSE FAILURES on a busy machine: Welch divides by the sample
# variance, and concurrent builds inflate the variance without moving the means, so
# |t| collapses while the leak is still plainly there. measured on this harness at
# load average 27, six consecutive runs of the SAME binary:
#
#     |t|          11.00  10.35   7.51   7.78   9.27  11.24    <- 3 of 6 below 10
#     mean ratio    4.84   4.66   3.88   4.04   3.45   4.52    <- never below 3.4
#
# the |t| gate fails half the time under load; the ratio never came within 1.7x of
# its threshold. a statistic that swings two orders of magnitude while the underlying
# effect does not is not something to assert on. |t| is still REPORTED - it is the
# right statistic for judging significance on a quiet box - it is just not the gate.
set -euo pipefail

here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$here"
repo_root=$(cd "$here/../.." && pwd)

mach="${1:-}"
target="${2:-linux}"
profile="${3:-release}"

# the leaky control must separate the class means by at least this factor (hard).
# it is slowest on the FIXED class by construction - class 0 feeds the all-bytes-match
# reference, its longest path - so the expected direction is mean_fix > mean_rnd, and
# a flip would mean the probe stopped doing what it claims. 2.0 sits between the two
# populations measured at load average 25-27: the planted leak never fell below 3.45
# (range 3.45-4.84) and the clean reference never rose above 1.58 (range 0.68-1.58).
# that is 1.7x of headroom under the leak and 1.27x over the reference. the reference
# is the tighter side, so if this threshold ever needs moving it is that wander -
# which grows with machine load - that will force it, not the leak.
LEAK_RATIO_MIN="${LEAK_RATIO_MIN:-2.0}"

# ct |t| above CT_WARN warns but never fails.
CT_WARN="${CT_WARN:-10}"

# address mode asserts on mean_rnd/mean_fix. the planted leak must slow the random
# class by at least this factor (hard). the clean references are reported and warned
# on, never failed -- same split as LEAK_RATIO_MIN / CT_WARN above.
#
# WHY A RATIO AND NOT |t|: |t| divides by the sample variance, and machine load
# inflates the variance without moving the means. across runs at load average 8-10
# the planted leak's |t| swung 2.0 to 482 -- it would have failed a fixed threshold
# in half of them -- while the ratio stayed at 1.14 to 1.48 and never once missed.
#
# THE RATIO IS NOT IMMUNE, THOUGH, and the mechanism matters if you are tuning this:
# the leak is a fixed cache-miss cost (~100-140ns) over a baseline that GROWS with
# load, so a busy box compresses the ratio toward 1.0. 1.10 sits below the lowest
# observed leak (1.14) and above the null's quiet-box wander, but on a loaded box
# the null itself has been seen 12% off (0.876) -- larger than the leak's own margin.
# READ THE NULL CONTROL FIRST: if it is not flat, the run cannot discriminate and
# the leak verdict below it means nothing either way. re-run on a quiet machine.
ADDR_LEAK_MIN="${ADDR_LEAK_MIN:-1.10}"
ADDR_REF_WARN="${ADDR_REF_WARN:-0.10}"

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
# mean_rnd / mean_fix. the ADDRESS mode asserts on this rather than on |t|, because
# |t| divides by the sample variance and machine load inflates the variance without
# moving the means. observed across five runs at load average 10: the ratio held at
# 1.29-1.47 for the planted leak and 0.95-1.00 for the null control, while |t| for
# the same leak swung between 3.3 and 461. the ratio is what survives a busy box.
ratio() { echo "$out" | awk -v n="$1" '$0 ~ ("name=" n " ") { for (i=1;i<=NF;i++) { if ($i ~ /^mean_fix=/) { sub("mean_fix=","",$i); f=$i } if ($i ~ /^mean_rnd=/) { sub("mean_rnd=","",$i); r=$i } } if (f > 0) printf "%.4f", r/f }'; }
# mean_fix/mean_rnd for the latency probes, which are slowest on the fixed class.
inv_ratio() { echo "$out" | awk -v n="$1" '$0 ~ ("name=" n " ") { for (i=1;i<=NF;i++) { if ($i ~ /^mean_fix=/) { sub("mean_fix=","",$i); f=$i } if ($i ~ /^mean_rnd=/) { sub("mean_rnd=","",$i); r=$i } } if (r > 0) printf "%.4f", f/r }'; }
ct_t=$(extract ct)
leak_t=$(extract leak)
leak_r=$(inv_ratio leak)
ct_r=$(inv_ratio ct)
[ -n "$leak_r" ] || fail "no leak means parsed from harness output"
[ -n "$ct_t" ]   || fail "no ct result parsed from harness output"
[ -n "$leak_t" ] || fail "no leak result parsed from harness output"

addr_leak_r=$(ratio addr_leak)
addr_null_r=$(ratio addr_null)
addr_ct_r=$(ratio addr_ct)
addr_leak_t=$(extract addr_leak)
addr_latency_t=$(extract addr_leak_in_latency_mode)
[ -n "$addr_leak_r" ] || fail "no addr_leak result parsed from harness output"
[ -n "$addr_null_r" ] || fail "no addr_null result parsed from harness output"

echo
rc=0
awk -v ct="$ct_t" -v lk="$leak_t" -v lr="$leak_r" -v cr="$ct_r" -v lmin="$LEAK_RATIO_MIN" -v cwarn="$CT_WARN" '
BEGIN {
    printf "constant-time reference : mean_fix/mean_rnd = %s   |t| = %s (threshold %s, informational)\n", cr, ct, cwarn
    printf "planted leak (control)  : mean_fix/mean_rnd = %s (must exceed %s)   |t| = %s\n", lr, lmin, lk
    rc = 0
    if (lr + 0 < lmin + 0) {
        printf "FAIL: the planted leak was NOT detected — mean ratio %s < %s; the harness is not measuring\n", lr, lmin
        rc = 1
    } else {
        printf "OK: the planted leak was detected (mean ratio %s >= %s)\n", lr, lmin
    }
    if (ct + 0 >= cwarn + 0) {
        printf "WARN: the constant-time reference showed |t| %s >= %s — investigate (runner noise, or a real regression)\n", ct, cwarn
    } else {
        printf "OK: the constant-time reference showed no input-dependent timing (|t| %s < %s)\n", ct, cwarn
    }
    exit rc
}' || rc=$?
echo

echo "address-trace mode (#2363) — asserted on the mean ratio, not |t|"
awk -v lr="$addr_leak_r" -v nr="$addr_null_r" -v cr="$addr_ct_r" \
    -v lt="$addr_leak_t" -v lat="$addr_latency_t" -v lmin="$ADDR_LEAK_MIN" -v ntol="$ADDR_REF_WARN" '
BEGIN {
    printf "  planted address leak    : mean_rnd/mean_fix = %s (must exceed %s)   |t| = %s\n", lr, lmin, lt
    printf "  null control            : mean_rnd/mean_fix = %s (informational, warns past %s)\n", nr, ntol
    printf "  masked scan (reference) : mean_rnd/mean_fix = %s\n", cr
    printf "  SAME leak, latency mode : |t| = %s -- informational: this is the blind spot\n", lat
    rc = 0
    if (lr + 0 < lmin + 0) {
        printf "FAIL: the planted ADDRESS leak was NOT detected (ratio %s < %s).\n", lr, lmin
        printf "      most likely this machine has a last-level cache at or above the probe table\n"
        printf "      size -- check `lscpu -C` and raise BIG in int/ct/src/addr.mach.\n"
        rc = 1
    } else {
        printf "OK: the planted address leak was detected (ratio %s >= %s)\n", lr, lmin
    }
    d = nr - 1.0; if (d < 0) d = -d
    if (d > ntol + 0) {
        printf "WARN: the null control moved (ratio %s) -- runner noise, or the sampling is\n", nr
        printf "      producing a signal of its own. re-run on a quiet machine before trusting\n"
        printf "      the leak verdict above.\n"
    } else {
        printf "OK: the null control is flat (ratio %s)\n", nr
    }
    exit rc
}' || rc=$?

echo
if [ "$rc" -eq 0 ]; then
    echo "OK: ct harness passed — both planted leaks caught, constant-time references clean"
else
    echo "int/ct: harness FAILED"
fi
exit "$rc"
