#!/usr/bin/env bash
# self-additive.sh <compiler> — the whole-compiler capstone of the -g additivity guard.
#
# dbg/verify.sh step 5 (and its release counterpart verify_inline_release) prove `-g`
# adds only debug sections on the small fixtures. This proves it on the compiler ITSELF:
# build this project at `--profile release -g` and `--profile release` with <compiler>,
# and require every PT_LOAD segment to be byte-identical between the two. Because the
# compiler exercises far more of the language than any fixture, this catches any pass
# whose decision reads debug metadata (inliner size gate #1932, mem2reg escape #1944,
# DCE salvage #1958, param homing #1956 were four such) — the whole class at once.
#
# WHY IT IS NOT A PER-PR LANE. it runs two full compiler self-builds, too heavy for the
# per-PR debuginfo job. The per-PR guards (verify.sh's fixture legs across all three ELF
# ISAs, plus the pass-level unit regressions) already cover known regressions; this is the
# release-cut / nightly backstop, wired from .github/workflows/cd.yml.
set -euo pipefail

compiler=${1:?usage: self-additive.sh <compiler>}
case "$compiler" in
    /*) : ;;
    *)  compiler=$(CDPATH= cd -- "$(dirname -- "$compiler")" && pwd)/$(basename -- "$compiler") ;;
esac

here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
root=$(CDPATH= cd -- "$here/.." && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

g="$tmp/mach.g"
nog="$tmp/mach.nog"

( cd "$root" && "$compiler" dep pull && "$compiler" build . --profile release -g -o "$g" )
( cd "$root" && "$compiler" build . --profile release -o "$nog" )

# _norm_shdr_fields <in> <out> — copy <in> to <out> zeroing the ELF header's
# section-table bookkeeping (e_shoff@40 8B, e_shnum@60 2B, e_shstrndx@62 2B), which
# legitimately differs once -g adds named debug sections.
_norm() {
    cp "$1" "$2"
    printf '\0\0\0\0\0\0\0\0' | dd of="$2" bs=1 seek=40 count=8 conv=notrunc status=none
    printf '\0\0'             | dd of="$2" bs=1 seek=60 count=2 conv=notrunc status=none
    printf '\0\0'             | dd of="$2" bs=1 seek=62 count=2 conv=notrunc status=none
}

an="$tmp/a.norm"; bn="$tmp/b.norm"
_norm "$g" "$an"; _norm "$nog" "$bn"

rc=0
n=0
while read -r off fsz; do
    [ "$fsz" -eq 0 ] && continue
    n=$((n + 1))
    if ! cmp -s \
        <(dd if="$an" bs=1M iflag=skip_bytes,count_bytes skip="$off" count="$fsz" status=none) \
        <(dd if="$bn" bs=1M iflag=skip_bytes,count_bytes skip="$off" count="$fsz" status=none); then
        echo "FAIL: -g perturbed a loadable segment of the compiler (PT_LOAD off=$off sz=$fsz)" >&2
        rc=1
    fi
done < <(readelf -lW "$g" 2>/dev/null | awk '/LOAD/{print strtonum($2), strtonum($5)}')

if [ "$n" -eq 0 ]; then
    echo "self-additive.sh: no PT_LOAD segments read from $g" >&2
    exit 1
fi
if [ "$rc" -ne 0 ]; then
    exit 1
fi
echo "self-additive: the compiler's own release build is -g additive ($n PT_LOAD segments identical)"
