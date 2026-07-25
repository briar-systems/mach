#!/usr/bin/env bash
# seed-tripwire.sh — fails when a bootstrap workaround becomes removable.
#
# usage: seed-tripwire.sh <seed-compiler>
#
# THE CLASS. CI builds the compiler in ONE generation from the released seed
# (`mach build . -o m`, then `int/run.sh ... ./m`), so a construct the seed
# miscompiles is miscompiled in the artifact every other lane then tests. A
# language fix therefore cannot be USED in the compiler's own source in the PR
# that lands it: the code is correct, generations B and C prove it, and only the
# seed cannot compile it. The workaround such a fix forces stays load-bearing
# until a release ships a seed carrying the fix.
#
# That is exactly the kind of item that rots. Nothing fails when it becomes
# removable, so the workaround outlives its cause and its comment quietly turns
# into a lie. This fires instead.
#
# HOW. Each entry runs one int case with the SEED — not with the from-source
# compiler — and asserts the case still FAILS. That failure IS the workaround's
# justification. The day the seed compiles the case correctly the assertion
# inverts, this exits nonzero, and the message names what to delete. Removing
# the workaround means removing its entry here too; the last entry leaving takes
# the script and its CI step with it.
#
# A case that stops failing for the RIGHT reason is the whole point, so an entry
# that can no longer observe its property (the case will not build, or no longer
# exists) is a loud failure and not a silent pass — a tripwire that has stopped
# watching is worse than none.
set -eu

usage() {
    echo "usage: seed-tripwire.sh <seed-compiler>" >&2
    exit 2
}

[ $# -eq 1 ] || usage
seed=$1
case "$seed" in
    /*) ;;
    *)  seed="$(cd "$(dirname "$seed")" && pwd)/$(basename "$seed")" ;;
esac
[ -x "$seed" ] || { echo "seed-tripwire: '$seed' is not an executable compiler" >&2; exit 2; }

root="$(cd "$(dirname "$0")/../.." && pwd)"
status=0

# assert one case still fails under the seed
# ---
# $1: the case directory name, as an int/run.sh --filter glob
# $2: what to delete once the case starts passing
watch() {
    name=$1
    obsolete=$2

    if out=$(bash "$root/int/run.sh" --target linux --filter "$name" "$seed" 2>&1); then
        echo "::error::seed-tripwire: the seed now compiles int/*/$name correctly."
        echo "::error::  $obsolete"
        status=1
        return
    fi

    # it failed — but only a golden DIFF means the seed still miscompiles the
    # construct. a build error or an unmatched filter means this entry has
    # stopped watching anything.
    if ! printf '%s\n' "$out" | grep -q "^FAIL .*/$name \[linux/.*\] (diff)$"; then
        echo "::error::seed-tripwire: int/*/$name no longer observes its property under the seed."
        echo "::error::  expected a golden diff; got:"
        printf '%s\n' "$out" | sed 's/^/::error::  /'
        status=1
    fi
}

# #2284: `-0.0` folds correctly since #2274, but the seed folds it to +0.0, so
# the reduction vectorizer cannot yet spell its identity element as the literal.
watch '2274-negative-zero' \
      'replace NEG_ZERO_BITS:~f64 with the -0.0 literal in src/lang/me/transform/vectorize.mach, drop the constant and its comment, and delete this entry (#2284).'

exit $status
