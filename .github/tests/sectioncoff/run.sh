#!/usr/bin/env bash
# section decorator COFF integration test (#1491, fast-follow #1476): the
# windows/COFF twin of sectiondec — prove a `#[section("...")]` decorator places a
# global's symbol and a function's code in the named COFF section, carried into
# PE/COFF object emission, while an un-decorated global stays in .data.
#
# the symbol table records which section each symbol is defined in; objdump -t
# reports a 1-based section index that maps to a name via objdump -h (the >8char
# name is resolved from the COFF string table). the decorated `g_sec` must land
# in `.machsec`, `f_sec` in `.hottext_long`, the plain `g_def` in `.data`.
# `.hottext_long` is >8 characters, so it exercises the COFF string-table `/N`
# redirection that the exactly-8 `.machsec` / `.hottext` names never reach. the
# binary must also run under wine. no PE inspector -> structural check skipped.
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
    echo "error: vendored std not found at $STD (run: mach dep pull)" >&2
    exit 2
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"

fail=0

rm -rf "$WORK/app/out"
if ! ( cd "$WORK/app" && "$MACH" build . ) >"$WORK/build.out" 2>&1; then
    echo "FAIL sectioncoff: windows build failed" >&2
    cat "$WORK/build.out" >&2
    exit 1
fi

EXE="$WORK/app/out/windows/bin/sectioncoff.exe"
OBJ="$WORK/app/out/windows/obj/sectioncoff/main.o"
test -f "$EXE" || { echo "error: windows binary not produced at $EXE" >&2; exit 1; }

# pick a PE inspector: prefer binutils objdump (PE-capable on ubuntu), fall back
# to llvm-readobj, and skip the structural half when neither can read the object
# rather than failing spuriously — mirroring dlllibdec.
INSPECT=""
if command -v objdump >/dev/null 2>&1 && objdump -h "$OBJ" >/dev/null 2>&1; then
    INSPECT="objdump"
elif command -v llvm-readobj >/dev/null 2>&1; then
    INSPECT="llvm-readobj"
fi

# sec_of <symbol> — the name of the COFF section the symbol is defined in.
sec_of() {
    local sym="$1" num
    case "$INSPECT" in
    objdump)
        # objdump -t reports the defining section as a 1-based "(sec N)"; map N to
        # the section name via objdump -h's 0-based Idx column.
        num=$(objdump -t "$OBJ" 2>/dev/null | awk -v s="$sym" '
            $NF==s { if (match($0, /\(sec +[0-9]+\)/)) { x=substr($0,RSTART,RLENGTH); gsub(/[^0-9]/,"",x); print x; exit } }')
        [ -n "$num" ] || { echo "?"; return; }
        objdump -h "$OBJ" 2>/dev/null | awk -v n="$num" '$1 ~ /^[0-9]+$/ && $1+0==n-1 {print $2; exit}'
        ;;
    llvm-readobj)
        # --symbols gives the symbol's 1-based section number; --sections maps it
        # to a name (the >8char name resolved from the string table).
        num=$(llvm-readobj --symbols "$OBJ" 2>/dev/null | awk -v s="$sym" '
            $1=="Name:" && $2==s { f=1; next }
            f && $1=="Section:" { n=$NF; gsub(/[()]/,"",n); print n; exit }')
        [ -n "$num" ] || { echo "?"; return; }
        llvm-readobj --sections "$OBJ" 2>/dev/null | awk -v n="$num" '
            $1=="Number:" { cur=($2==n) }
            cur && $1=="Name:" { print $2; exit }'
        ;;
    esac
}

if [ -n "$INSPECT" ]; then
    expect_sec() {
        local sym="$1" want="$2" got
        got=$(sec_of "$sym")
        if [ "$got" != "$want" ]; then
            echo "FAIL sectioncoff: $sym is in section '$got', expected '$want'" >&2
            fail=1
        else
            echo "PASS sectioncoff: $sym placed in $want"
        fi
    }
    expect_sec g_sec .machsec
    expect_sec g_def .data
    # the >8char text section exercises the COFF string-table /N redirection.
    expect_sec f_sec .hottext_long
    expect_sec main  .text
else
    echo "INFO sectioncoff: no PE inspector (objdump/llvm-readobj); skipping the section check"
fi

# behavioral: run under wine. f_sec lives in a section reached across a normal
# relocation, and the sectioned global is read back — a clean 100 + 23 == 123
# proves the placement did not break code or data references.
if command -v wine >/dev/null 2>&1; then
    set +e
    WINEDEBUG=-all wine "$EXE" >"$WORK/wine.out" 2>&1
    code=$?
    set -e
    if [ "$code" -eq 123 ]; then
        echo "PASS sectioncoff: the program with sectioned globals/code runs under wine"
    else
        echo "FAIL sectioncoff: program ran under wine with exit $code, expected 123" >&2
        cat "$WORK/wine.out" >&2
        fail=1
    fi
else
    echo "SKIP sectioncoff: 'wine' not available for the behavioral check"
fi

exit "$fail"
