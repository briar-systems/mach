#!/usr/bin/env bash
# vlib.sh [out]: writes vlib.bundle (repository vlib, tags v0.1.0 and v0.2.0) beside this script or to out, which steps reads.
# v0.1.0 is written for mach 6: it states `mach = "^6"` and a profile key mach 7 removed
set -eu
here=$(cd "$(dirname "$0")" && pwd)
out=${1:-$here/vlib.bundle}
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
cd "$work"
# fixed identity and dates, so the commit ids stay 14375e940f03 (v0.1.0) and 284ebe7033d0 (v0.2.0)
export GIT_AUTHOR_NAME=t GIT_AUTHOR_EMAIL=t@t GIT_COMMITTER_NAME=t GIT_COMMITTER_EMAIL=t@t
export GIT_AUTHOR_DATE='1795309889 -0500' GIT_COMMITTER_DATE='1795309889 -0500'
export GIT_CONFIG_GLOBAL=/dev/null GIT_CONFIG_NOSYSTEM=1

manifest() {
    cat > mach.toml <<TOML
[project]
id      = "vlib"
version = "$1"
mach    = "$2"
src     = "src"
work    = "out/{target.name}/{profile.name}"

[artifact.vlib]
kind    = "static"
entry   = "lib/vlib.mach"
out     = "{project.work}/lib/libvlib{artifact.suffix}"
targets = ["*"]

[profile.debug]
$3
debug    = true
simd     = "scalarize"
TOML
    printf '# a value the project exports\npub fun value() i64 {\n    ret %s;\n}\n' "$4" > src/lib/vlib.mach
}

git init -q -b main
mkdir -p src/lib
manifest 0.1.0 '^6' 'opt      = 0' 1
git add -A
git commit -q -m "vlib 0.1.0"
git tag v0.1.0
manifest 0.2.0 '^7' 'optimize = false' 2
git commit -q -a -m "vlib 0.2.0"
git tag v0.2.0
git bundle create -q "$out" --all
