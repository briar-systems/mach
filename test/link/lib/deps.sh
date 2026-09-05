#!/usr/bin/env bash
# deps.sh — the one reader of dependency declarations and of the repo's own pins.
#
# THE PIN SOURCE IS THE REPO'S OWN GITLINKS, and only that. there is no lock file:
# the committed gitlink under this repository's `dep/` is the record (R-DEP-4), so
# every git dep any case declares is one the compiler in the same checkout was also
# built against, and bumping the compiler's dependency bumps the suite's at the same
# moment. a second record beside this suite would be a second place to bump and
# could rot against the compiler's, which is what #2592 wanted ruled out.
#
# that this holds is checked rather than assumed: run.sh refuses to start a pinned
# run when a case declares a git dep this repository does not realize. a dep the
# root manifest has no reason to declare cannot be pinned from here, and a case
# reaching for one is reaching over the network for something outside the checkout -
# which is how an upstream rename turned every open PR red once already (#2831).
#
# resolution itself is never done here. `mach dep pull` maps a ref to a commit; this
# file only ever reads what a checkout ended up at, and seeds later cases from the
# checkout the first case produced so one run means one commit (#2619). a second
# implementation of that mapping is the drifting-parallel-enumeration shape this
# repo treats as a defect family.

_deps_lib_dir=$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)

deps_repo_root=$(CDPATH= cd -- "$_deps_lib_dir/../../.." && pwd)

# dep_field <toml> <name> <key> — the value of <key> inside the [dep.<name>] table
# of a mach.toml, or empty when the file, the table, or the key is absent.
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

# dep_names <toml> — the names of the [dep.<name>] tables a mach.toml declares.
dep_names() {
    [ -f "$1" ] || return 0
    awk '/^\[dep\./ { n = $0; sub(/^\[dep\./, "", n); sub(/\]$/, "", n); print n }' "$1"
}

# case_deps <case-dir> — the names of the [dep.<name>] tables a case declares.
case_deps() {
    dep_names "$1/mach.toml"
}

# pin_dep_commit <name> — the commit this repository's own gitlink records for
# dep/<name>, empty when it realizes no such dependency.
pin_dep_commit() {
    git -C "$deps_repo_root" rev-parse "HEAD:dep/$1" 2>/dev/null || true
}

# pin_dep_source <name> — this repository's own checkout of <name>, which holds the
# objects a case's copy is cloned from, so a pinned run reaches no network at all.
pin_dep_source() {
    printf '%s/dep/%s' "$deps_repo_root" "$1"
}

# case_dirs — every case directory holding a mach.toml, in a stable order.
case_dirs() {
    for dir in "$_deps_lib_dir"/../cases/*/; do
        dir=${dir%/}
        [ -f "$dir/mach.toml" ] || continue
        echo "$dir"
    done
}

# unpinnable_deps — one "<case> <dep> <url>" line per git dep declaration this
# repository does not itself realize. per DECLARATION, not per distinct dep: naming
# every case that reaches for an unpinnable dep is the diagnostic, and collapsing
# here would report one of them and hide the rest. a `path =` dep is a directory
# inside this repo, already as reproducible as the checkout, and has no commit.
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

# seed_case_dep <case-dir> <name> <source-checkout> <commit> <declared-url> — put an
# already-resolved dependency into a case, by cloning the local checkout that holds
# it and detaching at the recorded commit. no network, and no second ref resolution:
# the commit was decided once, by mach.
seed_case_dep() {
    _sd_dir=$1; _sd_name=$2; _sd_src=$3; _sd_commit=$4; _sd_url=$5
    [ -d "$_sd_src" ] || return 1
    [ -n "$_sd_commit" ] || return 1
    rm -rf "$_sd_dir/dep/$_sd_name"
    mkdir -p "$_sd_dir/dep"
    git clone -q --no-checkout "$_sd_src" "$_sd_dir/dep/$_sd_name" >/dev/null 2>&1 || return 1
    git -C "$_sd_dir/dep/$_sd_name" checkout -q --detach "$_sd_commit" >/dev/null 2>&1 || return 1
    git -C "$_sd_dir/dep/$_sd_name" remote set-url origin "$_sd_url" >/dev/null 2>&1 || return 1
    return 0
}

# dep_commits <case-dir> [full] — "name@sha" per git dep checked out under a case's
# dep/, ground-truthed from the CHECKOUT, which is the entity that was compiled
# against (#2387).
dep_commits() {
    dir=$1
    fmt=${2:-short}
    [ -d "$dir/dep" ] || return 0
    for depdir in "$dir"/dep/*/; do
        [ -d "${depdir}.git" ] || [ -f "${depdir}.git" ] || continue
        name=$(basename "$depdir")
        full=$(git -C "$depdir" rev-parse HEAD 2>/dev/null) || continue
        short=$(git -C "$depdir" rev-parse --short HEAD 2>/dev/null) || continue
        if [ "$fmt" = full ]; then head=$full; else head=$short; fi
        printf '%s@%s ' "$name" "$head"
    done
}
