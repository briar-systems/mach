#!/usr/bin/env bash
#
# differential.sh — optimization-level differential testing of the Mach compiler.
#
# for each input program in the corpus (the numbered `examples/`), compile and
# run it at -O0 and at -O2 and assert the program's exit code AND stdout are
# identical across the two levels. a mismatch means an optimization miscompile.
# optionally also diffs the same program compiled by `smach` against `mach`.
#
# usage:
#   tools/test/differential.sh [options]
#
# options:
#   --mach <path>     compiler under test       (default: out/bin/mach)
#   --smach <path>    second compiler to diff   (default: out/bin/smach, if present)
#   --examples <dir>  corpus directory          (default: examples)
#   --no-smach        skip the smach-vs-mach cross-compiler diff
#   -h, --help        show this message
#
# exit: 0 if every input matches across levels (and compilers), 1 on any mismatch
# or build failure, 2 on a usage/setup error.

set -u

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo="$(cd "$here/../.." && pwd)"

MACH="$repo/out/bin/mach"
SMACH="$repo/out/bin/smach"
EXAMPLES="$repo/examples"
DO_SMACH=1

while [ $# -gt 0 ]; do
    case "$1" in
        --mach) MACH="$2"; shift 2 ;;
        --smach) SMACH="$2"; shift 2 ;;
        --examples) EXAMPLES="$2"; shift 2 ;;
        --no-smach) DO_SMACH=0; shift ;;
        -h|--help) sed -n '2,21p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "error: unknown option '$1'" >&2; exit 2 ;;
    esac
done

if [ ! -x "$MACH" ]; then
    echo "error: mach compiler not found at '$MACH' (run 'make' first)" >&2
    exit 2
fi
if [ "$DO_SMACH" = 1 ] && [ ! -x "$SMACH" ]; then
    DO_SMACH=0
fi

work="$(mktemp -d "${TMPDIR:-/tmp}/mach-diff.XXXXXX")"
trap 'rm -rf "$work"' EXIT

# build <compiler> <project-dir> <O-level>; on success echoes the binary path,
# else echoes nothing. all build output is discarded.
build_one() {
    local cc="$1" proj="$2" olevel="$3"
    if ! "$cc" build "$olevel" --cwd "$proj" >/dev/null 2>&1; then
        return 1
    fi
    local bin
    bin="$(find "$proj/out" -type f -path '*/bin/*' 2>/dev/null | head -1)"
    [ -n "$bin" ] && printf '%s' "$bin"
}

# run <binary>; sets globals RUN_CODE and RUN_OUT.
run_one() {
    RUN_OUT="$("$1" 2>/dev/null)"
    RUN_CODE=$?
}

total=0
fail=0

printf '%-22s %-14s %-14s %-8s\n' "INPUT" "-O0 (code)" "-O2 (code)" "RESULT"
printf '%s\n' "------------------------------------------------------------"

for proj in "$EXAMPLES"/*/; do
    name="$(basename "$proj")"
    # skip non-runnable fixtures: a project must have a mach.toml with an
    # executable target. `full` is a syntax fixture, not a program.
    [ -f "$proj/mach.toml" ] || continue
    [ "$name" = "full" ] && continue

    total=$((total + 1))

    # isolated, dereferenced copy so examples/ is never written to and the
    # symlinked std dep travels with the project.
    sandbox="$work/$name"
    cp -rL "$proj" "$sandbox" 2>/dev/null

    if ! bin0="$(build_one "$MACH" "$sandbox" -O0)" || [ -z "$bin0" ]; then
        printf '%-22s %-14s %-14s %-8s\n' "$name" "BUILD-FAIL" "-" "FAIL"
        fail=$((fail + 1)); continue
    fi
    run_one "$bin0"; code0=$RUN_CODE; out0="$RUN_OUT"
    rm -rf "$sandbox/out"

    if ! bin2="$(build_one "$MACH" "$sandbox" -O2)" || [ -z "$bin2" ]; then
        printf '%-22s %-14s %-14s %-8s\n' "$name" "$code0" "BUILD-FAIL" "FAIL"
        fail=$((fail + 1)); continue
    fi
    run_one "$bin2"; code2=$RUN_CODE; out2="$RUN_OUT"
    rm -rf "$sandbox/out"

    result="PASS"
    if [ "$code0" != "$code2" ] || [ "$out0" != "$out2" ]; then
        result="FAIL"
        fail=$((fail + 1))
    fi
    printf '%-22s %-14s %-14s %-8s\n' "$name" "$code0" "$code2" "$result"

    if [ "$result" = "FAIL" ]; then
        echo "  -> -O0 vs -O2 divergence (exit $code0 vs $code2)"
        if [ "$out0" != "$out2" ]; then
            echo "  -O0 stdout:"; printf '%s\n' "$out0" | sed 's/^/    | /'
            echo "  -O2 stdout:"; printf '%s\n' "$out2" | sed 's/^/    | /'
        fi
    fi
done

# optional cross-compiler diff: smach vs mach at -O0.
if [ "$DO_SMACH" = 1 ]; then
    echo
    printf '%-22s %-14s %-14s %-8s\n' "INPUT (smach/mach)" "smach (code)" "mach (code)" "RESULT"
    printf '%s\n' "------------------------------------------------------------"
    for proj in "$EXAMPLES"/*/; do
        name="$(basename "$proj")"
        [ -f "$proj/mach.toml" ] || continue
        [ "$name" = "full" ] && continue

        sandbox="$work/x_$name"
        cp -rL "$proj" "$sandbox" 2>/dev/null

        if ! bs="$(build_one "$SMACH" "$sandbox" -O0)" || [ -z "$bs" ]; then
            printf '%-22s %-14s %-14s %-8s\n' "$name" "BUILD-FAIL" "-" "FAIL"
            fail=$((fail + 1)); rm -rf "$sandbox"; continue
        fi
        run_one "$bs"; cs=$RUN_CODE; os=$RUN_OUT
        rm -rf "$sandbox/out"

        if ! bm="$(build_one "$MACH" "$sandbox" -O0)" || [ -z "$bm" ]; then
            printf '%-22s %-14s %-14s %-8s\n' "$name" "$cs" "BUILD-FAIL" "FAIL"
            fail=$((fail + 1)); rm -rf "$sandbox"; continue
        fi
        run_one "$bm"; cm=$RUN_CODE; om=$RUN_OUT
        rm -rf "$sandbox"

        result="PASS"
        if [ "$cs" != "$cm" ] || [ "$os" != "$om" ]; then
            result="FAIL"; fail=$((fail + 1))
        fi
        printf '%-22s %-14s %-14s %-8s\n' "$name" "$cs" "$cm" "$result"
    done
fi

echo
if [ "$fail" -eq 0 ]; then
    echo "differential: $total input(s), all consistent"
    exit 0
fi
echo "differential: $fail mismatch(es) across $total input(s)"
exit 1
