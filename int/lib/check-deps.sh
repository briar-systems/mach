#!/usr/bin/env bash
# check-deps.sh — verify the pin sources cover every git dep the int suite builds.
#
# usage: check-deps.sh
#
# `--deps pin` is the lane a release gate and a bisect use, because a case that
# floats a ref can change verdict with zero changes in this repo (#2592). #2729 is
# what happened when the lane's inputs were only ever validated by running it: two
# cases declared mach-shader, no lock recorded it, and the pinned lane could not
# start on any target. Nothing reported that the reproducible lane never ran, and
# the pinned run CI does have is main-cadence darwin only, where neither SPIR-V case
# is in the leg set at all.
#
# So the condition is checked here instead of discovered there. Every check below is
# a property of committed files:
#
#   coverage    every git dep a case declares is recorded by mach.lock or
#               int/deps.lock. this is #2729 itself, and it fails naming the dep and
#               the case that declares it.
#   disjoint    int/deps.lock records nothing mach.lock already records, so no dep
#               has two places to bump and no run can be pinned to two answers.
#   live        int/deps.lock records nothing no case declares, so a dep dropped
#               from the suite does not leave a pin behind that looks maintained.
#   agreement   two cases declaring the same dep agree on its url and ref, and the
#               lock's recorded url and ref agree with them. an entry resolved from
#               a ref no case asks for pins the wrong history.
#   stable      a `branch/` ref is `branch/main`. the suite builds against stable
#               releases, and `main` is the reviewed release event on every
#               briar-systems library (doc/cli.md's Lockfile section) where `dev` is
#               whatever landed an hour ago. tag/ and commit/ refs are already fixed
#               points and are left alone.
#
# STATIC AND FAST: no compiler, no build, no network - reads manifests and locks
# only, so it belongs in the same lightweight int-matrix job as
# check-target-matrix.sh and runs on every ordinary PR. main-cadence-only
# enforcement is no enforcement (#2353), which is precisely how #2729 survived.
set -eu

here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$here/deps.sh"

decls=$(mktemp)
trap 'rm -f "$decls"' EXIT
int_git_deps >"$decls"

fails=0
checked=$(awk 'NF' "$decls" | wc -l | tr -d ' ')

if [ "$checked" -eq 0 ]; then
    echo "check-deps.sh: no git dep declarations found - broken harness, not a finding" >&2
    exit 2
fi

while read -r name url ref case_id; do
    [ -n "$name" ] || continue

    if [ -z "$(pin_dep_commit "$name")" ]; then
        echo "FAIL $case_id: git dep '$name' has no entry in mach.lock or int/deps.lock" >&2
        echo "     a root-manifest dep is recorded by 'mach dep pull' at the repo root;" >&2
        echo "     an int-only dep by 'bash int/lib/update-deps.sh <compiler>'." >&2
        fails=$((fails + 1))
    fi

    case "$ref" in
        branch/main|tag/*|commit/*) : ;;
        *) echo "FAIL $case_id: git dep '$name' declares ref '$ref'; the suite builds against stable releases, so a branch ref must be branch/main" >&2
           fails=$((fails + 1)) ;;
    esac

    # one lock entry serves every case declaring the name, so they have to agree
    # about what the name means. compared against the FIRST declaration, which is
    # also the one reported in the message.
    first=$(awk -v n="$name" '$1 == n { print $2, $3; exit }' "$decls")
    first_case=$(awk -v n="$name" '$1 == n { print $4; exit }' "$decls")
    if [ "$url $ref" != "$first" ]; then
        echo "FAIL $case_id: git dep '$name' declares '$url $ref', but $first_case declares '$first'" >&2
        fails=$((fails + 1))
    fi
done <"$decls"

# the lock's own record has to be the one the cases ask for.
for name in $(awk '{ print $1 }' "$decls" | sort -u); do
    src=$(pin_dep_source "$name")
    [ -n "$src" ] || continue
    lock=$root_lock
    [ "$src" = "int/deps.lock" ] && lock=$int_lock
    want=$(awk -v n="$name" '$1 == n { print $2, $3; exit }' "$decls")
    want_case=$(awk -v n="$name" '$1 == n { print $4; exit }' "$decls")
    got="$(dep_field "$lock" "$name" url) $(dep_field "$lock" "$name" ref)"
    if [ "$got" != "$want" ]; then
        echo "FAIL $src: '$name' is locked at '$got' but $want_case declares '$want'" >&2
        fails=$((fails + 1))
    fi
done

for name in $(dep_names "$int_lock"); do
    if [ -n "$(lock_commit "$root_lock" "$name")" ]; then
        echo "FAIL int/deps.lock: '$name' is already recorded by the root mach.lock; int/deps.lock holds only deps the root manifest does not declare" >&2
        fails=$((fails + 1))
    fi
    if ! awk -v n="$name" '$1 == n { found = 1 } END { exit !found }' "$decls"; then
        echo "FAIL int/deps.lock: '$name' is recorded but no int case declares it; re-run update-deps.sh" >&2
        fails=$((fails + 1))
    fi
done

if [ "$fails" -ne 0 ]; then
    echo "check-deps: $fails problem(s) in the int dependency pins" >&2
    exit 1
fi
echo "check-deps: $checked git dep declaration(s) OK; pin sources mach.lock + int/deps.lock"
