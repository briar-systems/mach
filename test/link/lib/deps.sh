#!/usr/bin/env bash
# deps.sh — the one reader of dependency declarations and lock files.
#
# THE PIN SOURCE IS THE REPO'S OWN mach.lock, and only that. every git dep any case
# declares is one the compiler in the same checkout was also built against, so the
# root lock already records it and bumping the compiler's dependency bumps the
# suite's at the same moment. a second lock beside this suite would be a second
# place to bump and could rot against the compiler's, which is what #2592 wanted
# ruled out.
#
# that this holds is checked rather than assumed: run.sh refuses to start a pinned
# run when a case declares a git dep the root lock does not record. a dep the root
# manifest has no reason to declare cannot be pinned from here, and a case reaching
# for one is reaching over the network for something outside the checkout - which is
# how an upstream rename turned every open PR red once already (#2831).
#
# resolution itself is never done here. `mach dep pull` maps a ref to a commit and
# this file only ever reads what it recorded. a second implementation of that
# mapping is the drifting-parallel-enumeration shape this repo treats as a defect
# family.

_deps_lib_dir=$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)

root_lock=$_deps_lib_dir/../../../mach.lock

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

# pin_dep_commit <name> — the commit the root lock records for <name>, empty when it
# records none.
pin_dep_commit() {
    lock_commit "$root_lock" "$1"
}

# case_dirs — every case directory holding a mach.toml, in a stable order.
case_dirs() {
    for dir in "$_deps_lib_dir"/../cases/*/; do
        dir=${dir%/}
        [ -f "$dir/mach.toml" ] || continue
        echo "$dir"
    done
}

# unpinnable_deps — one "<case> <dep> <url>" line per git dep declaration the root
# lock does not record. per DECLARATION, not per distinct dep: naming every case
# that reaches for an unpinnable dep is the diagnostic, and collapsing here would
# report one of them and hide the rest. a `path =` dep is a directory inside this
# repo, already as reproducible as the checkout, and has no commit to record.
unpinnable_deps() {
    for dir in $(case_dirs); do
        for name in $(case_deps "$dir"); do
            url=$(dep_field "$dir/mach.toml" "$name" git)
            [ -n "$url" ] || continue
            [ -n "$(pin_dep_commit "$name")" ] && continue
            printf '%s %s %s\n' "$(basename "$dir")" "$name" "$url"
        done
    done
}
