#!/usr/bin/env bash
# installs the seed release and builds this compiler from it: the seed builds a
# at debug, so no older optimiser compiles this tree, a builds b at the profile,
# and --fixpoint adds c, built by b at the profile on another thread count, and
# requires b == c. when --cross-dir and --cross-targets are given, b (or c)
# cross-compiles a native stage compiler for each requested target into
# <cross-dir>/<target>/mach[.exe].
# contract: reads RUNNER_OS and RUNNER_TEMP from the runner, and writes
# MACH=<absolute path of the last stage> to GITHUB_ENV for later steps.
#
# usage: bootstrap.sh [--fixpoint] [--profile <name>] [--cross-dir <dir> --cross-targets <csv>]
set -euo pipefail

# a published release that builds this tree at debug. it moves to a later
# release only when the tree first needs something this one cannot read
seed=7.0.0

fixpoint=0
profile=debug
cross_dir=
cross_targets=
while [ $# -gt 0 ]; do
    case "$1" in
        --fixpoint) fixpoint=1 ;;
        --profile) shift; profile=${1:?--profile needs a name} ;;
        --cross-dir) shift; cross_dir=${1:?--cross-dir needs a path} ;;
        --cross-targets) shift; cross_targets=${1:?--cross-targets needs a list} ;;
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

# the install scripts verify the archive against the release's SHA256SUMS
dir=$RUNNER_TEMP/seed
if [ "$RUNNER_OS" = Windows ]; then
    MACH_VERSION=$seed MACH_INSTALL_DIR=$dir pwsh -NoProfile -File dist/install.ps1
    dir=$(cygpath -u "$dir")
else
    MACH_VERSION=$seed MACH_INSTALL_DIR=$dir sh dist/install.sh
fi
seed_bin=$dir/mach$exe

"$seed_bin" dep pull .
"$seed_bin" build . --profile debug -o "a$exe"
"./a$exe" dep pull .
"./a$exe" build . --profile "$profile" --threads 2 -o "b$exe"
last=b$exe
if [ "$fixpoint" = 1 ]; then
    "./b$exe" build . --profile "$profile" --threads 3 -o "c$exe"
    cmp "b$exe" "c$exe" || { echo "::error::the $profile build did not reach its fixpoint"; exit 1; }
    last=c$exe
fi
echo "MACH=$PWD/$last" >>"$GITHUB_ENV"

if [ -n "$cross_dir" ] && [ -n "$cross_targets" ]; then
    mkdir -p "$cross_dir"
    host_target=
    if [ "$RUNNER_OS" = Linux ]; then
        case "$(uname -m)" in
            x86_64|amd64) host_target=linux-x86_64 ;;
            arm64|aarch64) host_target=linux-aarch64 ;;
        esac
    fi
    IFS=, read -r -a requested <<< "$cross_targets"
    for t in "${requested[@]}"; do
        [ -n "$t" ] || continue
        cexe=
        case "$t" in windows-*) cexe=.exe ;; esac
        mkdir -p "$cross_dir/$t"
        if [ "$t" = "$host_target" ]; then
            cp "$last" "$cross_dir/$t/mach$cexe"
        else
            "./$last" build . --profile "$profile" -t "$t" -o "cross-$t$cexe"
            mv "cross-$t$cexe" "$cross_dir/$t/mach$cexe"
        fi
    done
fi
