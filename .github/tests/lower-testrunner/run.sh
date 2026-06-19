#!/usr/bin/env bash
# test-runner regression suite: each app exercises a `mach test` lowering bug
# from the 2026-06 audit (issue #1251) in the synthesized test runner.
#
#   nlabel:   a test whose label text contains `N<digit>`. the runner used to
#             recover the printable label by inverse-parsing the mangled linkage
#             name and locked onto the in-label `N3`, listing "foo" instead of
#             the real label. the label is now carried on the IR function, so
#             `--list` prints the full text.
#   duplabel: two tests sharing one label in a module. the clash used to surface
#             only at link time as "multiple definition" of an internal mangled
#             symbol; lowering now reports a source-located "duplicate test
#             label" diagnostic and the build fails cleanly.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
# absolutize a path argument: the suite cds into temp dirs, so a relative
# compiler path would break after the first cd.
case "$MACH" in */*) MACH="$(cd "$(dirname "$MACH")" && pwd)/$(basename "$MACH")";; esac
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$HERE/../../.." && pwd)"
STD="$REPO_ROOT/dep/mach-std"

if [ ! -d "$STD/src" ]; then
    echo "error: vendored std not found at $STD (run: mach dep pull)" >&2
    exit 2
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

fail=0

# stage <app-dir-name>: copy the app into a temp dir and vendor std into it,
# echoing the staged path.
stage() {
    local app="$1"; local dst="$WORK/$app"
    cp -r "$HERE/$app" "$dst"
    mkdir -p "$dst/dep"
    ln -s "$STD" "$dst/dep/mach-std"
    echo "$dst"
}

# nlabel: `--list` must print the full label, not the inverse-mangled fragment.
dst="$(stage nlabel)"
out="$( ( cd "$dst" && "$MACH" test . --list ) 2>&1 )" || true
if echo "$out" | grep -qF 'check N3foo parsing'; then
    echo "PASS lower-testrunner/nlabel"
else
    echo "FAIL lower-testrunner/nlabel: --list did not print the full label" >&2
    echo "$out" >&2
    fail=1
fi

# duplabel: the build must fail with a source-located duplicate-label diagnostic
# rather than succeeding or emitting a mangled link error.
dst="$(stage duplabel)"
set +e
out="$( ( cd "$dst" && "$MACH" build . ) 2>&1 )"
code=$?
set -e
if [ "$code" -eq 0 ]; then
    echo "FAIL lower-testrunner/duplabel: build succeeded, expected a duplicate-label error" >&2
    fail=1
elif echo "$out" | grep -qF 'duplicate test label'; then
    echo "PASS lower-testrunner/duplabel"
else
    echo "FAIL lower-testrunner/duplabel: build failed without the duplicate-label diagnostic" >&2
    echo "$out" >&2
    fail=1
fi

exit "$fail"
