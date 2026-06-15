#!/usr/bin/env bash
# section decorator integration test (#1476): prove a `section("...")` decorator
# places a global's symbol in the named object section, carried into ELF object
# emission, while an un-decorated global stays in .data.
#
# the symbol table records which section each symbol is defined in; objdump -t
# names it. the decorated `g_sec` must land in `.machsec`, the plain `g_def` in
# `.data`. the binary must also run. objdump absent -> structural check skipped.
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

fail=0

rm -rf "$WORK/app/out"
if ! ( cd "$WORK/app" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "FAIL sectiondec: build failed" >&2
    exit 1
fi

BIN="$WORK/app/out/linux/bin/sectiondec"
OBJ="$WORK/app/out/linux/obj/sectiondec/main.o"

# run: 100 + 23 == 123.
set +e
"$BIN"; code=$?
set -e
if [ "$code" -ne 123 ]; then
    echo "FAIL sectiondec: program ran with exit $code, expected 123" >&2
    fail=1
else
    echo "PASS sectiondec: the program with a sectioned global runs correctly"
fi

# structural: each global's defining section name from the object symbol table.
if command -v objdump >/dev/null 2>&1 && [ -f "$OBJ" ]; then
    # sec_of <symbol> — the section name (the field beginning with '.') on the
    # symbol-table line whose final field is the symbol name.
    sec_of() {
        objdump -t "$OBJ" 2>/dev/null | awk -v s="$1" '
            $NF==s { for (i=1;i<=NF;i++) if ($i ~ /^\./) { print $i; exit } }'
    }
    expect_sec() {
        local sym="$1" want="$2" got
        got=$(sec_of "$sym")
        if [ "$got" != "$want" ]; then
            echo "FAIL sectiondec: $sym is in section '$got', expected '$want'" >&2
            fail=1
        else
            echo "PASS sectiondec: $sym placed in $want"
        fi
    }
    expect_sec g_sec .machsec
    expect_sec g_def .data
else
    echo "INFO sectiondec: objdump unavailable or object missing; skipping the section check"
fi

exit "$fail"
