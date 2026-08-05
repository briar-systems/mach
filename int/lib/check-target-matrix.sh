#!/usr/bin/env bash
# check-target-matrix.sh — verify every case declares a [target.<build-target>]
# block in its mach.toml for every leg it actually builds on.
#
# usage: check-target-matrix.sh
#
# targets.conf documents that adding a leg "is one row here and touches no
# workflow" - true of the workflow, false of case manifests (#2353). A case
# with no case.conf `targets:` restriction defaults to `targets: all` (see
# case.sh's read_case_conf), so it is expected to build on every targets.conf
# leg and needs a [target.<leg>] block for each - or, when case.conf names a
# `build-target`, a single [target.<build-target>] block regardless of how
# many legs cross-build that same format (a structural case that runs
# `targets: linux` and cross-builds windows / darwin / freestanding / bmos /
# spirv only ever needs the one block its build-target names, never a block
# per leg it happens to run on).
#
# Five real cases were added while x86_64-darwin was dropped from
# targets.conf, none declared its block, and they would have failed to
# compile the moment the leg returned - caught only because someone was
# actively restoring the platform at the time (#2327).
#
# STATIC AND FAST: no compiler, no build, no macOS runner - reads manifests
# only. Wired into the int-matrix job, the same lightweight job that already
# reads targets.conf to generate the CI matrix, so this runs on every
# ordinary PR - not only at main cadence, the cadence the darwin gap this
# guards against actually surfaces on.
set -eu

here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$here/case.sh"

fails=0
checked=0

for dir in "$here"/../surface/*/ "$here"/../regression/*/; do
    dir=${dir%/}
    [ -f "$dir/mach.toml" ] || continue
    case_id=${dir#"$here"/../}

    read_case_conf "$dir"

    for leg in $case_allow; do
        in_list "$leg" "$case_exempt" && continue
        build_target=${case_build_target:-$leg}
        checked=$((checked + 1))
        if ! grep -q "^\[target\.$build_target\]" "$dir/mach.toml"; then
            echo "FAIL $case_id: leg '$leg' needs [target.$build_target] in mach.toml" >&2
            fails=$((fails + 1))
        fi
    done
done

if [ "$checked" -eq 0 ]; then
    echo "check-target-matrix.sh: no case x leg pairs checked - broken harness, not a finding" >&2
    exit 2
fi

if [ "$fails" -ne 0 ]; then
    echo "check-target-matrix: $fails missing target block(s)" >&2
    exit 1
fi
echo "check-target-matrix: $checked case x leg pair(s) OK"
