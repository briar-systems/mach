#!/usr/bin/env bash
# decorator integration test (#1476): prove a `symbol` backtick decorator drives
# the emitted link name, the 1:1 replacement for the legacy `$<sym>.symbol`
# setter.
#
# builds a tiny library (`declib`) to a loose object whose `pub fun mach_dec_add`
# carries a `` `symbol("mach_dec_add")` `` decorator, then links a consumer
# (`app`) — which declares that symbol `ext` and whose `main` carries a
# `` `symbol("main")` `` decorator — against the object. the program returns
# `mach_dec_add(20, 21)` which must be 42. if the decorator did not route the
# link name, the library symbol would be mangled and the `ext` reference would
# not resolve, failing the link — so a clean 42 proves emission followed the
# decorator. a build with no external input must fail on the undefined symbol.
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
OBJ="$WORK/extlib/out/linux/obj/declib/ext.o"
test -f "$OBJ" || { echo "error: library object not produced at $OBJ" >&2; exit 1; }

fail=0

# explicit object path: link the consumer against the decorator-named symbol.
rm -rf "$WORK/app/out"
if ! ( cd "$WORK/app" && "$MACH" build . "$OBJ" ) >/dev/null 2>&1; then
    echo "FAIL decorator: symbol decorator link — build failed" >&2
    fail=1
else
    set +e
    "$WORK/app/out/linux/bin/decapp"
    code=$?
    set -e
    if [ "$code" -ne 42 ]; then
        echo "FAIL decorator: symbol decorator link — ran with exit $code, expected 42" >&2
        fail=1
    else
        echo "PASS decorator: symbol decorator drives the emitted link name"
    fi
fi

# no external input: the undefined symbol must make the link fail, proving the
# definition comes from the linked object, not the graph.
rm -rf "$WORK/app/out"
if ( cd "$WORK/app" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "FAIL decorator: missing input did not fail the link" >&2
    fail=1
else
    echo "PASS decorator: missing external input fails the link"
fi

exit "$fail"
