#!/usr/bin/env bash
# align decorator integration test (#1476): prove an `align(...)` decorator sets
# the emitted alignment of a global, including the acceptance-criteria case of a
# comptime-expr argument (`align($size_of(Pair))` folding to 16).
#
# each decorated global lands in its own data section whose `sh_addralign` is the
# requested alignment; the object file carries the per-symbol section header that
# readelf reads (the final static executable strips section headers). the build
# asserts: g_lit64 -> 64 (literal), g_cmp16 -> 16 (comptime expr), g_plain -> 8
# (default), g_over -> 32 (type-raised). the binary itself ALSO probes each over-
# aligned global's address at runtime — an alignment-dependent check that holds
# even when readelf is absent — exiting 27 only when every probe passes.
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
if ! ( cd "$WORK/app" && "$MACH" build . -O2 ) >/dev/null 2>&1; then
    echo "FAIL aligndec: build failed" >&2
    exit 1
fi

BIN="$WORK/app/out/linux/bin/aligndec"
OBJ="$WORK/app/out/linux/obj/aligndec/main.o"

# run: the alignment probes pass (exit 27 = 7 + 9 + 11); a misaligned global
# would exit 91/92/93 instead, naming which probe failed.
set +e
"$BIN"; code=$?
set -e
if [ "$code" -ne 27 ]; then
    echo "FAIL aligndec: program ran with exit $code, expected 27 (91/92/93 = a runtime misalignment)" >&2
    fail=1
else
    echo "PASS aligndec: the over-aligned globals are correctly aligned at runtime"
fi

# structural check: each global's section header alignment in the object file.
if command -v readelf >/dev/null 2>&1 && [ -f "$OBJ" ]; then
    # sec_align <symbol> — the sh_addralign of the section the symbol is defined in.
    sec_align() {
        local sym="$1" ndx
        ndx=$(readelf -sW "$OBJ" 2>/dev/null | awk -v n="$sym" '$8==n {print $7; exit}')
        [ -n "$ndx" ] || { echo "?"; return; }
        readelf -SW "$OBJ" 2>/dev/null | grep -E "^[[:space:]]*\[[[:space:]]*${ndx}\]" | awk '{print $NF}'
    }
    expect_align() {
        local sym="$1" want="$2" got
        got=$(sec_align "$sym")
        if [ "$got" != "$want" ]; then
            echo "FAIL aligndec: $sym section align is $got, expected $want" >&2
            fail=1
        else
            echo "PASS aligndec: $sym aligned to $want"
        fi
    }
    expect_align g_lit64 64
    expect_align g_cmp16 16
    expect_align g_plain 8
    # a global of an `align(32)` record inherits the type's raised alignment.
    expect_align g_over  32
else
    echo "INFO aligndec: readelf unavailable or object missing; skipping the alignment check"
fi

exit "$fail"
