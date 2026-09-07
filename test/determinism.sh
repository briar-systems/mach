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
#
# Every build destination is project-relative: `-o` rejects any path outside the
# project root, so a temporary directory cannot hold build outputs.
set -eu

die() {
    echo "::error::$*" >&2
    exit 1
}

cc=${1:-}
dir=${2:-.}
if [ -z "$cc" ]; then
    echo "usage: determinism.sh <compiler> [project-dir]" >&2
    exit 2
fi
[ -x "$cc" ] || die "compiler '$cc' is not an executable file"
cc=$(realpath "$cc")
[ -d "$dir" ] || die "project directory '$dir' does not exist"
cd "$dir"
root=$(pwd)
[ -f mach.toml ] || die "'$root' has no mach.toml; determinism.sh needs a mach project"

# `-o` destinations, project-relative because the planner rejects anything else
rel=out/determinism
work=$(mktemp -d)
restore_version=0
cleanup() {
    if [ "$restore_version" = 1 ]; then
        cp "$work/version.mach.bak" "$root/src/lang/version.mach" 2>/dev/null || true
    fi
    rm -rf "$work"
    rm -rf "$root/out/determinism"
}
trap cleanup EXIT

# build <destination-relative-path> <project-dir> <what>
build() {
    dest=$1
    proj=$2
    what=$3
    if ! "$cc" build "$proj" -o "$dest" >"$work/log" 2>&1; then
        echo "--- compiler output ---" >&2
        cat "$work/log" >&2
        die "$what: the build failed"
    fi
    [ -f "$proj/$dest" ] || die "$what: the build reported success but produced no '$proj/$dest'"
}

# a clean baseline, then a warm no-op rebuild over its populated out/
rm -rf "$root/out"
build "$rel/clean" "$root" "clean baseline"
build "$rel/warm_noop" "$root" "warm no-op rebuild"
cmp -s "$rel/clean" "$rel/warm_noop" \
    || die "warm no-op rebuild diverged from the clean build"

# source edit-invalidation: bump the compiler-owned release identity, then compare
# a warm rebuild against a clean rebuild of the same edit
grep -q '^pub val MACH_VERSION: str = ' src/lang/version.mach \
    || die "no compiler version constant to drive the invalidation edit"
cp src/lang/version.mach "$work/version.mach.bak"
restore_version=1
sed -i 's/^pub val MACH_VERSION: str = "\(.*\)";/pub val MACH_VERSION: str = "\1-det";/' src/lang/version.mach
grep -q -- '-det";$' src/lang/version.mach \
    || die "the version edit did not apply; the check would prove nothing"

cp "$rel/clean" "$work/clean.bin"
build "$rel/inc_edited" "$root" "incremental build after a source edit"
cp "$rel/inc_edited" "$work/inc_edited.bin"
rm -rf "$root/out"
build "$rel/clean_edited" "$root" "clean build after a source edit"

cp "$work/version.mach.bak" src/lang/version.mach
restore_version=0

cmp -s "$work/inc_edited.bin" "$rel/clean_edited" \
    || die "incremental build after an edit diverged from a clean build (stale invalidation)"

# guard against a false pass: the edit must actually have changed the output
if cmp -s "$work/clean.bin" "$rel/clean_edited"; then
    die "the source edit did not change the binary; the check proves nothing"
fi

# manifest edit-invalidation: keep source fixed while `$project.version` changes
# in a tiny static-library fixture, preserving the manifest-query regression.
# The fixture declares no dependency, so it exercises the manifest input channel
# without also depending on dependency realization.
proj="$work/project-version"
mkdir -p "$proj/src"
case "$(uname -m)" in
    x86_64)  isa=x86_64; abi=sysv64 ;;
    aarch64) isa=aarch64; abi=aapcs64 ;;
    riscv64) isa=riscv64; abi=lp64d ;;
    *) die "unsupported host architecture '$(uname -m)'" ;;
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
EOF
cat > "$proj/src/main.mach" <<'EOF'
#[symbol("det_version_byte")]
pub fun det_version_byte(i: u64) u8 {
    val v: *u8 = $project.version::*u8;
    ret v[i];
}
EOF

build "$rel/project_clean" "$proj" "fixture clean build"
cp "$proj/$rel/project_clean" "$work/project_clean.bin"

sed -i 's/^version = "1.0.0"/version = "1.0.1"/' "$proj/mach.toml"
grep -q '^version = "1.0.1"' "$proj/mach.toml" \
    || die "the manifest edit did not apply; the check would prove nothing"

build "$rel/project_inc" "$proj" "fixture incremental build after a manifest edit"
cp "$proj/$rel/project_inc" "$work/project_inc.bin"
rm -rf "$proj/out"
build "$rel/project_clean_edited" "$proj" "fixture clean build after a manifest edit"

cmp -s "$work/project_inc.bin" "$proj/$rel/project_clean_edited" \
    || die "manifest-only incremental build diverged from a clean build"
if cmp -s "$work/project_clean.bin" "$proj/$rel/project_clean_edited"; then
    die "the manifest edit did not change the fixture; the check proves nothing"
fi

echo "determinism: warm no-op and post-edit incremental builds both match clean"
echo "determinism: manifest-only incremental build matches clean"
