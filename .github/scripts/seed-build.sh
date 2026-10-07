#!/usr/bin/env bash
# builds this compiler with a 6.x seed, which reads only the previous major's
# manifest keys. the seed builds a staged copy, out/seed: the source, std at the
# committed pin as a path dependency, and both manifests rewritten to the
# previous keys. a compatibility path listed on #4490, removed once the seed is
# a 7.x release
#
# usage: seed-build.sh <seed> <output> [build options...]
set -euo pipefail

seed=${1:?seed-build.sh needs the seed compiler}
output=${2:?seed-build.sh needs an output path}
shift 2
case "$seed" in
    */*) seed="$(cd "$(dirname "$seed")" && pwd)/$(basename "$seed")" ;;
    *) seed=$(command -v "$seed") ;;
esac

# [project] work as out, {project.work} as {project.out}, and a profile's
# optimize as opt with vectorize and float_reassoc spelled out
previous() {
    if grep -qE '^(pass|skip|relax)[[:space:]]*=' "$1"; then
        echo "seed-build.sh: $1 sets a pass, skip or relax list, which the seed cannot read" >&2
        return 1
    fi
    awk '
        /^work[[:space:]]*=/ { sub(/^work/, "out") }
        { gsub(/\{project\.work\}/, "{project.out}") }
        /^optimize[[:space:]]*=[[:space:]]*(true|false)/ {
            level = 0
            if ($0 ~ /true/) { level = 2 }
            print "opt = " level
            print "vectorize = true"
            print "float_reassoc = false"
            next
        }
        /^\[dep\.std\]/ { print; print "path = \"std\""; dep = 1; next }
        /^\[/ { dep = 0 }
        dep && /^(git|ref|version)[[:space:]]*=/ { next }
        { print }
    ' "$1"
}

url=$(awk '/^\[dep\.std\]/ { d = 1; next } /^\[/ { d = 0 } d && /^git[[:space:]]*=/ { sub(/^git[[:space:]]*=[[:space:]]*"/, ""); sub(/".*$/, ""); print }' mach.toml)
pin=$(git rev-parse HEAD:dep/std)

rm -rf out/seed
mkdir -p out/seed
cp -R src out/seed/src
git clone --quiet "$url" out/seed/std
git -C out/seed/std checkout --quiet "$pin"
previous mach.toml >out/seed/mach.toml
previous out/seed/std/mach.toml >out/seed/std/mach.toml.previous
mv out/seed/std/mach.toml.previous out/seed/std/mach.toml

name=$(basename "$output")
(cd out/seed && "$seed" dep pull . && "$seed" build . -o "$name" "$@")
mv "out/seed/$name" "$output"
