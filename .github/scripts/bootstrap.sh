#!/usr/bin/env bash
# walks the bootstrap chain (.github/bootstrap-chain) and builds this compiler
# from its last step: that step builds a at debug, so no older optimiser compiles
# this tree, a builds b at the profile, and --fixpoint adds c, built by b at the
# profile on another thread count, and requires b == c.
# contract: reads RUNNER_OS and RUNNER_TEMP from the runner, and writes
# MACH=<absolute path of the last stage> to GITHUB_ENV for later steps.
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
while read -r sha overrides || [ -n "${sha:-}" ]; do
    case "$sha" in ""|\#*|seed) continue ;; esac
    key=$sha
    for ov in $overrides; do
        key="${key}-${ov%=*}-${ov#*=}"
    done
    bin=$root/$key/mach$exe
    if [ ! -x "$bin" ]; then
        src=$root/src-$key
        fetch "$sha" "$src"
        if [ -z "$prev" ]; then
            # the seed reads only the previous major's manifest keys
            dir=$RUNNER_TEMP/seed
            if [ "$RUNNER_OS" = Windows ]; then
                seed_target=
                [ "${RUNNER_ARCH:-}" = ARM64 ] && seed_target=x86_64-windows
                MACH_VERSION=$seed MACH_INSTALL_DIR=$dir MACH_TARGET=$seed_target pwsh -NoProfile -File dist/install.ps1
                dir=$(cygpath -u "$dir")
            else
                MACH_VERSION=$seed MACH_INSTALL_DIR=$dir sh dist/install.sh
            fi
            mkdir -p "$root/$key"
            (cd "$src" && bash .github/scripts/seed-build.sh "$dir/mach$exe" "$bin")
        else
            mkdir -p "$root/$key"
            # build -o refuses a path outside the project
            (
                cd "$src"
                "$prev" dep pull .
                for ov in $overrides; do
                    dep=${ov%=*}
                    pin=${ov#*=}
                    git -C "dep/$dep" fetch --quiet --depth 1 origin "$pin"
                    git -C "dep/$dep" checkout --quiet FETCH_HEAD
                done
                # 52b293da3 predates 4502a7336 (system env on non-spirv targets), which b03933133's
                # mach.toml states for windows and darwin; 52b293da3 implies those environments without env
                if [ "$sha" = b0393313348484d1c4da720ab8006d295d86b7ff ]; then
                    awk '!/^env[[:space:]]*=[[:space:]]*"(msvc|macos)"/' mach.toml >mach.toml.tmp
                    mv mach.toml.tmp mach.toml
                fi
                # 8bdc1b465 predates dialect-aware asm lexing (71faeddfe), so an apostrophe inside
                # a // comment in darwin/aarch64.mach fails as an unterminated char literal
                if [ "$sha" = 761e119834342662a9224301495ae3505b0ec548 ] && [ -f dep/std/src/runtime/darwin/aarch64.mach ]; then
                    awk '{ gsub(/\/\//, "#"); print }' dep/std/src/runtime/darwin/aarch64.mach >dep/std/src/runtime/darwin/aarch64.mach.tmp
                    mv dep/std/src/runtime/darwin/aarch64.mach.tmp dep/std/src/runtime/darwin/aarch64.mach
                fi
                "$prev" build . -o "mach$exe"
            )
            mv "$src/mach$exe" "$bin"
        fi
        rm -rf "$src"
    fi
    prev=$bin
done < <(tr -d '\r' <"$chain")

target=()
if [ "$RUNNER_OS" = Windows ] && [ "${RUNNER_ARCH:-}" = ARM64 ]; then
    target=(-t windows-aarch64)
fi

"$prev" dep pull .
"$prev" build . --profile debug "${target[@]}" -o "a$exe"
"./a$exe" dep pull .
"./a$exe" build . --profile "$profile" --threads 2 "${target[@]}" -o "b$exe"
last=b$exe
if [ "$fixpoint" = 1 ]; then
    "./b$exe" build . --profile "$profile" --threads 3 "${target[@]}" -o "c$exe"
    cmp "b$exe" "c$exe" || { echo "::error::the $profile build did not reach its fixpoint"; exit 1; }
    last=c$exe
fi
echo "MACH=$PWD/$last" >>"$GITHUB_ENV"
