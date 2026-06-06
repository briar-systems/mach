#!/usr/bin/env bash
#
# fuzz.sh — crash fuzzer for the Mach compiler.
#
# generates N small, mostly-valid Mach programs from a constrained grammar of
# SAFE constructs (see gen.awk) and compiles each with `mach`. the goal is to
# surface compiler CRASHES — an internal-error abort (exit 2) or a fatal signal
# (segfault / exit >= 128) — NOT to require that every random program compile.
# a clean run finds no crashes.
#
# usage:
#   tools/test/fuzz.sh [options]
#
# options:
#   -n, --iterations <N>   number of programs to generate   (default: 100)
#   -s, --seed <N>         base seed for the generator       (default: $RANDOM)
#   --mach <path>          compiler under test               (default: out/bin/mach)
#   --keep                 keep generated sources even when they don't crash
#   --out <dir>            directory for crash artifacts      (default: tmp/fuzz-crashes)
#   -h, --help             show this message
#
# each iteration uses seed = base + i, so a reported crash is reproducible with
# `awk -v seed=<S> -f tools/test/gen.awk`. crashing sources are saved under the
# crash directory along with the compiler's stderr.
#
# exit: 0 if no crash was found, 1 if any input crashed the compiler, 2 on a
# usage/setup error.

set -u

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo="$(cd "$here/../.." && pwd)"

MACH="$repo/out/bin/mach"
GEN="$here/gen.awk"
ITERS=100
BASE_SEED="$RANDOM"
KEEP=0
CRASH_DIR="$repo/tmp/fuzz-crashes"

while [ $# -gt 0 ]; do
    case "$1" in
        -n|--iterations) ITERS="$2"; shift 2 ;;
        -s|--seed) BASE_SEED="$2"; shift 2 ;;
        --mach) MACH="$2"; shift 2 ;;
        --keep) KEEP=1; shift ;;
        --out) CRASH_DIR="$2"; shift 2 ;;
        -h|--help) sed -n '2,27p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "error: unknown option '$1'" >&2; exit 2 ;;
    esac
done

if [ ! -x "$MACH" ]; then
    echo "error: mach compiler not found at '$MACH' (run 'make' first)" >&2
    exit 2
fi
if [ ! -f "$GEN" ]; then
    echo "error: generator not found at '$GEN'" >&2
    exit 2
fi

work="$(mktemp -d "${TMPDIR:-/tmp}/mach-fuzz.XXXXXX")"
trap 'rm -rf "$work"' EXIT

# a single throwaway project reused across iterations: only main.mach changes.
proj="$work/proj"
mkdir -p "$proj/dep"
cp -rL "$repo/dep/mach-std" "$proj/dep/mach-std"
cat > "$proj/mach.toml" <<'EOF'
[project]
id = "fuzz"
name = "fuzz"
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
binary = "linux/bin/fuzz"
EOF
# append the std dependency table the build resolves locally.
{
    echo
    echo "[deps.mach-std]"
    echo 'type = "local"'
    echo 'path = "dep/mach-std"'
    echo 'version = "local"'
} >> "$proj/mach.toml"

echo "fuzz: $ITERS iteration(s), base seed $BASE_SEED, compiler $MACH"

crashes=0
ok=0
rejected=0

i=0
while [ "$i" -lt "$ITERS" ]; do
    seed=$((BASE_SEED + i))
    src="$proj/main.mach"
    awk -v seed="$seed" -f "$GEN" > "$src"

    err="$work/stderr.txt"
    "$MACH" build -O0 --cwd "$proj" >/dev/null 2>"$err"
    code=$?
    rm -rf "$proj/out"

    # exit 0 = compiled, 1 = rejected (user error, not a bug). exit 2 = internal
    # error, >= 128 = fatal signal: both are compiler crashes worth reporting.
    if [ "$code" -eq 2 ] || [ "$code" -ge 128 ]; then
        crashes=$((crashes + 1))
        mkdir -p "$CRASH_DIR"
        dst="$CRASH_DIR/crash_seed_${seed}.mach"
        cp "$src" "$dst"
        cp "$err" "$CRASH_DIR/crash_seed_${seed}.stderr"
        echo "CRASH seed=$seed exit=$code -> $dst"
        sed 's/^/    /' "$err" | head -5
    elif [ "$code" -eq 0 ]; then
        ok=$((ok + 1))
        [ "$KEEP" = 1 ] && cp "$src" "$work/ok_seed_${seed}.mach"
    else
        rejected=$((rejected + 1))
    fi

    i=$((i + 1))
done

echo
echo "fuzz: compiled=$ok rejected=$rejected crashed=$crashes (of $ITERS)"
if [ "$crashes" -eq 0 ]; then
    echo "fuzz: no compiler crashes found"
    exit 0
fi
echo "fuzz: $crashes crashing input(s) saved under $CRASH_DIR"
exit 1
