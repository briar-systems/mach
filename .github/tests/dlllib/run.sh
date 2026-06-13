#!/usr/bin/env bash
# dlllib integration test (#1382 attribution, #1388 trailing call-thunk):
# per-symbol windows DLL attribution and the import call-thunk dispatch.
#
# the PE import directory used to be built position-based — build_dynamic_info
# pinned every undefined `ext` import to DYNLIB_ANY, which the COFF writer
# routed onto the first dependency (kernel32.dll) — and register_ext_fun
# re-registered the bare `ext` identifier over any `$<sym>.symbol` rename, so
# renames silently did nothing. winsock bindings therefore landed on kernel32
# under their mach identifiers and aborted at call time on real windows.
#
# #1388 then exposed the call-thunk side: pe_iat_slot_rva never advanced the
# dependency index, so the *trailing* import descriptor's stubs jumped through
# the previous descriptor's null IAT slot (a `jmp 0` access violation on the
# first call). advapi32.dll is the trailing descriptor here and its import is
# actually called, so a clean run proves the last thunk dispatches.
#
# this builds a windows app that spreads imports across kernel32.dll, ws2_32.dll
# and advapi32.dll with `$<sym>.library`, renames decls with `$<sym>.symbol`, and
# uses both directives on one decl, then verifies three ways:
#   1. structurally — parse the emitted PE's import directory and assert each
#      import sits under the DLL it was attributed to, by its final (renamed)
#      link name. uses binutils objdump (BFD's pei support), falling back to
#      llvm-readobj, and skips this half with a notice when neither reads a PE.
#   2. behaviorally — run the binary under wine; WSAStartup/WSACleanup resolve
#      only through a correct ws2_32 attribution, and SystemFunction036 dispatches
#      only through a correct trailing-descriptor thunk, so exit 0 proves both.
#   3. negatively — a `$<sym>.library` naming a DLL absent from the link's
#      dependencies must fail the build rather than silently fall back.
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
    echo "FAIL dlllib: windows build failed" >&2
    cat "$WORK/build.out" >&2
    exit 1
fi
EXE="$APP/out/windows/bin/dlllib.exe"
test -f "$EXE" || { echo "error: windows binary not produced at $EXE" >&2; exit 1; }

# imports_under <dll> — the member names imported from <dll>, one per line, read
# from the emitted PE's import directory by whichever inspector is available.
imports_under() {
    case "$INSPECT" in
    objdump)
        # objdump -x lists each descriptor as `DLL Name: <dll>` followed by its
        # member rows; a blank line ends a block, so reset the current DLL there
        # to keep the trailing null descriptor and inter-block rows out.
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

    # assert <list> contains <symbol> under <dll-label>.
    has() {
        if printf '%s\n' "$2" | grep -qx "$1"; then
            echo "PASS dlllib: '$1' imported under $3"
        else
            echo "FAIL dlllib: '$1' not imported under $3" >&2
            fail=1
        fi
    }
    # assert <list> does NOT contain <symbol> (the mach identifier was renamed).
    lacks() {
        if printf '%s\n' "$2" | grep -qx "$1"; then
            echo "FAIL dlllib: unrenamed identifier '$1' leaked into the imports of $3" >&2
            fail=1
        fi
    }

    has Sleep            "$k32" kernel32.dll   # `.symbol` rename, default attribution
    has WSAStartup       "$ws2" ws2_32.dll     # `.library` pin, bare name
    has WSACleanup       "$ws2" ws2_32.dll     # `.library` + `.symbol` on one decl
    has SystemFunction036 "$adv" advapi32.dll  # trailing descriptor (#1388)
    # the winsock imports must be attributed away from the first dependency.
    lacks WSAStartup  "$k32" kernel32.dll
    lacks WSACleanup  "$k32" kernel32.dll
    # advapi32's import must not land on the first dependency either.
    lacks SystemFunction036 "$k32" kernel32.dll
    # the mach identifiers must not survive their `.symbol` renames.
    lacks sleep_ms    "$k32" kernel32.dll
    lacks ws2_cleanup "$ws2" ws2_32.dll
    lacks rng_fill    "$adv" advapi32.dll
else
    echo "INFO dlllib: no PE import inspector (objdump/llvm-readobj); skipping the structural check"
fi

# behavioral: run the binary under wine. the winsock calls resolve only through
# the ws2_32 attribution, and the trailing advapi32 call (SystemFunction036)
# dispatches only through a correct last-descriptor thunk (#1388) — under the bug
# it jumped through a null IAT slot and faulted with rip=0. a clean exit proves
# both the attribution and the trailing call-thunk.
if command -v wine >/dev/null 2>&1; then
    if WINEDEBUG=-all wine "$EXE" >"$WORK/wine.out" 2>&1; then
        echo "PASS dlllib: winsock and the trailing advapi32 call resolve under wine"
    else
        echo "FAIL dlllib: program aborted under wine (mis-attribution or trailing thunk?)" >&2
        cat "$WORK/wine.out" >&2
        fail=1
    fi
else
    echo "SKIP dlllib: 'wine' not available for the behavioral check"
fi

# negative: a `.library` naming a DLL absent from the dependencies must fail the
# link. derive the error app from the real source so the fixture can't drift —
# repoint the winsock imports at an undeclared DLL.
ERR="$WORK/errapp"
cp -r "$HERE/app" "$ERR"
mkdir -p "$ERR/dep"
ln -s "$STD" "$ERR/dep/mach-std"
sed 's/"ws2_32.dll"/"nope_not_declared.dll"/' "$HERE/app/src/main.mach" >"$ERR/src/main.mach"
if ( cd "$ERR" && "$MACH" build . ) >"$WORK/err.out" 2>&1; then
    echo "FAIL dlllib: a build pinned to an undeclared library should have failed" >&2
    fail=1
elif grep -q "not among the link's dependencies" "$WORK/err.out"; then
    echo "PASS dlllib: a .library naming an undeclared DLL fails the link"
else
    echo "FAIL dlllib: undeclared .library failed for the wrong reason" >&2
    cat "$WORK/err.out" >&2
    fail=1
fi

exit "$fail"
