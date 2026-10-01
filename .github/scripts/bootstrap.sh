#!/usr/bin/env bash
# installs the pinned seed release with dist/ and builds this compiler from it:
# the seed builds a, a builds b, and --fixpoint adds c and requires b == c.
# the last stage is exported to later steps as MACH.
#
# usage: bootstrap.sh [--fixpoint] [--profile <name>]
set -euo pipefail

seed=6.8.0

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

dir=$RUNNER_TEMP/seed
exe=
if [ "$RUNNER_OS" = Windows ]; then
    MACH_VERSION=$seed MACH_INSTALL_DIR=$dir pwsh -NoProfile -File dist/install.ps1
    dir=$(cygpath -u "$dir")
    exe=.exe
else
    MACH_VERSION=$seed MACH_INSTALL_DIR=$dir sh dist/install.sh
fi

"$dir/mach$exe" dep pull .
"$dir/mach$exe" build . --profile "$profile" -o "a$exe"
"./a$exe" build . --profile "$profile" -o "b$exe"
last=b$exe
if [ "$fixpoint" = 1 ]; then
    "./b$exe" build . --profile "$profile" -o "c$exe"
    cmp "b$exe" "c$exe" || { echo "::error::the $profile build did not reach its fixpoint"; exit 1; }
    last=c$exe
fi
echo "MACH=$PWD/$last" >>"$GITHUB_ENV"
