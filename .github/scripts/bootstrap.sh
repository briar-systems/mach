#!/usr/bin/env bash
# walks the bootstrap chain (.github/bootstrap-chain) and builds this compiler
# from its last step: that step builds a, a builds b, and --fixpoint adds c and
# requires b == c.
# the last stage is exported to later steps as MACH.
#
# usage: bootstrap.sh [--fixpoint] [--profile <name>]
set -euo pipefail

fixpoint=0
profile=debug
while [ $# -gt 0 ]; do
    case "$1" in
        --fixpoint) fixpoint=1 ;;
        --profile) shift; profile=${1:?--profile needs a name} ;;
        *) echo "bootstrap.sh: unknown option '$1'" >&2; exit 2 ;;
    esac
    shift
done

# rosetta 2 cannot host mach's raw bsdthread_create worker threads
if [ "$RUNNER_OS" = macOS ] && [ "$(sysctl -n sysctl.proc_translated 2>/dev/null || echo 0)" = 1 ]; then
    echo "::error::this runner is translated by rosetta 2, not native silicon"
    exit 1
fi

exe=
[ "$RUNNER_OS" = Windows ] && exe=.exe

# the chain file names the seed release, then each mach sha in order. a step's
# binary is kept under $RUNNER_TEMP/chain/<sha>, which CI caches by os, arch and
# the chain file's hash, and a step already there is not rebuilt
chain=.github/bootstrap-chain
seed=$(awk '$1 == "seed" { print $2 }' "$chain")
steps=$(awk '!/^#/ && NF && $1 != "seed" { print $1 }' "$chain")
[ -n "$seed" ] && [ -n "$steps" ] || { echo "::error::$chain needs a seed and a step"; exit 1; }
root=$RUNNER_TEMP/chain
[ "$RUNNER_OS" = Windows ] && root=$(cygpath -u "$root")

fetch() {
    rm -rf "$2"
    git init --quiet "$2"
    git -C "$2" remote add origin https://github.com/briar-systems/mach
    git -C "$2" fetch --quiet --depth 1 origin "$1"
    git -C "$2" checkout --quiet FETCH_HEAD
}

prev=
for sha in $steps; do
    bin=$root/$sha/mach$exe
    if [ ! -x "$bin" ]; then
        src=$root/src-$sha
        fetch "$sha" "$src"
        if [ -z "$prev" ]; then
            # the seed reads only the previous major's manifest keys
            dir=$RUNNER_TEMP/seed
            if [ "$RUNNER_OS" = Windows ]; then
                MACH_VERSION=$seed MACH_INSTALL_DIR=$dir pwsh -NoProfile -File dist/install.ps1
                dir=$(cygpath -u "$dir")
            else
                MACH_VERSION=$seed MACH_INSTALL_DIR=$dir sh dist/install.sh
            fi
            mkdir -p "$root/$sha"
            (cd "$src" && bash .github/scripts/seed-build.sh "$dir/mach$exe" "$bin")
        else
            mkdir -p "$root/$sha"
            # build -o refuses a path outside the project
            (cd "$src" && "$prev" dep pull . && "$prev" build . -o "mach$exe")
            mv "$src/mach$exe" "$bin"
        fi
        rm -rf "$src"
    fi
    prev=$bin
done

"$prev" dep pull .
"$prev" build . --profile "$profile" -o "a$exe"
"./a$exe" dep pull .
"./a$exe" build . --profile "$profile" -o "b$exe"
last=b$exe
if [ "$fixpoint" = 1 ]; then
    "./b$exe" build . --profile "$profile" -o "c$exe"
    cmp "b$exe" "c$exe" || { echo "::error::the $profile build did not reach its fixpoint"; exit 1; }
    last=c$exe
fi
echo "MACH=$PWD/$last" >>"$GITHUB_ENV"
