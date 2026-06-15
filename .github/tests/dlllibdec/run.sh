#!/usr/bin/env bash
# dlllibdec integration test (#1476): the decorator-spelled twin of dlllib —
# prove `library(...)` / `symbol(...)` backtick decorators drive per-symbol
# windows DLL attribution identically to the legacy `$<sym>.library` /
# `$<sym>.symbol` setters. the acceptance-criteria case: a windows extern with
# `` `library("ws2_32.dll")` `` + `` `symbol("...")` `` importing correctly.
#
# builds the windows binary, reads its PE import directory, and asserts each
# renamed symbol lands under the decorator-named DLL (and the mach identifiers do
# not leak), runs it under wine, and confirms a `library` naming an undeclared
# DLL fails the link.
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

cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"
APP="$WORK/app"

fail=0

# build the windows binary (the app's manifest targets windows).
if ! ( cd "$APP" && "$MACH" build . ) >"$WORK/build.out" 2>&1; then
    echo "FAIL dlllibdec: windows build failed" >&2
    cat "$WORK/build.out" >&2
    exit 1
fi
EXE="$APP/out/windows/bin/dlllibdec.exe"
test -f "$EXE" || { echo "error: windows binary not produced at $EXE" >&2; exit 1; }

# imports_under <dll> — the member names imported from <dll>, one per line, read
# from the emitted PE's import directory by whichever inspector is available.
imports_under() {
    case "$INSPECT" in
    objdump)
        objdump -x "$EXE" 2>/dev/null | awk -v dll="$1" '
            /DLL Name:/      { cur=$3; next }
            /^[[:space:]]*$/ { cur=""; next }
            cur=="" || cur!=dll { next }
            /Member-Name/    { next }
            { print $NF }'
        ;;
    llvm-readobj)
        llvm-readobj --coff-imports "$EXE" 2>/dev/null | awk -v dll="$1" '
            $1=="Name:"   { cur=($2==dll); next }
            cur && $1=="Symbol:" { print $2 }'
        ;;
    esac
}

# pick a PE import inspector: prefer binutils objdump (present on ubuntu with
# pei support), fall back to llvm-readobj, and skip the structural half when
# neither can read the import directory rather than failing spuriously.
INSPECT=""
if command -v objdump >/dev/null 2>&1 && objdump -x "$EXE" 2>/dev/null | grep -q "The Import Tables"; then
    INSPECT="objdump"
elif command -v llvm-readobj >/dev/null 2>&1; then
    INSPECT="llvm-readobj"
fi

if [ -n "$INSPECT" ]; then
    k32="$(imports_under kernel32.dll)"
    ws2="$(imports_under ws2_32.dll)"
    adv="$(imports_under advapi32.dll)"

    has() {
        if printf '%s\n' "$2" | grep -qx "$1"; then
            echo "PASS dlllibdec: '$1' imported under $3"
        else
            echo "FAIL dlllibdec: '$1' not imported under $3" >&2
            fail=1
        fi
    }
    lacks() {
        if printf '%s\n' "$2" | grep -qx "$1"; then
            echo "FAIL dlllibdec: unrenamed identifier '$1' leaked into the imports of $3" >&2
            fail=1
        fi
    }

    has Sleep             "$k32" kernel32.dll   # `symbol` rename, default attribution
    has WSAStartup        "$ws2" ws2_32.dll     # `library` pin, bare name
    has WSACleanup        "$ws2" ws2_32.dll     # `library` + `symbol` on one decl
    has SystemFunction036 "$adv" advapi32.dll   # trailing descriptor, pinned
    lacks WSAStartup        "$k32" kernel32.dll
    lacks WSACleanup        "$k32" kernel32.dll
    lacks SystemFunction036 "$k32" kernel32.dll
    lacks sleep_ms    "$k32" kernel32.dll
    lacks ws2_cleanup "$ws2" ws2_32.dll
    lacks rng_fill    "$adv" advapi32.dll
else
    echo "INFO dlllibdec: no PE import inspector (objdump/llvm-readobj); skipping the structural check"
fi

# behavioral: run under wine. the winsock and trailing advapi32 calls resolve
# only through correct decorator-driven attribution; a clean exit proves it.
if command -v wine >/dev/null 2>&1; then
    if WINEDEBUG=-all wine "$EXE" >"$WORK/wine.out" 2>&1; then
        echo "PASS dlllibdec: decorator-attributed imports resolve under wine"
    else
        echo "FAIL dlllibdec: program aborted under wine (mis-attribution?)" >&2
        cat "$WORK/wine.out" >&2
        fail=1
    fi
else
    echo "SKIP dlllibdec: 'wine' not available for the behavioral check"
fi

# negative: a `library` decorator naming a DLL absent from the dependencies must
# fail the link. derive the error app from the real source so it can't drift.
ERR="$WORK/errapp"
cp -r "$HERE/app" "$ERR"
mkdir -p "$ERR/dep"
ln -s "$STD" "$ERR/dep/mach-std"
sed 's/"ws2_32.dll"/"nope_not_declared.dll"/' "$HERE/app/src/main.mach" >"$ERR/src/main.mach"
if ( cd "$ERR" && "$MACH" build . ) >"$WORK/err.out" 2>&1; then
    echo "FAIL dlllibdec: a build pinned to an undeclared library should have failed" >&2
    fail=1
elif grep -q "not among the link's dependencies" "$WORK/err.out"; then
    echo "PASS dlllibdec: a library decorator naming an undeclared DLL fails the link"
else
    echo "FAIL dlllibdec: undeclared library decorator failed for the wrong reason" >&2
    cat "$WORK/err.out" >&2
    fail=1
fi

exit "$fail"
