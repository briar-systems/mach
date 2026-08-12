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
# edit only partially). A compiler-source edit and a separate manifest-only
# fixture cover both invalidation channels without one masking the other.
set -eu

cc=${1:-}
dir=${2:-.}
if [ -z "$cc" ]; then
    echo "usage: determinism.sh <compiler> [project-dir]" >&2
    exit 2
fi
cc=$(realpath "$cc")
cd "$dir"
root=$(pwd)

work=$(mktemp -d)
trap 'rm -rf "$work" out; cp "$work/version.mach.bak" src/lang/version.mach 2>/dev/null || true' EXIT

# a clean baseline, then a warm no-op rebuild over its populated out/
rm -rf out
"$cc" build . -o "$work/clean"
"$cc" build . -o "$work/warm_noop"
if ! cmp -s "$work/clean" "$work/warm_noop"; then
    echo "::error::warm no-op rebuild diverged from the clean build"
    exit 1
fi

# source edit-invalidation: bump the compiler-owned release identity, then compare
# a warm rebuild against a clean rebuild of the same edit
cp src/lang/version.mach "$work/version.mach.bak"
if ! grep -q '^pub val MACH_VERSION: str = ' src/lang/version.mach; then
    echo "::error::no compiler version constant to drive the invalidation edit"
    exit 1
fi
sed -i 's/^pub val MACH_VERSION: str = "\(.*\)";/pub val MACH_VERSION: str = "\1-det";/' src/lang/version.mach

"$cc" build . -o "$work/inc_edited"
rm -rf out
"$cc" build . -o "$work/clean_edited"
cp "$work/version.mach.bak" src/lang/version.mach

if ! cmp -s "$work/inc_edited" "$work/clean_edited"; then
    echo "::error::incremental build after an edit diverged from a clean build (stale invalidation)"
    exit 1
fi

# guard against a false pass: the edit must actually have changed the output
if cmp -s "$work/clean" "$work/clean_edited"; then
    echo "::error::the source edit did not change the binary; the check proves nothing"
    exit 1
fi

# manifest edit-invalidation: keep source fixed while `$project.version` changes
# in a tiny static-library fixture, preserving the manifest-query regression
proj="$work/project-version"
mkdir -p "$proj/src"
case "$(uname -m)" in
    x86_64)  isa=x86_64; abi=sysv64 ;;
    aarch64) isa=aarch64; abi=aapcs64 ;;
    *) echo "::error::unsupported host architecture"; exit 2 ;;
esac
cat > "$proj/mach.toml" <<EOF
[project]
id = "det"
version = "1.0.0"
src = "src"
out = "out/{target.name}/{profile.name}"
[target.host]
isa = "$isa"
os = "linux"
abi = "$abi"
[profile.debug]
opt = 0
debug = false
simd = "scalarize"
[artifact.det]
kind = "static"
entry = "main.mach"
out = "lib/det"
targets = ["*"]
link = []
need = []
[dep.mach-std]
path = "$root/dep/mach-std"
EOF
cat > "$proj/src/main.mach" <<'EOF'
use std.types.string.str;

pub val VERSION: str = $project.version;
EOF

(cd "$proj" && "$cc" dep pull)
"$cc" build "$proj" -o "$work/project_clean"
sed -i 's/^version = "1.0.0"/version = "1.0.1"/' "$proj/mach.toml"
"$cc" build "$proj" -o "$work/project_inc"
rm -rf "$proj/out"
"$cc" build "$proj" -o "$work/project_clean_edited"
if ! cmp -s "$work/project_inc" "$work/project_clean_edited"; then
    echo "::error::manifest-only incremental build diverged from a clean build"
    exit 1
fi
if cmp -s "$work/project_clean" "$work/project_clean_edited"; then
    echo "::error::the manifest edit did not change the fixture; the check proves nothing"
    exit 1
fi

echo "determinism: warm no-op and post-edit incremental builds both match clean"
