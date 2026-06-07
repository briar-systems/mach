#!/usr/bin/env bash
# run.sh — compiler-performance benchmark harness for the Mach compiler.
#
# times `mach build` over a set of representative inputs (the compiler's own
# source as the large workload, a couple of examples as small workloads, and
# a generated synthetic input for a scaling signal), runs each input N times,
# and reports per-input wall-clock min + median in milliseconds alongside the
# input's module and line counts.
#
# usage: tools/bench/run.sh [path-to-mach] [iterations]
#   path-to-mach  compiler binary to benchmark   (default: out/bin/mach)
#   iterations    timed runs per input           (default: 5, or $BENCH_ITER)
#
# it measures end-to-end `mach build` wall time, not isolated compiler phases.
# all build output is written into a throwaway artifacts directory that is
# removed on exit, so the harness leaves no stray objects or binaries behind.
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

mach="${1:-$repo_root/out/bin/mach}"
iterations="${2:-${BENCH_ITER:-5}}"

# resolve the compiler to an absolute path so per-input --cwd builds find it.
case "$mach" in
    /*) : ;;
    *)  mach="$(cd "$(dirname "$mach")" 2>/dev/null && pwd)/$(basename "$mach")" || true ;;
esac

if [[ ! -x "$mach" ]]; then
    echo "error: compiler not found or not executable: $mach" >&2
    echo "build it first with: make clean && make" >&2
    exit 1
fi

if ! [[ "$iterations" =~ ^[0-9]+$ ]] || (( iterations < 1 )); then
    echo "error: iterations must be a positive integer, got: $iterations" >&2
    exit 1
fi

# throwaway tree holding the generated synthetic project. resolved to an
# absolute path so its contents are stable regardless of the harness cwd.
tmp_dir="$(cd "$(mktemp -d "${TMPDIR:-/tmp}/mach-bench.XXXXXX")" && pwd)"

# `mach build` always roots -o and --artifacts at the project root, so every
# build's output necessarily lands inside the project's own out/ tree. we
# confine all of it to out/<artifacts_name>/ and remove that per project on
# exit, leaving the project tree as it was found.
artifacts_name="bench-tmp"
cleanup() {
    for proj in "${PROJECT_DIRS[@]:-}"; do
        [[ -n "$proj" ]] || continue
        rm -rf "$proj/out/$artifacts_name"
        # drop a now-empty project out/ the build created; rmdir leaves a
        # non-empty out/ (e.g. the repo's out/bin holding the compiler) intact.
        rmdir "$proj/out" 2>/dev/null || true
    done
    rm -rf "$tmp_dir"
}
trap cleanup EXIT

# generate the synthetic scaling input as a standalone project.
syn_dir="$tmp_dir/synthetic"
mkdir -p "$syn_dir/dep"
ln -s "$repo_root/dep/mach-std" "$syn_dir/dep/mach-std"
bash "$repo_root/tools/bench/gen.sh" "${BENCH_SYN_COUNT:-400}" > "$syn_dir/main.mach"
cat > "$syn_dir/mach.toml" <<EOF
[project]
id = "synthetic"
name = "Synthetic Benchmark"
version = "0.1.0"
src = "."
dep = "dep"
out = "out"
dir_src = "."
dir_dep = "dep"
dir_out = "out"
target = "native"

[targets.linux]
os = "linux"
isa = "x86_64"
abi = "sysv64"
mode = "executable"
entrypoint = "main.mach"
artifacts = "linux"
binary = "linux/bin/synthetic"

[deps.mach-std]
type = "local"
path = "dep/mach-std"
version = "local"
EOF

# benchmark set: "label|project-dir". the project dir is where mach.toml lives.
BENCHMARKS=(
    "compiler-self|$repo_root"
    "ex-01-hello|$repo_root/examples/01_hello"
    "ex-09-collections|$repo_root/examples/09_collections"
    "synthetic|$syn_dir"
)

# track project dirs whose out/<artifacts_name> we must clean.
PROJECT_DIRS=()

# now_ms: current wall-clock time in integer milliseconds.
now_ms() {
    local ns
    ns="$(date +%s%N)"
    echo $(( ns / 1000000 ))
}

# count_lines: total non-dep .mach source lines under a project dir.
count_lines() {
    local dir="$1"
    find "$dir" -name '*.mach' -not -path '*/dep/*' -not -path '*/out/*' \
        -exec cat {} + 2>/dev/null | wc -l | tr -d ' '
}

# module_count: module count the build reports via verbose output, or "?" if
# the probe build fails (the timed runs below still surface the failure).
module_count() {
    local dir="$1"
    local out
    out="$("$mach" build --cwd "$dir" --artifacts "$artifacts_name" \
        -o "out/$artifacts_name/probe.bin" -v 2>&1 1>/dev/null || true)"
    local n
    n="$(printf '%s\n' "$out" | sed -n 's/.*(\([0-9]\+\) module(s)).*/\1/p' | head -1)"
    [[ -n "$n" ]] && echo "$n" || echo "?"
}

# median: median of the integer args (sorted), rounding the even-count average down.
median() {
    local sorted
    sorted=($(printf '%s\n' "$@" | sort -n))
    local n=${#sorted[@]}
    local mid=$(( n / 2 ))
    if (( n % 2 == 1 )); then
        echo "${sorted[$mid]}"
    else
        echo $(( (sorted[mid - 1] + sorted[mid]) / 2 ))
    fi
}

echo "mach compiler benchmark"
echo "  compiler:   $mach"
echo "  iterations: $iterations"
echo "  date:       $(date -u '+%Y-%m-%dT%H:%M:%SZ')"
echo
printf '%-20s %8s %8s %8s %8s\n' "input" "modules" "lines" "min(ms)" "med(ms)"
printf '%-20s %8s %8s %8s %8s\n' "-----" "-------" "-----" "-------" "-------"

failures=0
for entry in "${BENCHMARKS[@]}"; do
    label="${entry%%|*}"
    dir="${entry#*|}"
    PROJECT_DIRS+=("$dir")

    if [[ ! -f "$dir/mach.toml" ]]; then
        printf '%-20s %8s %8s %8s %8s\n' "$label" "?" "?" "SKIP" "no-toml"
        continue
    fi

    lines="$(count_lines "$dir")"
    modules="$(module_count "$dir")"

    times=()
    failed=0
    for (( i = 0; i < iterations; i++ )); do
        # remove prior objects so each run is a full rebuild, not incremental.
        rm -rf "$dir/out/$artifacts_name"
        start="$(now_ms)"
        if ! "$mach" build --cwd "$dir" --artifacts "$artifacts_name" \
                -o "out/$artifacts_name/$label.bin" >/dev/null 2>&1; then
            failed=1
            break
        fi
        end="$(now_ms)"
        times+=( $(( end - start )) )
    done

    if (( failed )); then
        printf '%-20s %8s %8s %8s %8s\n' "$label" "$modules" "$lines" "FAIL" "FAIL"
        failures=$(( failures + 1 ))
        continue
    fi

    min="${times[0]}"
    for t in "${times[@]}"; do (( t < min )) && min="$t"; done
    med="$(median "${times[@]}")"

    printf '%-20s %8s %8s %8s %8s\n' "$label" "$modules" "$lines" "$min" "$med"
done

echo
if (( failures > 0 )); then
    echo "$failures input(s) failed to build" >&2
    exit 1
fi
echo "done"
