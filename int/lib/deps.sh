#!/usr/bin/env bash
# deps.sh — the one reader of dependency declarations and lock files, shared by
# run.sh, check-deps.sh and update-deps.sh.
#
# WHICH SET EACH LOCK COVERS. the repo root's mach.lock is written by `mach dep
# pull` from the ROOT manifest, so it covers the compiler's own dependency closure
# and nothing else. int builds a strictly larger set: 46 cases declare mach-std,
# which the root also declares, and the two SPIR-V cases declare mach-shader, which
# the root has no reason to. #2729 is that gap - `--deps pin` resolved every case
# against the root lock alone, so the one lane that exists to be reproducible could
# not start at all, and it was never a stale revision a refresh would fix.
#
# so the pin source is TWO locks with disjoint coverage, in this order:
#
#   mach.lock       the root's own, authoritative for every dep the compiler also
#                   builds against. bumping the compiler's dependency bumps int's
#                   at the same time, which is the property #2592 wanted and the
#                   reason int does not keep its own copy of mach-std.
#
#   int/deps.lock   the deps only int builds. one entry per dep the root manifest
#                   does not declare, written by update-deps.sh from what `mach dep
#                   pull` resolved, never by hand.
#
# disjoint is enforced, not assumed: check-deps.sh fails if int/deps.lock carries a
# name the root lock already carries, because then a dep would have two places to
# bump and the two could disagree about what a run was pinned to.
#
# resolution itself is never done here. update-deps.sh asks the compiler and records
# the answer; run.sh only ever reads a recorded one. a second implementation of
# ref-to-commit mapping is the drifting-parallel-enumeration shape this repo already
# treats as a defect family.

_deps_lib_dir=$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)

# the two pin sources, in lookup order.
root_lock=$_deps_lib_dir/../../mach.lock
int_lock=$_deps_lib_dir/../deps.lock

# dep_field <toml> <name> <key> — the value of <key> inside the [dep.<name>] table
# of a mach.toml or mach.lock, or empty when the file, the table, or the key is
# absent. one reader for both files: they spell the same table differently (a
# manifest's `git =` is a lock's `url =`) but the table structure is identical, and a
# second parser for it is the drifting-parallel-enumeration shape this repo already
# treats as a defect family (see case.sh's header).
dep_field() {
    [ -f "$1" ] || return 0
    awk -v want="$2" -v key="$3" '
        /^\[/ { in_dep = 0 }
        /^\[dep\./ {
            name = $0; sub(/^\[dep\./, "", name); sub(/\]$/, "", name)
            in_dep = (name == want); next
        }
        in_dep && $1 == key && $2 == "=" {
            v = $0; sub(/^[^=]*=[[:space:]]*"/, "", v); sub(/"[[:space:]]*$/, "", v)
            print v; exit
        }
    ' "$1"
}

# dep_names <toml> — the names of the [dep.<name>] tables a mach.toml or mach.lock
# declares, in file order.
dep_names() {
    [ -f "$1" ] || return 0
    awk '/^\[dep\./ { n = $0; sub(/^\[dep\./, "", n); sub(/\]$/, "", n); print n }' "$1"
}

# lock_commit <lock> <name> — the commit a lock records for [dep.<name>].
lock_commit() {
    dep_field "$1" "$2" commit
}

# case_deps <case-dir> — the names of the [dep.<name>] tables a case declares.
case_deps() {
    dep_names "$1/mach.toml"
}

# pin_dep_commit <name> — the commit the pin sources record for <name>, root lock
# first. empty when neither records it, which is the condition check-deps.sh exists
# to catch before anyone runs the pinned lane.
pin_dep_commit() {
    commit=$(lock_commit "$root_lock" "$1")
    if [ -n "$commit" ]; then echo "$commit"; return 0; fi
    lock_commit "$int_lock" "$1"
}

# pin_dep_source <name> — the lock pin_dep_commit would take <name> from, as a path
# relative to the repo root, or empty when neither has it.
pin_dep_source() {
    if [ -n "$(lock_commit "$root_lock" "$1")" ]; then echo "mach.lock"; return 0; fi
    if [ -n "$(lock_commit "$int_lock" "$1")" ]; then echo "int/deps.lock"; fi
}

# int_case_dirs — every case directory holding a mach.toml, in a stable order.
int_case_dirs() {
    for dir in "$_deps_lib_dir"/../surface/*/ "$_deps_lib_dir"/../regression/*/; do
        dir=${dir%/}
        [ -f "$dir/mach.toml" ] || continue
        echo "$dir"
    done
}

# int_git_deps — one "<name> <url> <ref> <case-id>" line per git dep DECLARATION
# across every int case, not per distinct dep: two cases naming mach-shader produce
# two lines. check-deps.sh needs every declaration to compare them against each
# other, and collapsing here would hide exactly the disagreement it looks for. a
# `path =` dep is a directory inside this repo, already as reproducible as the
# checkout, and has no commit to record.
int_git_deps() {
    for dir in $(int_case_dirs); do
        case_id=${dir#"$_deps_lib_dir"/../}
        for name in $(case_deps "$dir"); do
            url=$(dep_field "$dir/mach.toml" "$name" git)
            [ -n "$url" ] || continue
            printf '%s %s %s %s\n' \
                "$name" "$url" "$(dep_field "$dir/mach.toml" "$name" ref)" "$case_id"
        done
    done
}
