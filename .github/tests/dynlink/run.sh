#!/usr/bin/env bash
# dynlink integration test: dynamically link a Mach program against a shared
# library (libc) and confirm it builds, loads, and runs with the right result.
#
# the consumer (`app`) declares libc's `getpid` `ext` and calls it; no in-graph
# definition exists, so the symbol can only be satisfied by a dynamic import. it
# is linked against `libc` (which resolves to `libc.so.6`) through each supported
# surface — `-l c` and the `[targets.*].libs` manifest field — producing a
# dynamically-linked ELF (PT_INTERP + .dynamic/PLT/GOT). the program returns 0
# only when `getpid()` returns a positive pid, proving the import resolved and
# the PLT call ran at load time. a build with no shared dependency must fail on
# the undefined `ext`, proving the dynamic link is what supplies the binding.
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

# the linux dynamic loader and libc must be present for a dynamic link to run.
if [ ! -e /lib64/ld-linux-x86-64.so.2 ] && [ ! -e /usr/lib64/ld-linux-x86-64.so.2 ]; then
    echo "SKIP dynlink: no /lib64/ld-linux-x86-64.so.2 dynamic loader"
    exit 0
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"

fail=0

# expect_0 <desc> <build args...> — the build args are passed verbatim; each case
# supplies its own project-path positional (`.`).
expect_0() {
    local desc="$1"; shift
    rm -rf "$WORK/app/out"
    if ! ( cd "$WORK/app" && "$MACH" build "$@" ) >/dev/null 2>&1; then
        echo "FAIL dynlink: $desc — build failed" >&2
        fail=1
        return
    fi
    local bin="$WORK/app/out/linux/bin/dynapp"
    # confirm the binary is actually dynamically linked (has an interpreter).
    if command -v readelf >/dev/null 2>&1; then
        if ! readelf -l "$bin" 2>/dev/null | grep -q "program interpreter"; then
            echo "FAIL dynlink: $desc — produced a static binary (no PT_INTERP)" >&2
            fail=1
            return
        fi
    fi
    set +e
    "$bin"
    local code=$?
    set -e
    if [ "$code" -ne 0 ]; then
        echo "FAIL dynlink: $desc — ran with exit $code, expected 0" >&2
        fail=1
        return
    fi
    echo "PASS dynlink: $desc"
}

# `-l c` resolves libc.so.6 and binds getpid dynamically.
expect_0 "-l c (libc.so.6)" . -l c

# manifest libs: `[targets.linux].libs = ["c"]` resolves the same shared lib.
sed 's|^binary = "linux/bin/dynapp"|&\nlibs = ["c"]|' "$HERE/app/mach.toml" > "$WORK/app/mach.toml"
expect_0 "[targets.*].libs manifest" .
cp "$HERE/app/mach.toml" "$WORK/app/mach.toml"

# no shared dependency: the undefined `ext getpid` must make the link fail.
rm -rf "$WORK/app/out"
if ( cd "$WORK/app" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "FAIL dynlink: missing -l c did not fail the link on undefined getpid" >&2
    fail=1
else
    echo "PASS dynlink: missing shared dependency fails the link"
fi

exit "$fail"
