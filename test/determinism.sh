#!/usr/bin/env bash
# determinism.sh: prove the incremental (warm) build path is deterministic.
#
# usage: determinism.sh <compiler> [project-dir]
#
# The clean self-host fixpoint (b == c) that the build lanes run only exercises
# from-scratch builds. This guards the warm/incremental path the query engine
# drives: a warm rebuild must produce byte-identical output to a clean build, both
# with no change (cache reuse is sound) and after a source edit (invalidation is
# sound, the failure mode #2045 recorded, where an incremental build reflected an
# edit only partially). The edit is the release version bump: it updates both the
# Mach-owned compiler version and the matching project manifest value.
set -eu

cc=${1:-}
dir=${2:-.}
if [ -z "$cc" ]; then
    echo "usage: determinism.sh <compiler> [project-dir]" >&2
    exit 2
fi
cc=$(realpath "$cc")
cd "$dir"

work=$(mktemp -d)
trap 'rm -rf "$work" out; cp "$work/mach.toml.bak" mach.toml 2>/dev/null || true; cp "$work/version.mach.bak" src/lang/version.mach 2>/dev/null || true' EXIT

# a clean baseline, then a warm no-op rebuild over its populated out/
rm -rf out
"$cc" build . -o "$work/clean"
"$cc" build . -o "$work/warm_noop"
if ! cmp -s "$work/clean" "$work/warm_noop"; then
    echo "::error::warm no-op rebuild diverged from the clean build"
    exit 1
fi

# edit-invalidation: bump the release identity, then compare a warm rebuild
# against a clean rebuild of the same edit
cp mach.toml "$work/mach.toml.bak"
cp src/lang/version.mach "$work/version.mach.bak"
if ! grep -q '^version = ' mach.toml; then
    echo "::error::no [project].version line to drive the invalidation edit"
    exit 1
fi
if ! grep -q '^pub val MACH_VERSION: str = ' src/lang/version.mach; then
    echo "::error::no compiler version constant to drive the invalidation edit"
    exit 1
fi
sed -i 's/^version = "\(.*\)"/version = "\1-det"/' mach.toml
sed -i 's/^pub val MACH_VERSION: str = "\(.*\)";/pub val MACH_VERSION: str = "\1-det";/' src/lang/version.mach

"$cc" build . -o "$work/inc_edited"
rm -rf out
"$cc" build . -o "$work/clean_edited"
cp "$work/mach.toml.bak" mach.toml
cp "$work/version.mach.bak" src/lang/version.mach

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
