#!/usr/bin/env bash
# the codegen corpus, the link cases and the dwarf verify: one loop over
# test/cases/<group>/<case>.mach and the targets, grounded in external tools.
# --incremental instead proves the warm build path against clean builds, and
# --docs compiles the mach code blocks of doc/language.
# see test/README.md for the case contract and how to add a case.
#
# usage: test/run.sh [--target <t>]... [--case <group>/<name>]... [--bless]
#                    [--qemu] [--link] [--dwarf] [--incremental] [--docs]
#
# per case and target: build the object in release, disassemble it with the
# external decoder and diff against test/golden/<target>/<group>/<case>.dis; on a
# target this host can execute, build at O0 and O2, run both, and compare the
# checksums to the C reference built from test/ref/<group>/<case>.c. spirv is
# built, validated with spirv-val, and diffed through spirv-dis. a case named in
# test/golden/<target>/SKIPS is not built for that target; one named in its NORUN
# keeps its golden and skips the differential.
#
#   --target <t>   one target (repeatable); default every target with a golden dir
#   --case <g/n>   one case (repeatable)
#   --bless        write the goldens instead of diffing them, print the diff
#   --qemu         execute aarch64-linux, riscv64-linux, riscv64zkt-linux and riscv32 under
#                  qemu-user when this host cannot run them natively
#                  (a missing emulator is announced and its target stays golden only)
#   --link         run the link cases (test/link/cases) instead of the corpus
#   --dwarf        build every case with -g and verify its debug model (llvm-dwarfdump --verify, spirv-val)
#   --incremental  warm rebuilds of this compiler and of a manifest fixture match clean builds
#   --docs         compile every mach block in doc/language and run each one with a main
#                  (--case <page> selects one page, such as --case operators; one hosted
#                  --target compiles for it instead of the host, running only natively)
#   MACH           the compiler under test, default out/<host>/debug/bin/mach
#   DOCS           the pages --docs reads, default doc/language
set -u

here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo=$(CDPATH= cd -- "$here/.." && pwd)
out=$here/out
mkdir -p "$out"

# the external decoders and their exact flag sets. a golden is only reproducible
# against a named tool and a named flag set, so both are stated here and nowhere
# else. --symbolize-operands replaces every branch target with a local label and
# --no-leading-addr drops the address column, so a layout shift that changed no
# code changes no golden.
objdump_flags="-d --no-leading-addr --no-show-raw-insn --symbolize-operands"
spirv_dis_flags="--no-color --no-indent"
objdump_major=22
spirv_tools_version=2026.3
# -ffp-contract=off keeps a fused multiply-add out of the reference so the anchor
# disagrees with mach about semantics only, never about rounding
cflags_O0="-std=c11 -O0 -ffp-contract=off -Wall -Wextra"
cflags_O2="-std=c11 -O2 -ffp-contract=off -Wall -Wextra"
cflags_ubsan="-std=c11 -O0 -ffp-contract=off -fsanitize=undefined -fno-sanitize-recover=all"

# the targets: name isa os abi of kind entry decoder qemu. qemu names the
# qemu-user command that runs the target under --qemu, or - for none. a direct
# target with one also builds a run bin from test/lib/start_<name>.mach, re-laid
# by test/lib/elf_loadable.py, since qemu-user cannot map a freestanding image.
targets_all='
x86_64-linux      x86_64      linux         sysv64   -    bin     hosted  objdump    -
aarch64-linux     aarch64     linux         aapcs64  -    bin     hosted  objdump    qemu-aarch64
riscv64-linux     riscv64     linux         lp64d    -    bin     hosted  objdump    qemu-riscv64
riscv64zkt-linux  rv64gc_zkt  linux         lp64d    -    bin     hosted  objdump    qemu-riscv64
x86_64-windows    x86_64      windows       win64    -    bin     hosted  objdump    -
x86_64-darwin     x86_64      darwin        sysv64   -    bin     hosted  objdump    -
aarch64-darwin    aarch64     darwin        aapcs64  -    bin     hosted  objdump    -
spirv             spirv       freestanding  spirv    -    bin     direct  spirv-dis  -
riscv32           rv32imafdc  freestanding  ilp32d   elf  static  direct  objdump    qemu-riscv32
'
# where a run bin is based: above the host's mmap floor with a page for its headers
run_base=0x20000

usage() { sed -n '2,32p' "$0" | sed 's/^# \{0,1\}//' >&2; exit 2; }

want_targets=
want_cases=
bless=0
qemu=0
mode=corpus
dwarf=0
while [ $# -gt 0 ]; do
    case "$1" in
        --target) shift; [ $# -gt 0 ] || usage; want_targets="$want_targets $1" ;;
        --case)   shift; [ $# -gt 0 ] || usage; want_cases="$want_cases $1" ;;
        --bless)  bless=1 ;;
        --qemu)   qemu=1 ;;
        --link)   mode=link ;;
        --incremental) mode=incremental ;;
        --docs)   mode=docs ;;
        --dwarf)  dwarf=1 ;;
        -h|--help) usage ;;
        *) echo "run.sh: unknown option '$1'" >&2; usage ;;
    esac
    shift
done

case "$(uname -s)" in
    Linux)  host_os=linux ;;
    Darwin) host_os=darwin ;;
    MINGW*|MSYS*|CYGWIN*) host_os=windows ;;
    *) host_os=$(uname -s) ;;
esac
case "$(uname -m)" in
    x86_64|amd64)  host_isa=x86_64 ;;
    aarch64|arm64) host_isa=aarch64 ;;
    *) host_isa=$(uname -m) ;;
esac
exe=
[ "$host_os" = windows ] && exe=.exe
case "$host_os/$host_isa" in
    linux/x86_64)   host_dir=linux-x86_64 ;;
    linux/aarch64)  host_dir=linux-arm64 ;;
    darwin/x86_64)  host_dir=darwin-x86_64 ;;
    darwin/aarch64) host_dir=darwin-aarch64 ;;
    *)              host_dir=windows-x86_64 ;;
esac

mach=${MACH:-$repo/out/$host_dir/debug/bin/mach$exe}
case "$mach" in /*) : ;; *) mach=$PWD/$mach ;; esac
if [ ! -x "$mach" ]; then
    echo "run.sh: no compiler at $mach; build one or set MACH" >&2
    exit 2
fi
echo "compiler: $mach ($("$mach" info 2>/dev/null | sed -n 1p))"

if [ -n "$want_targets" ]; then
    for t in $want_targets; do
        printf '%s\n' "$targets_all" | awk -v t="$t" '$1 == t { f = 1 } END { exit !f }' ||
            { echo "run.sh: no such target '$t'" >&2; exit 2; }
    done
    targets=$want_targets
else
    targets=$(for d in "$here"/golden/*/; do basename "$d"; done | tr '\n' ' ')
fi

fails=0
passes=0
skips=0
noruns=0
fail() { echo "FAIL $*"; fails=$((fails + 1)); }
# unrun <target>: a failed case on a target with a differential never reached it,
# so the summary says how many behaviour checks did not execute
unruns=0
unrun() { [ "$(engine "$1")" = - ] || unruns=$((unruns + 1)); }

# target_field <target> <column>
target_field() { printf '%s\n' "$targets_all" | awk -v t="$1" -v c="$2" '$1 == t { print $c }'; }

# object_format <target>
object_format() {
    isa=$(target_field "$1" 2); os=$(target_field "$1" 3); of=$(target_field "$1" 5)
    [ "$isa" = spirv ] && { echo spv; return; }
    [ "$of" != - ] && { echo "$of"; return; }
    case "$os" in linux) echo elf ;; windows) echo coff ;; darwin) echo macho ;; *) echo raw ;; esac
}

# engine <target>: "" for the host itself, the qemu command, or "-" when nothing here
# runs it. qemu serves only a target this host cannot run: it is compute evidence,
# never ABI evidence, so aarch64-linux under qemu-aarch64 lets a corpus run on an
# x86_64 host execute its cases while CI still proves them on real silicon.
# riscv64zkt-linux selects Zkt, whose only effect on codegen is admitting the
# secret multiply, so qemu is compute evidence for that column too.
engine() {
    isa=$(target_field "$1" 2); os=$(target_field "$1" 3); q=$(target_field "$1" 9)
    if [ "$os" = "$host_os" ] && [ "$isa" = "$host_isa" ]; then echo ""; return; fi
    if [ "$qemu" -eq 1 ] && [ "$q" != - ] && command -v "$q" >/dev/null 2>&1; then
        echo "$q"; return
    fi
    echo -
}

# runs_bare <target>: a direct target the differential executes
runs_bare() { [ "$(target_field "$1" 7)" = direct ] && [ "$(target_field "$1" 9)" != - ]; }

docs=${DOCS:-$repo/doc/language}
case "$docs" in /*) : ;; *) docs=$PWD/$docs ;; esac

# the cases, as group/name; under --docs a case is a page of $docs
if [ "$mode" = docs ]; then
    cases=
    for c in $want_cases; do
        [ -f "$docs/$c.md" ] || { echo "run.sh: no such page '$docs/$c.md'" >&2; exit 2; }
    done
elif [ -n "$want_cases" ]; then
    cases=
    for c in $want_cases; do
        case "$c" in
            */*) [ -f "$here/cases/$c.mach" ] || { echo "run.sh: no such case '$c'" >&2; exit 2; }
                 cases="$cases $c" ;;
            *)   [ -d "$here/link/cases/$c" ] || { echo "run.sh: no such link case '$c'" >&2; exit 2; } ;;
        esac
    done
else
    cases=$(cd "$here/cases" && for f in */*.mach; do echo "${f%.mach}"; done | LC_ALL=C sort | tr '\n' ' ')
fi

# listed <file> <target> <case>: the first field of a test/golden/<target>/<file>
# line is a case name or a glob; the rest of the line is the reason
listed() {
    [ -f "$here/golden/$2/$1" ] || return 1
    while read -r pat rest; do
        case "$pat" in ''|\#*) continue ;; esac
        case "$3" in $pat) return 0 ;; esac
    done <"$here/golden/$2/$1"
    return 1
}
# served: the column's scope. a target with an ONLY file serves the cases its
# lines match and no other; without one it serves every case. skipped: the target
# cannot build the case. norun: it builds and its golden is diffed, but its
# differential disagrees with the reference, so the disagreement is not a failure.
# a norun case counts as a pass with its column reported golden only. SKIPS and
# NORUN are claims about a case and are checked, not trusted: a skipped case that
# builds and a norun case whose differential agrees are stale lines, and the run
# fails on them. ONLY is a decision about the column, not a claim, so it is not.
served()  { [ ! -f "$here/golden/$1/ONLY" ] || listed ONLY "$1" "$2"; }
skipped() { listed SKIPS "$1" "$2"; }
norun()   { listed NORUN "$1" "$2"; }

# one generated project per entry shape: hosted cases print their checksum through
# std, direct cases (spirv, riscv32) are the artifact and reach no runtime at all.
# a direct target qemu runs also gets a run bin per case, whose entry is the
# target's start_<name>.mach and reports the checksum by raw syscalls.
materialise() {
    hosted=$out/hosted
    bare=$out/bare
    rm -rf "$hosted" "$bare"
    for proj in "$hosted" "$bare"; do
        mkdir -p "$proj/src/cases" "$proj/src/lib"
        cp "$here"/lib/fold.mach "$here"/lib/fold128.mach "$proj/src/lib/"
        for c in $cases; do
            mkdir -p "$proj/src/cases/${c%/*}"
            cp "$here/cases/$c.mach" "$proj/src/cases/$c.mach"
        done
    done
    mkdir -p "$hosted/src/entry" "$hosted/dep/std"
    cp -r "$repo/dep/std/src" "$hosted/dep/std/src"
    cp "$repo/dep/std/mach.toml" "$hosted/dep/std/"
    for c in $cases; do
        cat >"$hosted/src/entry/$(art "$c").mach" <<EOF
use std.runtime;
use std.types.size.usize;
use p: std.print;
use k: corpus.cases.${c%/*}.${c#*/};

#[symbol("main")]
fun main(argc: usize, argv: **u8) i64 {
    p.printlnf("{:016x}", k.checksum(argc::u64 - 1));
    ret 0;
}
EOF
    done
    for t in $(printf '%s\n' "$targets_all" | awk '$7 == "direct" && $9 != "-" { print $1 }'); do
        mkdir -p "$bare/src/run/$t"
        cp "$here/lib/start_$t.mach" "$bare/src/lib/"
        for c in $cases; do
            cat >"$bare/src/run/$t/$(art "$c").mach" <<EOF
use start: corpus.lib.start_$t;
use k: corpus.cases.${c%/*}.${c#*/};

#[symbol("corpus_run")]
fun run(argc: u32) {
    start.report(k.checksum((argc - 1)::u64));
}
EOF
        done
    done
    manifest "$hosted" hosted >"$hosted/mach.toml"
    manifest "$bare" direct >"$bare/mach.toml"
}

art() { echo "$1" | tr / _; }

# manifest <project> <entry-shape>
manifest() {
    shape=$2
    echo '[project]'; echo 'id = "corpus"'; echo 'version = "0.0.0"'; echo 'src = "src"'
    echo 'out = "o/{target.name}/{profile.name}"'; echo
    names=
    printf '%s\n' "$targets_all" | while read -r name isa os abi of kind entry decoder q; do
        [ -n "$name" ] && [ "$entry" = "$shape" ] || continue
        echo "[target.$name]"; echo "isa = \"$isa\""; echo "os  = \"$os\""; echo "abi = \"$abi\""
        [ "$of" != - ] && echo "of  = \"$of\""
        [ "$shape" = direct ] && runs_bare "$name" && echo "base = $run_base"
        echo
    done
    for p in o0 o2 g g2; do
        echo "[profile.$p]"
        case $p in o0) echo 'opt = 0'; echo 'debug = false'; echo 'default = true' ;;
                   o2) echo 'opt = 2'; echo 'debug = false' ;;
                   g)  echo 'opt = 0'; echo 'debug = true' ;;
                   g2) echo 'opt = 2'; echo 'debug = true' ;; esac
        echo 'simd = "scalarize"'; echo 'vectorize = true'; echo 'float_reassoc = false'; echo
    done
    for c in $cases; do
        a=$(art "$c")
        if [ "$shape" = hosted ]; then
            echo "[artifact.$a]"; echo 'kind = "bin"'; echo "entry = \"entry/$a.mach\""
            echo "out = \"bin/$a\""
            echo "targets = [$(printf '%s\n' "$targets_all" | awk '$7 == "hosted" { printf "%s\"%s\"", (n++ ? ", " : ""), $1 }')]"
            echo 'link = []'; echo 'need = []'; echo
        else
            printf '%s\n' "$targets_all" | while read -r name isa os abi of kind entry decoder q; do
                [ -n "$name" ] && [ "$entry" = direct ] || continue
                echo "[artifact.${a}_$kind]"; echo "kind = \"$kind\""; echo "entry = \"cases/$c.mach\""
                if [ "$kind" = static ]; then echo "out = \"lib/$a.a\""; else echo "out = \"bin/$a\""; fi
                echo "targets = [\"$name\"]"; echo 'link = []'; echo 'need = []'; echo
                [ "$q" != - ] || continue
                echo "[artifact.${a}_run_$name]"; echo 'kind = "bin"'; echo "entry = \"run/$name/$a.mach\""
                echo "out = \"run/$a\""
                echo "targets = [\"$name\"]"; echo 'link = []'; echo 'need = []'; echo
            done
        fi
    done
    [ "$shape" = hosted ] && { echo '[dep.std]'; echo 'path = "dep/std"'; echo; }
    return 0
}

# build <target> <profile> <case> [run]: the artifact for one case, log at log_of.
# run asks for what the differential executes, which on a direct target is the
# run bin re-laid for qemu-user rather than the case's own artifact.
build() {
    t=$1; p=$2; c=$3; role=${4:-}
    a=$(art "$c")
    run_bin=
    if [ "$(target_field "$t" 7)" = hosted ]; then
        proj=$out/hosted; sel="--bin $a"
    elif [ "$role" = run ]; then
        proj=$out/bare; sel="--bin ${a}_run_$t"; run_bin=$out/bare/o/$t/$p/run/$a
    else
        proj=$out/bare
        kind=$(target_field "$t" 6)
        if [ "$kind" = static ]; then sel="--lib ${a}_$kind"; else sel="--bin ${a}_$kind"; fi
    fi
    log=$(log_of "$t" "$p" "$c" "$role")
    mkdir -p "$out/log"
    "$mach" build "$proj" --target "$t" --profile "$p" $sel >"$log" 2>&1 || return 1
    [ -z "$run_bin" ] || python3 "$here/lib/elf_loadable.py" "$run_bin" "$run_bin.load" >>"$log" 2>&1
}

# differential <target> <case> <engine>: mach at O0 and O2 against the C reference,
# 0 when both agree, otherwise 1 with the disagreement in why
differential() {
    t=$1; c=$2; eng=$3
    ref=$(reference "$c" 2>"$out/log/ref.$(art "$c").err") || {
        why="reference: $(cat "$out/log/ref.$(art "$c").err")"; return 1
    }
    # a hosted o2 build above is already the run bin, a direct one is not
    for p in o0 o2; do
        if { [ "$p" = o0 ] || runs_bare "$t"; } && ! build "$t" "$p" "$c" run; then
            why="build $p run: $(first_error "$(log_of "$t" "$p" "$c" run)")"; return 1
        fi
        bin=$(artifact "$t" "$p" "$c" run)
        # the status is the program's own, read before any substitution (PIPESTATUS
        # after an assignment is the assignment's); a wrong program may print a
        # NUL, which a substitution would warn about, so the output is filed first
        timeout 60 $eng "$bin" >"$out/log/$t.$p.$(art "$c").out" 2>"$out/log/$t.$p.$(art "$c").err"; rc=$?
        got=$(tr -d '\0' <"$out/log/$t.$p.$(art "$c").out")
        # a program admitting a secret multiply sets PSTATE.DIT at start and
        # refuses, with std's one-line refusal and status 255, on an aarch64
        # host without FEAT_DIT: the host cannot run it, and the cell is not a
        # verdict on the compiler
        if [ "$rc" -eq 255 ] && grep -q "data-independent-timing mode (PSTATE.DIT)" "$out/log/$t.$p.$(art "$c").err"; then
            why="$p: this host provides no PSTATE.DIT, so the program refused to start"; return 2
        fi
        if [ "$rc" -ne 0 ]; then why="$p: exit $rc"; return 1; fi
        if [ -s "$out/log/$t.$p.$(art "$c").err" ]; then why="$p: wrote to stderr"; return 1; fi
        if [ "$got" != "$ref" ]; then why="$p: mach says $got, C reference says $ref"; return 1; fi
    done
    return 0
}

# log_of <target> <profile> <case> [run]
log_of() { echo "$out/log/$1.$2.$(art "$3")${4:+.$4}.log"; }

# object <target> <profile> <case>
object() {
    fmt=$(object_format "$1")
    base=$out/$( [ "$(target_field "$1" 7)" = hosted ] && echo hosted || echo bare )/o/$1/$2/obj/corpus/cases/$3
    case "$fmt" in spv) echo "$base.spv" ;; *) echo "$base.o" ;; esac
}

# artifact <target> <profile> <case> [run]: the executable, archive or module
artifact() {
    a=$(art "$3")
    if [ "${4:-}" = run ] && [ "$(target_field "$1" 7)" = direct ]; then
        echo "$out/bare/o/$1/$2/run/$a.load"
    elif [ "$(target_field "$1" 7)" = hosted ]; then
        d=$out/hosted/o/$1/$2/bin
        if [ -f "$d/$a.exe" ]; then echo "$d/$a.exe"; else echo "$d/$a"; fi
    elif [ "$(target_field "$1" 6)" = static ]; then
        echo "$out/bare/o/$1/$2/lib/$a.a"
    else
        object "$1" "$2" "$3"
    fi
}

# first_error <log>
first_error() { grep -m1 -E '^error:|: error' "$1" || tail -n1 "$1"; }

# disassemble <target> <case> <object>: the decoder text, path-free, LF, trimmed
disassemble() {
    t=$1; c=$2; o=$3
    fmt=$(object_format "$t")
    if [ "$fmt" = spv ]; then
        spirv-dis $spirv_dis_flags "$o"
    else
        llvm-objdump $objdump_flags "$o"
    fi | awk -v head="$c.${o##*.}:" -v path="$o" '
        { sub(/\r$/, "") }
        index($0, path) { $0 = head }
        { l[++n] = $0 }
        END {
            s = 1; while (s <= n && l[s] ~ /^[[:space:]]*$/) s++
            e = n; while (e >= s && l[e] ~ /^[[:space:]]*$/) e--
            for (i = s; i <= e; i++) print l[i]
        }'
}

# reference <case>: the C answer, built and run once per case at O0, O2 and ubsan;
# the three must agree before either is compared with mach. the answer is kept
# until the reference or the shared header is edited, so a run never compares
# mach against a stale reference
reference() {
    c=$1
    ans=$out/ref/$c.ans
    src=$here/ref/$c.c
    [ -f "$src" ] || { echo "no C reference at $src" >&2; return 1; }
    if [ -f "$ans" ] && [ ! "$src" -nt "$ans" ] && [ ! "$here/lib/corpus.h" -nt "$ans" ]; then
        cat "$ans"; return 0
    fi
    mkdir -p "$out/ref/${c%/*}"
    modes="O0 O2 ubsan"
    [ "$host_os" = windows ] && modes="O0 O2"
    got=
    for m in $modes; do
        eval "flags=\$cflags_$m"
        if ! ${CC:-cc} $flags -I "$here/lib" -o "$out/ref/$c.$m" "$src" >"$out/ref/$c.$m.log" 2>&1; then
            echo "cc -$m failed to build the reference: $(first_error "$out/ref/$c.$m.log")" >&2; return 1
        fi
        v=$("$out/ref/$c.$m") || { echo "reference -$m exited $?" >&2; return 1; }
        case "$v" in [0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f]) ;;
            *) echo "reference -$m printed '$v', not one 16-hex-digit line" >&2; return 1 ;; esac
        if [ -n "$got" ] && [ "$v" != "$got" ]; then
            echo "harness defect: reference answers disagree ($got vs $v at -$m)" >&2; return 1
        fi
        got=$v
    done
    echo "$got" >"$ans"
    echo "$got"
}

# run_case <target> <case>
run_case() {
    t=$1; c=$2
    fmt=$(object_format "$t")
    if ! served "$t" "$c"; then skips=$((skips + 1)); return; fi
    if skipped "$t" "$c"; then
        if build "$t" o2 "$c"; then fail "$t $c builds: its golden/$t/SKIPS line is stale"; else skips=$((skips + 1)); fi
        return
    fi

    # release build, decoded and diffed against the golden
    if ! build "$t" o2 "$c"; then
        fail "$t $c build o2: $(first_error "$out/log/$t.o2.$(art "$c").log")"; unrun "$t"; return
    fi
    o=$(object "$t" o2 "$c")
    [ -f "$o" ] || { fail "$t $c o2: no object at $o"; unrun "$t"; return; }
    if [ "$fmt" = spv ]; then
        if ! spirv-val "$o" >"$out/log/$t.val.$(art "$c").log" 2>&1; then
            fail "$t $c spirv-val: $(head -n1 "$out/log/$t.val.$(art "$c").log")"; unrun "$t"; return
        fi
    fi
    golden=$here/golden/$t/$c.dis
    dis=$out/log/$t.$(art "$c").dis
    disassemble "$t" "$c" "$o" >"$dis" || { fail "$t $c disassemble"; unrun "$t"; return; }
    # a golden verdict is held, not returned on: the differential below is the
    # stronger fact and runs whatever the golden says, so one run reports both
    golden_why=
    if [ "$bless" -eq 1 ]; then
        mkdir -p "$(dirname "$golden")"
        if [ ! -f "$golden" ] || ! cmp -s "$golden" "$dis"; then
            [ -f "$golden" ] && diff -u "$golden" "$dis" | sed 's/^/    /'
            cp "$dis" "$golden"
            echo "BLESS $t $c"
        fi
    elif [ ! -f "$golden" ]; then
        golden_why="golden: none at ${golden#"$here"/}; run --bless"
    elif ! cmp -s "$golden" "$dis"; then
        golden_why="golden: $(diff "$golden" "$dis" | head -n1 | sed 's/^/line /')"
    fi

    # the differential: mach at O0 and O2 against the C reference. a norun case
    # runs it too, and the disagreement is what its line claims
    eng=$(engine "$t")
    if [ "$eng" != - ]; then
        differential "$t" "$c" "$eng"; verdict=$?
        if [ "$verdict" -eq 0 ]; then
            if norun "$t" "$c"; then fail "$t $c agrees with the C reference: its golden/$t/NORUN line is stale"; return; fi
            [ -z "$golden_why" ] || { fail "$t $c $golden_why (differential agrees with the C reference)"; return; }
        elif [ "$verdict" -eq 2 ]; then
            [ -z "$golden_why" ] || { fail "$t $c $golden_why (differential not run: $why)"; return; }
            echo "NORUN $t $c: $why"
            skips=$((skips + 1)); return
        elif norun "$t" "$c"; then
            [ -z "$golden_why" ] || { fail "$t $c $golden_why (differential disagrees as its NORUN line claims)"; return; }
            noruns=$((noruns + 1))
        else
            case $why in build\ *) unrun "$t" ;; esac
            fail "$t $c ${golden_why:+$golden_why; }$why"; return
        fi
    elif [ -n "$golden_why" ]; then
        fail "$t $c $golden_why"; return
    fi

    # the -g build through the external verifier for its debug model
    if [ "$dwarf" -eq 1 ] && { [ "$fmt" = elf ] || [ "$fmt" = macho ] || [ "$fmt" = coff ]; }; then
        if ! build "$t" g "$c"; then
            fail "$t $c build g: $(first_error "$out/log/$t.g.$(art "$c").log")"; return
        fi
        bin=$(artifact "$t" g "$c")
        if ! llvm-dwarfdump --verify "$bin" >"$out/log/$t.g.$(art "$c").verify" 2>&1; then
            fail "$t $c dwarfdump --verify: $(grep -m1 -E 'error|warning' "$out/log/$t.g.$(art "$c").verify")"; return
        fi
    fi
    # spirv at O2, the level its golden is built at: some cases are refused at O0 for
    # reasons that have nothing to do with debug info
    if [ "$dwarf" -eq 1 ] && [ "$fmt" = spv ]; then
        if ! build "$t" g2 "$c"; then
            fail "$t $c build g2: $(first_error "$out/log/$t.g2.$(art "$c").log")"; return
        fi
        o=$(object "$t" g2 "$c")
        if ! spirv-val "$o" >"$out/log/$t.g2.$(art "$c").verify" 2>&1; then
            fail "$t $c -g spirv-val: $(head -n1 "$out/log/$t.g2.$(art "$c").verify")"; return
        fi
        spirv-dis "$o" 2>/dev/null | grep -q ' OpLine ' || { fail "$t $c -g: the module carries no OpLine"; return; }
    fi
    passes=$((passes + 1))
}

# the tools the selected columns reach, checked before anything is built
need_tool() { command -v "$1" >/dev/null 2>&1 || { echo "run.sh: $2 needs $1 on PATH" >&2; exit 2; }; }
if [ "$mode" = corpus ]; then
for t in $targets; do
    if [ "$(object_format "$t")" = spv ]; then
        need_tool spirv-val "$t"; need_tool spirv-dis "$t"
        spirv-val --version 2>/dev/null | grep -q "v$spirv_tools_version" ||
            echo "run.sh: warning: spirv-tools is not $spirv_tools_version, the spirv goldens were blessed with it"
    else
        need_tool llvm-objdump "$t"
        got_major=$(llvm-objdump --version | sed -n 's/.*LLVM version \([0-9]*\).*/\1/p' | head -n1)
        [ "$got_major" = "$objdump_major" ] ||
            echo "run.sh: warning: llvm-objdump is major $got_major, the goldens were blessed with $objdump_major"
    fi
    q=$(target_field "$t" 9)
    if [ "$qemu" -eq 1 ] && [ "$q" != - ] && [ "$(engine "$t")" = - ]; then
        echo "run.sh: warning: --qemu asked for $t but $q is not on PATH, so $t is golden only"
    fi
    [ "$(engine "$t")" = - ] || need_tool "${CC:-cc}" "the $t differential"
    if [ "$(engine "$t")" != - ] && runs_bare "$t"; then need_tool python3 "the $t differential"; fi
    case "$(object_format "$t")" in elf|macho|coff) [ "$dwarf" -eq 0 ] || need_tool llvm-dwarfdump --dwarf ;; esac
done
echo "targets: $targets"
echo "cases:   $(echo $cases | wc -w)"
materialise
for t in $targets; do
    fmt=$(object_format "$t")
    eng=$(engine "$t")
    case "$eng" in
        -) how="golden only" ;;
        '') how="golden + native differential" ;;
        *) how="golden + differential under $eng" ;;
    esac
    [ "$dwarf" -eq 1 ] && how="$how + dwarf"
    echo "target:  $t ($how)"
    before=$noruns
    for c in $cases; do run_case "$t" "$c"; done
    [ "$noruns" -eq "$before" ] || echo "norun:   $t $((noruns - before)) cases golden only, listed in golden/$t/NORUN"
done
fi

# the link cases: test/link/cases/<name>/ is a project, case.conf says which legs
# run it, what it builds and how it is checked, and expect*.txt is the recorded
# observable. see test/README.md.
#
# `goal: test` compiles the case with `mach test` in place of `mach build`: the
# artifact's closure is loaded as a build loads it, the collected tests run
# through the leg's engine as part of the compile step, and the artifact is the
# test dispatcher.
# a defect that exists only under a test build (#3535) is reachable by no other
# goal.
link_cell() {
    dir=$1; leg=$2; profile=$3
    id=$(basename "$dir")
    label="$id [$leg/$profile]"
    build_target=${case_target:-$leg}
    runner=$(engine "$leg")
    case "$runner" in '') eng=native ;; *) eng="qemu:$runner" ;; esac
    goal_flags=
    [ "$case_goal" = test ] && [ -n "$runner" ] && goal_flags="--runner $runner"
    tmp=$(mktemp -d)
    rm -rf "$dir/out/link"; mkdir -p "$dir/out/link"
    bin=$dir/out/link/prog$exe

    # a self-host case is compiled by the compiler cross-built for the leg and run
    # under the leg's engine; the cross-build's -o has to sit inside the repo
    buildcc=$mach
    if [ -n "$case_self_host" ]; then
        rel=${dir#"$repo"/}/out/link/selfhostcc$exe
        if ! (cd "$repo" && "$mach" build . --target "$case_self_host" --profile "$profile" -o "$rel") >"$tmp/selfhost.log" 2>&1; then
            fail "$label self-host cross-build: $(first_error "$tmp/selfhost.log")"; rm -rf "$tmp"; return
        fi
        case "$eng" in qemu:*) buildcc="${eng#qemu:} $repo/$rel" ;; *) buildcc=$repo/$rel ;; esac
    fi
    if (cd "$dir" && "$mach" dep pull . && $buildcc "$case_goal" . --target "$build_target" --profile "$profile" $case_build_flags $goal_flags -o "out/link/prog$exe") >"$tmp/build.log" 2>&1; then
        built=1
    else
        built=0
    fi

    case "$case_run" in
        build-fails)
            if [ "$built" -eq 1 ]; then fail "$label built, expected a link error"; rm -rf "$tmp"; return; fi
            grep '^error:' "$tmp/build.log" >"$tmp/out.txt"
            [ -s "$tmp/out.txt" ] || { fail "$label failed without an 'error:' diagnostic: $(tail -n1 "$tmp/build.log")"; rm -rf "$tmp"; return; }
            ;;
        *)
            if [ "$built" -eq 0 ]; then
                fail "$label $case_goal: $(first_error "$tmp/build.log")"; tail -n 6 "$tmp/build.log" | sed 's/^/    /'; rm -rf "$tmp"; return
            fi
            gbin=
            if [ "$case_gbuild" = yes ]; then
                gbin=$dir/out/link/prog-g$exe
                if ! (cd "$dir" && $buildcc "$case_goal" . --target "$build_target" --profile "$profile" $case_build_flags $goal_flags -g -o "out/link/prog-g$exe") >"$tmp/build-g.log" 2>&1; then
                    fail "$label $case_goal -g: $(first_error "$tmp/build-g.log")"; rm -rf "$tmp"; return
                fi
            fi
            # a fixture-owned .so the case's own steps built has to be findable at run time
            so_dirs=$(find "$dir/out" -name '*.so*' -exec dirname {} \; 2>/dev/null | sort -u | tr '\n' ':')
            export LD_LIBRARY_PATH="$so_dirs${base_ld_library_path}"
            if [ -f "$dir/check.sh" ]; then check=$dir/check.sh; else check=$here/link/check/$case_run.sh; fi
            if [ -f "$check" ]; then
                bash "$check" "$eng" "$leg" "$bin" "$gbin" "$profile" >"$tmp/out.txt" 2>"$tmp/err.txt"; rc=$?
                if [ "$rc" -ne 0 ]; then
                    fail "$label check exit $rc"; sed 's/^/    /' "$tmp/err.txt"; rm -rf "$tmp"; return
                fi
            elif [ "$case_run" = exec ] && [ "$case_goal" = test ]; then
                # the dispatcher runs one test per invocation, `<exe> <index>`; the
                # observable is every collected test's stdout in collection order
                if ! (cd "$dir" && $buildcc test . --target "$build_target" --profile "$profile" $case_build_flags --list --format json) >"$tmp/list.json" 2>"$tmp/err.txt"; then
                    fail "$label test --list: $(first_error "$tmp/err.txt")"; rm -rf "$tmp"; return
                fi
                n=$(grep -c '"event":"case"' "$tmp/list.json")
                [ "$n" -gt 0 ] || { fail "$label collected no tests"; rm -rf "$tmp"; return; }
                : >"$tmp/out.txt"; i=0
                while [ "$i" -lt "$n" ]; do
                    $runner "$bin" "$i" >>"$tmp/out.txt" 2>"$tmp/err.txt"; rc=$?
                    if [ "$rc" -ne 0 ]; then
                        fail "$label test $i exit $rc"; sed 's/^/    /' "$tmp/out.txt" "$tmp/err.txt"; rm -rf "$tmp"; return
                    fi
                    i=$((i + 1))
                done
            elif [ "$case_run" = exec ]; then
                $runner "$bin" >"$tmp/out.txt" 2>"$tmp/err.txt"; rc=$?
                if [ "$rc" -ne 0 ]; then
                    fail "$label exit $rc"; sed 's/^/    /' "$tmp/out.txt" "$tmp/err.txt"; rm -rf "$tmp"; return
                fi
            elif [ "$case_run" = built ]; then
                [ -s "$bin" ] || { fail "$label artifact missing or empty"; rm -rf "$tmp"; return; }
                echo "built=1" >"$tmp/out.txt"
            else
                fail "$label no check script for run mode '$case_run'"; rm -rf "$tmp"; return
            fi
            ;;
    esac

    # the most specific recorded observable wins
    golden=
    for g in "expect.$build_target.$profile.txt" "expect.$profile.txt" "expect.$build_target.txt" expect.txt; do
        [ -f "$dir/$g" ] && { golden=$dir/$g; break; }
    done
    if [ "$bless" -eq 1 ]; then
        [ -n "$golden" ] || golden=$dir/expect.txt
        if [ ! -f "$golden" ] || ! cmp -s "$golden" "$tmp/out.txt"; then
            [ -f "$golden" ] && diff -u "$golden" "$tmp/out.txt" | sed 's/^/    /'
            cp "$tmp/out.txt" "$golden"
            echo "BLESS $label -> $(basename "$golden")"
        fi
        passes=$((passes + 1))
    elif [ -z "$golden" ]; then
        fail "$label no expect file; run --bless"
    elif ! cmp -s "$golden" "$tmp/out.txt"; then
        fail "$label differs from $(basename "$golden")"; diff -u "$golden" "$tmp/out.txt" | sed 's/^/    /'
    else
        passes=$((passes + 1))
    fi
    rm -rf "$tmp"
}

# read_case_conf <dir>: the case's defaults, then its case.conf
read_case_conf() {
    case_legs=; case_skip=; case_profiles="debug release"; case_run=exec
    case_target=; case_build_flags=; case_self_host=; case_gbuild=no; case_goal=build
    [ -f "$1/case.conf" ] || return 0
    while IFS= read -r line || [ -n "$line" ]; do
        case "$line" in ''|\#*) continue ;; esac
        key=$(echo "${line%%:*}" | tr -d '[:space:]')
        value=$(echo "${line#*:}" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')
        case "$key" in
            legs) case_legs=$value ;; skip) case_skip=$value ;; profiles) case_profiles=$value ;;
            run) case_run=$value ;; target) case_target=$value ;; build-flags) case_build_flags=$value ;;
            self-host) case_self_host=$value ;; gbuild) case_gbuild=$value ;;
            goal) case_goal=$value ;;
            *) echo "run.sh: $1/case.conf: unknown key '$key'" >&2; exit 2 ;;
        esac
    done <"$1/case.conf"
    case "$case_goal" in
        build|test) ;;
        *) echo "run.sh: $1/case.conf: goal is 'build' or 'test', not '$case_goal'" >&2; exit 2 ;;
    esac
}

# inc_build <project> <what> <dest>: -o must sit inside the project, so dest is relative
inc_build() {
    if "$mach" build "$1" -o "$3" >"$out/log/incremental.log" 2>&1 && [ -f "$1/$3" ]; then return 0; fi
    fail "incremental: $2 did not build"
    sed 's/^/  /' "$out/log/incremental.log"
    return 1
}

# inc_same <a> <b> <what>: a pass when equal, a failure otherwise
inc_same() {
    if cmp -s "$1" "$2"; then passes=$((passes + 1)); else fail "incremental: $3"; fi
}

# inc_changed <a> <b> <what>: an edit that leaves the output unchanged proves nothing
inc_changed() {
    if cmp -s "$1" "$2"; then fail "incremental: $3 did not change the output, the check proves nothing"; return 1; fi
}

# the warm path the query engine drives, which a from-scratch fixpoint never
# reaches: a warm rebuild matches a clean one with no change (reuse is sound) and
# after an edit (invalidation is sound, #2045). a source edit and a manifest-only
# fixture cover the two invalidation channels without one masking the other.
if [ "$mode" = incremental ]; then
    mkdir -p "$out/log"
    self=$out/incremental/self
    rm -rf "$out/incremental"
    mkdir -p "$self/dep/std"
    cp -r "$repo/src" "$repo/mach.toml" "$self/"
    cp -r "$repo/dep/std/src" "$repo/dep/std/mach.toml" "$self/dep/std/"
    # the copy is no git checkout, so std is the pinned tree taken by path
    sed -i '/^\[dep\.std\]$/,/^$/{s/^git = .*$/path = "dep\/std"/;/^ref = /d;/^version = /d}' "$self/mach.toml"
    grep -q '^path = "dep/std"$' "$self/mach.toml" || fail "incremental: the copied manifest still names std by git"
    echo "incremental: $self"
    if inc_build "$self" "the clean build" o/clean &&
        inc_build "$self" "the warm no-op rebuild" o/warm; then
        inc_same "$self/o/clean" "$self/o/warm" "a warm no-op rebuild differs from the clean build"
        cp "$self/o/clean" "$out/incremental/clean"
        sed -i 's/^pub val MACH_VERSION: str = "\(.*\)";$/pub val MACH_VERSION: str = "\1-inc";/' "$self/src/lang/version.mach"
        if ! grep -q -- '-inc";$' "$self/src/lang/version.mach"; then
            fail "incremental: the version edit did not apply to src/lang/version.mach"
        elif inc_build "$self" "the warm rebuild after a source edit" o/warm_edit; then
            cp "$self/o/warm_edit" "$out/incremental/warm_edit"
            rm -rf "$self/o" "$self/out"
            if inc_build "$self" "the clean rebuild after a source edit" o/clean_edit &&
                inc_changed "$out/incremental/clean" "$self/o/clean_edit" "the source edit"; then
                inc_same "$out/incremental/warm_edit" "$self/o/clean_edit" "a warm rebuild after a source edit differs from a clean one (stale invalidation)"
            fi
        fi
    fi

    fix=$out/incremental/fixture
    mkdir -p "$fix/src"
    abi=$(printf '%s\n' "$targets_all" | awk -v i="$host_isa" -v o="$host_os" '$2 == i && $3 == o { print $4; exit }')
    cat >"$fix/mach.toml" <<EOF
[project]
id = "inc"
version = "1.0.0"
src = "src"
out = "out/{target.name}/{profile.name}"

[target.host]
isa = "$host_isa"
os  = "$host_os"
abi = "$abi"

[profile.debug]
default = true
opt = 0
debug = false
simd = "scalarize"
vectorize = false
float_reassoc = false

[artifact.inc]
kind = "static"
entry = "main.mach"
out = "lib/inc"
targets = ["*"]
link = []
need = []
EOF
    cat >"$fix/src/main.mach" <<'EOF'
#[symbol("inc_version_byte")]
pub fun inc_version_byte(i: u64) u8 {
    val v: *u8 = $project.version::*u8;
    ret v[i];
}
EOF
    if inc_build "$fix" "the fixture's clean build" o/clean; then
        cp "$fix/o/clean" "$out/incremental/fixture_clean"
        sed -i 's/^version = "1.0.0"$/version = "1.0.1"/' "$fix/mach.toml"
        if inc_build "$fix" "the fixture's warm rebuild after a manifest edit" o/warm_edit; then
            cp "$fix/o/warm_edit" "$out/incremental/fixture_warm_edit"
            rm -rf "$fix/o" "$fix/out"
            if inc_build "$fix" "the fixture's clean rebuild after a manifest edit" o/clean_edit &&
                inc_changed "$out/incremental/fixture_clean" "$fix/o/clean_edit" "the manifest edit"; then
                inc_same "$out/incremental/fixture_warm_edit" "$fix/o/clean_edit" "a warm rebuild after a manifest-only edit differs from a clean one"
            fi
        fi
    fi
fi

if [ "$mode" = link ]; then
    base_ld_library_path=${LD_LIBRARY_PATH:-}
    # a link case is ABI and loader evidence, which qemu is not, so a leg is
    # emulated only when no native leg can prove it; aarch64-linux has one
    link_emulated="riscv64-linux"
    legs=
    for t in x86_64-linux aarch64-linux riscv64-linux x86_64-windows x86_64-darwin aarch64-darwin; do
        case "$(engine "$t")" in
            -) continue ;;
            '') ;;
            *) case " $link_emulated " in *" $t "*) ;; *) continue ;; esac ;;
        esac
        legs="$legs $t"
    done
    echo "legs:   $legs"
    for dir in "$here"/link/cases/*/; do
        dir=${dir%/}
        [ -f "$dir/mach.toml" ] || continue
        id=$(basename "$dir")
        if [ -n "$want_cases" ]; then
            case " $want_cases " in *" $id "*) ;; *) continue ;; esac
        fi
        read_case_conf "$dir"
        for leg in $legs; do
            if [ -n "$case_legs" ]; then case " $case_legs " in *" $leg "*) ;; *) continue ;; esac; fi
            case " $case_skip " in *" $leg "*) skips=$((skips + 1)); continue ;; esac
            for profile in $case_profiles; do link_cell "$dir" "$leg" "$profile"; done
        done
    done
fi

# doc_extract <page.md> <dir>: one directory per mach block, <dir>/<nnn>/, holding
# src/ (split at `# file: src/<path>` lines, main.mach before the first) and meta:
# the fence line number, the fence info after `mach`, and the entry file
doc_extract() {
    awk -v dir="$2" '
        function open_file(rel) {
            if (cur != "") close(cur)
            cur = blk "/src/" rel
            d = cur; sub(/\/[^\/]*$/, "", d)
            system("mkdir -p \"" d "\"")
            printf "" > cur
            last = rel
        }
        function finish() {
            if (cur != "") close(cur)
            entry = (have_main || last == "") ? "main.mach" : last
            if (last == "") { printf "" > (blk "/src/main.mach"); close(blk "/src/main.mach") }
            print line > (blk "/meta"); print info > (blk "/meta"); print entry > (blk "/meta")
            close(blk "/meta")
            inb = 0
        }
        { sub(/\r$/, "") }
        !inb && /^```mach([ \t]|$)/ {
            inb = 1; n++; line = NR; cur = ""; last = ""; have_main = 0
            info = substr($0, 8); sub(/^[ \t]+/, "", info); sub(/[ \t]+$/, "", info)
            blk = dir "/" sprintf("%03d", n)
            system("mkdir -p \"" blk "/src\"")
            next
        }
        inb && /^```[ \t]*$/ { finish(); next }
        inb && /^# file: src\/[^ ]+\.mach[ \t]*$/ {
            rel = $3; sub(/^src\//, "", rel)
            if (rel == "main.mach") have_main = 1
            open_file(rel); next
        }
        inb {
            if (cur == "") open_file("main.mach")
            print > cur
        }
        END { if (inb) { print "unterminated mach block at line " line > "/dev/stderr"; exit 1 } }
    ' "$1"
}

# doc_cell <block dir> <label>: build the block as its own project, id `example`,
# and write pass, skip or FAIL <why> to <block dir>/result
doc_cell() {
    b=$1; label=$2
    { read -r line; read -r info; read -r entry; } <"$b/meta"
    label="$label:$line"
    annot=${info%%[ 	]*}
    expect=${info#"$annot"}
    expect=${expect#"${expect%%[! 	]*}"}
    case "$annot" in
        fragment) echo skip >"$b/result"; return ;;
        ''|error) : ;;
        *) echo "FAIL $label unknown block annotation '$annot' (fragment or error)" >"$b/result"; return ;;
    esac
    if [ "$annot" = error ] && [ -z "$expect" ]; then
        echo "FAIL $label an error block names the diagnostic it expects: \`\`\`mach error <text>" >"$b/result"; return
    fi
    kind=static; art_out=lib/block.a
    if grep -rqF '#[symbol("main")]' "$b/src"; then kind=bin; art_out=bin/block; fi
    mkdir -p "$b/dep/std"
    cp -R "$docs_std/src" "$b/dep/std/src"
    cp "$docs_std/mach.toml" "$b/dep/std/"
    {
        echo '[project]'; echo 'id = "example"'; echo 'version = "0.0.0"'; echo 'src = "src"'
        echo 'out = "o"'; echo "mach = \"^$docs_major.0\""; echo
        echo "[target.$docs_target]"
        echo "isa = \"$(target_field "$docs_target" 2)\""
        echo "os  = \"$(target_field "$docs_target" 3)\""
        echo "abi = \"$(target_field "$docs_target" 4)\""; echo
        echo '[profile.debug]'; echo 'opt = 0'; echo 'debug = false'; echo 'simd = "scalarize"'
        echo 'vectorize = true'; echo 'float_reassoc = false'; echo
        echo '[artifact.block]'; echo "kind = \"$kind\""; echo "entry = \"$entry\""
        echo "out = \"$art_out\""; echo "targets = [\"$docs_target\"]"; echo 'link = []'; echo 'need = []'; echo
        echo '[dep.std]'; echo 'path = "dep/std"'
    } >"$b/mach.toml"
    "$mach" build "$b" >"$b/log" 2>&1; rc=$?
    rm -rf "$b/dep"
    if [ "$annot" = error ]; then
        if [ "$rc" -eq 0 ]; then
            echo "FAIL $label an error block compiled; expected a diagnostic containing '$expect'" >"$b/result"
        elif ! grep -qF -- "$expect" "$b/log"; then
            echo "FAIL $label an error block failed without '$expect': $(first_error "$b/log")" >"$b/result"
        else
            echo "pass error" >"$b/result"
        fi
        return
    fi
    if [ "$rc" -ne 0 ]; then
        echo "FAIL $label does not compile: $(first_error "$b/log")" >"$b/result"; return
    fi
    if [ "$kind" = static ] || [ "$(engine "$docs_target")" != "" ]; then
        echo "pass compiled" >"$b/result"; return
    fi
    bin=$b/o/bin/block$exe
    timeout 60 "$bin" </dev/null >"$b/run.out" 2>&1; rc=$?
    if [ "$rc" -ne 0 ]; then
        echo "FAIL $label exits $rc: $(tail -n1 "$b/run.out")" >"$b/result"; return
    fi
    echo "pass run" >"$b/result"
}

if [ "$mode" = docs ]; then
    if [ -n "$want_targets" ]; then
        set -- $want_targets
        [ $# -eq 1 ] && [ "$(target_field "$1" 7)" = hosted ] ||
            { echo "run.sh: --docs takes one hosted --target" >&2; exit 2; }
        docs_target=$1
    else
        docs_target=$(printf '%s\n' "$targets_all" |
            awk -v i="$host_isa" -v o="$host_os" '$2 == i && $3 == o && $7 == "hosted" { print $1; exit }')
        [ -n "$docs_target" ] || { echo "run.sh: --docs has no hosted target for $host_os/$host_isa" >&2; exit 2; }
    fi
    echo "target:   $docs_target"
    docs_major=$("$mach" info 2>/dev/null | sed -n '1s/^mach \([0-9][0-9]*\)\..*/\1/p')
    [ -n "$docs_major" ] || docs_major=5
    docs_std=$repo/dep/std
    [ -f "$docs_std/mach.toml" ] || { echo "run.sh: --docs needs the std checkout at $docs_std" >&2; exit 2; }
    jobs_max=${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)}
    root=$out/docs
    rm -rf "$root"
    mkdir -p "$root"
    echo "docs:     $docs"
    pages=$want_cases
    [ -n "$pages" ] || pages=$(cd "$docs" && for f in *.md; do echo "${f%.md}"; done | LC_ALL=C sort | tr '\n' ' ')
    for pg in $pages; do
        doc_extract "$docs/$pg.md" "$root/$pg" || { fail "$pg.md cannot be read"; continue; }
        [ -d "$root/$pg" ] || continue
        for b in "$root/$pg"/*/; do
            b=${b%/}
            while [ "$(jobs -rp | wc -l)" -ge "$jobs_max" ]; do sleep 0.1; done
            doc_cell "$b" "$pg.md" &
        done
    done
    wait
    compiled=0; ran=0; fragments=0; errors=0; total=0
    for pg in $pages; do
        [ -d "$root/$pg" ] || continue
        for b in "$root/$pg"/*/; do
            total=$((total + 1))
            r=$(cat "${b%/}/result" 2>/dev/null || echo "FAIL ${b%/} left no result")
            case "$r" in
                skip)            fragments=$((fragments + 1)); skips=$((skips + 1)) ;;
                "pass compiled") compiled=$((compiled + 1)); passes=$((passes + 1)) ;;
                "pass run")      compiled=$((compiled + 1)); ran=$((ran + 1)); passes=$((passes + 1)) ;;
                "pass error")    errors=$((errors + 1)); passes=$((passes + 1)) ;;
                *)               echo "$r"; fails=$((fails + 1)) ;;
            esac
        done
    done
    echo "docs: $total blocks, $compiled compiled ($ran run), $errors error, $fragments fragment"
fi

tail=; [ "$unruns" -eq 0 ] || tail=", $unruns differential not run"
echo "run.sh: $passes pass, $fails fail, $skips skip$tail"
[ "$fails" -eq 0 ]
