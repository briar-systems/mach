#!/usr/bin/env bash
# check-determinism.sh — prove the incremental (warm) build path is deterministic.
#
# usage: check-determinism.sh <compiler> [project-dir]
#
# The clean self-host fixpoint (b == c) that the build lanes run only exercises
# from-scratch builds. This guards the warm/incremental path the query engine
# drives: a warm rebuild must produce byte-identical output to a clean build, both
# with no change (cache reuse is sound) and after a source edit (invalidation is
# sound — the failure mode #2045 recorded, where an incremental build reflected an
# edit only partially). The edit is a `[project].version` bump: it flows through
# `$project.version` into the compiler and rides the comptime-input invalidation
# path, the subtlest one to get right.
set -eu

cc=${1:-}
dir=${2:-.}
if [ -z "$cc" ]; then
    echo "usage: check-determinism.sh <compiler> [project-dir]" >&2
    exit 2
fi
cc=$(realpath "$cc")
cd "$dir"

work=$(mktemp -d)
trap 'rm -rf "$work" out; cp "$work/mach.toml.bak" mach.toml 2>/dev/null || true' EXIT

# a clean baseline, then a warm no-op rebuild over its populated out/
rm -rf out
"$cc" build . -o "$work/clean"
"$cc" build . -o "$work/warm_noop"
if ! cmp -s "$work/clean" "$work/warm_noop"; then
    echo "::error::warm no-op rebuild diverged from the clean build"
    exit 1
fi

# edit-invalidation: bump [project].version, which $project.version folds into the
# compiler, then compare a warm rebuild against a clean rebuild of the same edit
cp mach.toml "$work/mach.toml.bak"
if ! grep -q '^version = ' mach.toml; then
    echo "::error::no [project].version line to drive the invalidation edit"
    exit 1
fi
sed -i 's/^version = "\(.*\)"/version = "\1-det"/' mach.toml

"$cc" build . -o "$work/inc_edited"
rm -rf out
"$cc" build . -o "$work/clean_edited"
cp "$work/mach.toml.bak" mach.toml

if ! cmp -s "$work/inc_edited" "$work/clean_edited"; then
    echo "::error::incremental build after an edit diverged from a clean build (stale invalidation)"
    exit 1
fi

# guard against a false pass: the edit must actually have changed the output
if cmp -s "$work/clean" "$work/clean_edited"; then
    echo "::error::the version edit did not change the binary; the check proves nothing"
    exit 1
fi

echo "determinism: warm no-op and post-edit incremental builds both match clean"
