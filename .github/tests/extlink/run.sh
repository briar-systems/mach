#!/usr/bin/env bash
# extlink integration test: link a Mach program against an external precompiled
# `.o` and confirm it builds and runs with the correct result.
#
# builds a tiny library (`extlib`) to a loose object exporting `mach_ext_add`,
# then links a consumer (`app`) — which declares that symbol `ext` and calls it
# — against the object via each supported surface (explicit path, `-L`/`-l`, and
# the `[targets.*].libs` manifest field). the program returns `mach_ext_add(20,
# 21)` which must be 42. a build with no external input must fail on the
# undefined `ext` symbol, proving the link is what supplies the definition.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$HERE/../../.." && pwd)"
STD="$REPO_ROOT/dep/mach-std"

if [ ! -d "$STD/src" ]; then
    echo "error: vendored std not found at $STD (run: git submodule update --init)" >&2
    exit 2
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp -r "$HERE/extlib" "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"

# build the external library object.
( cd "$WORK/extlib" && "$MACH" build . )
OBJ="$WORK/extlib/out/linux/obj/extlib/ext.o"
test -f "$OBJ" || { echo "error: library object not produced at $OBJ" >&2; exit 1; }

# a `lib<name>.o` copy so `-L`/`-l` resolution has something to find.
mkdir -p "$WORK/libs"
cp "$OBJ" "$WORK/libs/libextadd.o"

fail=0

expect_42() {
    local desc="$1"; shift
    rm -rf "$WORK/app/out"
    if ! ( cd "$WORK/app" && "$MACH" build . "$@" ) >/dev/null 2>&1; then
        echo "FAIL extlink: $desc — build failed" >&2
        fail=1
        return
    fi
    set +e
    "$WORK/app/out/linux/bin/extapp"
    local code=$?
    set -e
    if [ "$code" -ne 42 ]; then
        echo "FAIL extlink: $desc — ran with exit $code, expected 42" >&2
        fail=1
        return
    fi
    echo "PASS extlink: $desc"
}

expect_42 "explicit object path" "$OBJ"
expect_42 "-L dir -l name"       -L "$WORK/libs" -l extadd

# manifest libs: point `[targets.linux].libs` at the object by absolute path.
sed "s|^binary = \"linux/bin/extapp\"|&\nlibs = [\"$OBJ\"]|" "$HERE/app/mach.toml" > "$WORK/app/mach.toml"
expect_42 "[targets.*].libs manifest"

# no external input: the undefined `ext` symbol must make the link fail.
cp "$HERE/app/mach.toml" "$WORK/app/mach.toml"
rm -rf "$WORK/app/out"
if ( cd "$WORK/app" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "FAIL extlink: missing input did not fail the link" >&2
    fail=1
else
    echo "PASS extlink: missing external input fails the link"
fi

exit "$fail"
