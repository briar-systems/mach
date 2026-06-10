#!/usr/bin/env bash
# extlink integration test: link a Mach program against external precompiled
# code — a loose `.o` object and a static `.a` archive — and confirm it builds
# and runs with the correct result.
#
# builds a tiny library (`extlib`) to a loose object exporting `mach_ext_add`,
# then links a consumer (`app`) — which declares that symbol `ext` and calls it
# — against the object via each supported surface (explicit path, `-L`/`-l`, and
# the `[targets.*].libs` manifest field). the same surfaces are then exercised
# against a `.a` archive of that object (built with `ar`, skipped when `ar` is
# absent). the program returns `mach_ext_add(20, 21)` which must be 42. a build
# with no external input must fail on the undefined `ext` symbol, proving the
# link is what supplies the definition.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
# absolutize a path argument: the suites cd into temp dirs, so a relative
# compiler path would break after the first cd.
case "$MACH" in */*) MACH="$(cd "$(dirname "$MACH")" && pwd)/$(basename "$MACH")";; esac
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

# a `lib<name>.o` copy so `-L`/`-l` resolution has a loose object to find.
mkdir -p "$WORK/libs"
cp "$OBJ" "$WORK/libs/libextadd.o"

# a static archive of the object, in its own dir so `-l` resolves it as a `.a`
# (a loose `.o` in the search path would otherwise win first).
AR=""
if command -v ar >/dev/null 2>&1; then
    mkdir -p "$WORK/alibs"
    ar rcs "$WORK/alibs/libextadd.a" "$OBJ"
    AR="$WORK/alibs/libextadd.a"
fi

fail=0

# expect_42 <desc> <build args...> — the build args are passed verbatim, so each
# case supplies its own project-path positional (`.`, `./`, ...).
expect_42() {
    local desc="$1"; shift
    rm -rf "$WORK/app/out"
    if ! ( cd "$WORK/app" && "$MACH" build "$@" ) >/dev/null 2>&1; then
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

# loose `.o` surfaces.
expect_42 "explicit object path" . "$OBJ"
expect_42 "-L dir -l name"       . -L "$WORK/libs" -l extadd
# a `./`-style path positional is the project root, not an object input — it
# must be skipped by the link-input scan even though it ends in '/'.
expect_42 "project-path positional skipped" ./ -L "$WORK/libs" -l extadd

# manifest libs: point `[targets.linux].libs` at the object by absolute path.
sed "s|^binary = \"linux/bin/extapp\"|&\nlibs = [\"$OBJ\"]|" "$HERE/app/mach.toml" > "$WORK/app/mach.toml"
expect_42 "[targets.*].libs manifest" .
cp "$HERE/app/mach.toml" "$WORK/app/mach.toml"

# static `.a` archive surfaces — the archive's member object supplies the
# definition exactly as the loose object does.
if [ -n "$AR" ]; then
    expect_42 "explicit archive path"        . "$AR"
    expect_42 "-L dir -l name (.a)"          . -L "$WORK/alibs" -l extadd
    sed "s|^binary = \"linux/bin/extapp\"|&\nlibs = [\"$AR\"]|" "$HERE/app/mach.toml" > "$WORK/app/mach.toml"
    expect_42 "[targets.*].libs manifest (.a)" .
    cp "$HERE/app/mach.toml" "$WORK/app/mach.toml"
else
    echo "SKIP extlink: .a archive cases — 'ar' not available"
fi

# no external input: the undefined `ext` symbol must make the link fail.
rm -rf "$WORK/app/out"
if ( cd "$WORK/app" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "FAIL extlink: missing input did not fail the link" >&2
    fail=1
else
    echo "PASS extlink: missing external input fails the link"
fi

exit "$fail"
