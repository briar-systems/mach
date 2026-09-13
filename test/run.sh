#!/usr/bin/env bash
# the codegen corpus, the link cases and the dwarf verify: one loop over
# test/cases/<group>/<case>.mach and the targets, grounded in external tools.
# see test/README.md for the case contract and how to add a case.
#
# usage: test/run.sh [--target <t>]... [--case <group>/<name>]... [--bless]
#                    [--qemu] [--link] [--dwarf]
#
# per case and target: build the object in release, disassemble it with the
# external decoder and diff against test/golden/<target>/<group>/<case>.dis; on a
# target this host can execute, build at O0 and O2, run both, and compare the
# checksums to the C reference built from test/ref/<group>/<case>.c. spirv is
# built, validated with spirv-val, and diffed through spirv-dis.
#
#   --target <t>   one target (repeatable); default every target with a golden dir
#   --case <g/n>   one case (repeatable)
#   --bless        write the goldens instead of diffing them, print the diff
#   --qemu         execute a foreign linux target under qemu-<isa>
#   --link         run the link cases (test/link/cases) instead of the corpus
#   --dwarf        build every case with -g and run llvm-dwarfdump --verify
#   MACH           the compiler under test, default out/<host>/debug/bin/mach
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

# the targets: name isa os abi of kind entry decoder
targets_all='
x86_64-linux    x86_64      linux        sysv64   -    bin     hosted  objdump
aarch64-linux   aarch64     linux        aapcs64  -    bin     hosted  objdump
riscv64-linux   riscv64     linux        lp64d    -    bin     hosted  objdump
x86_64-windows  x86_64      windows      win64    -    bin     hosted  objdump
x86_64-darwin   x86_64      darwin       sysv64   -    bin     hosted  objdump
aarch64-darwin  aarch64     darwin       aapcs64  -    bin     hosted  objdump
spirv           spirv       freestanding spirv    -    bin     direct  spirv-dis
riscv32         rv32imafdc  freestanding ilp32d   elf  static  direct  objdump
'

usage() { sed -n '2,20p' "$0" | sed 's/^# \{0,1\}//' >&2; exit 2; }

want_targets=
want_cases=
bless=0
qemu=0
link=0
dwarf=0
while [ $# -gt 0 ]; do
    case "$1" in
        --target) shift; [ $# -gt 0 ] || usage; want_targets="$want_targets $1" ;;
        --case)   shift; [ $# -gt 0 ] || usage; want_cases="$want_cases $1" ;;
        --bless)  bless=1 ;;
        --qemu)   qemu=1 ;;
        --link)   link=1 ;;
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
fail() { echo "FAIL $*"; fails=$((fails + 1)); }

# target_field <target> <column>
target_field() { printf '%s\n' "$targets_all" | awk -v t="$1" -v c="$2" '$1 == t { print $c }'; }

# object_format <target>
object_format() {
    isa=$(target_field "$1" 2); os=$(target_field "$1" 3); of=$(target_field "$1" 5)
    [ "$isa" = spirv ] && { echo spv; return; }
    [ "$of" != - ] && { echo "$of"; return; }
    case "$os" in linux) echo elf ;; windows) echo coff ;; darwin) echo macho ;; *) echo raw ;; esac
}

# engine <target>: "" for the host itself, the qemu command, or "-" when nothing here runs it
engine() {
    isa=$(target_field "$1" 2); os=$(target_field "$1" 3)
    if [ "$os" = "$host_os" ] && [ "$isa" = "$host_isa" ]; then echo ""; return; fi
    if [ "$qemu" -eq 1 ] && [ "$os" = linux ] && command -v "qemu-$isa" >/dev/null 2>&1; then
        echo "qemu-$isa"; return
    fi
    echo -
}

# the cases, as group/name
if [ -n "$want_cases" ]; then
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

# skipped <target> <case>: the first field of a test/golden/<target>/SKIPS line is a
# case name or a glob; the rest of the line is the reason
skipped() {
    [ -f "$here/golden/$1/SKIPS" ] || return 1
    while read -r pat rest; do
        case "$pat" in ''|\#*) continue ;; esac
        case "$2" in $pat) return 0 ;; esac
    done <"$here/golden/$1/SKIPS"
    return 1
}

# one generated project per entry shape: hosted cases print their checksum through
# std, direct cases (spirv, riscv32) are the artifact and reach no runtime at all.
materialise() {
    hosted=$out/hosted
    bare=$out/bare
    rm -rf "$hosted" "$bare"
    for proj in "$hosted" "$bare"; do
        mkdir -p "$proj/src/cases" "$proj/src/lib"
        cp "$here"/lib/fold.mach "$proj/src/lib/"
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
    printf '%s\n' "$targets_all" | while read -r name isa os abi of kind entry decoder; do
        [ -n "$name" ] && [ "$entry" = "$shape" ] || continue
        echo "[target.$name]"; echo "isa = \"$isa\""; echo "os  = \"$os\""; echo "abi = \"$abi\""
        [ "$of" != - ] && echo "of  = \"$of\""
        echo
    done
    for p in o0 o2 g; do
        echo "[profile.$p]"
        case $p in o0) echo 'opt = 0'; echo 'debug = false'; echo 'default = true' ;;
                   o2) echo 'opt = 2'; echo 'debug = false' ;;
                   g)  echo 'opt = 0'; echo 'debug = true' ;; esac
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
            printf '%s\n' "$targets_all" | while read -r name isa os abi of kind entry decoder; do
                [ -n "$name" ] && [ "$entry" = direct ] || continue
                echo "[artifact.${a}_$kind]"; echo "kind = \"$kind\""; echo "entry = \"cases/$c.mach\""
                if [ "$kind" = static ]; then echo "out = \"lib/$a.a\""; else echo "out = \"bin/$a\""; fi
                echo "targets = [\"$name\"]"; echo 'link = []'; echo 'need = []'; echo
            done
        fi
    done
    [ "$shape" = hosted ] && { echo '[dep.std]'; echo 'path = "dep/std"'; echo; }
    return 0
}

# build <target> <profile> <case> [flags]: the artifact for one case, log on failure
build() {
    t=$1; p=$2; c=$3; shift 3
    a=$(art "$c")
    if [ "$(target_field "$t" 7)" = hosted ]; then
        proj=$out/hosted; sel="--bin $a"
    else
        proj=$out/bare
        kind=$(target_field "$t" 6)
        if [ "$kind" = static ]; then sel="--lib ${a}_$kind"; else sel="--bin ${a}_$kind"; fi
    fi
    log=$out/log/$t.$p.$a.log
    mkdir -p "$out/log"
    "$mach" build "$proj" --target "$t" --profile "$p" $sel "$@" >"$log" 2>&1
}

# object <target> <profile> <case>
object() {
    fmt=$(object_format "$1")
    base=$out/$( [ "$(target_field "$1" 7)" = hosted ] && echo hosted || echo bare )/o/$1/$2/obj/corpus/cases/$3
    case "$fmt" in spv) echo "$base.spv" ;; *) echo "$base.o" ;; esac
}

# artifact <target> <profile> <case>: the executable, archive or module
artifact() {
    a=$(art "$3")
    if [ "$(target_field "$1" 7)" = hosted ]; then
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
# the three must agree before either is compared with mach
reference() {
    c=$1
    ans=$out/ref/$c.ans
    [ -f "$ans" ] && { cat "$ans"; return 0; }
    mkdir -p "$out/ref/${c%/*}"
    src=$here/ref/$c.c
    [ -f "$src" ] || { echo "no C reference at $src" >&2; return 1; }
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
    if skipped "$t" "$c"; then skips=$((skips + 1)); return; fi

    # release build, decoded and diffed against the golden
    if ! build "$t" o2 "$c"; then
        fail "$t $c build o2: $(first_error "$out/log/$t.o2.$(art "$c").log")"; return
    fi
    o=$(object "$t" o2 "$c")
    [ -f "$o" ] || { fail "$t $c o2: no object at $o"; return; }
    if [ "$fmt" = spv ]; then
        if ! spirv-val "$o" >"$out/log/$t.val.$(art "$c").log" 2>&1; then
            fail "$t $c spirv-val: $(head -n1 "$out/log/$t.val.$(art "$c").log")"; return
        fi
    fi
    golden=$here/golden/$t/$c.dis
    dis=$out/log/$t.$(art "$c").dis
    disassemble "$t" "$c" "$o" >"$dis" || { fail "$t $c disassemble"; return; }
    if [ "$bless" -eq 1 ]; then
        mkdir -p "$(dirname "$golden")"
        if [ ! -f "$golden" ] || ! cmp -s "$golden" "$dis"; then
            [ -f "$golden" ] && diff -u "$golden" "$dis" | sed 's/^/    /'
            cp "$dis" "$golden"
            echo "BLESS $t $c"
        fi
    elif [ ! -f "$golden" ]; then
        fail "$t $c golden: none at ${golden#"$here"/}; run --bless"; return
    elif ! cmp -s "$golden" "$dis"; then
        fail "$t $c golden: $(diff "$golden" "$dis" | head -n1 | sed 's/^/line /')"; return
    fi

    # the differential: mach at O0 and O2 against the C reference
    eng=$(engine "$t")
    if [ "$eng" != - ]; then
        ref=$(reference "$c" 2>"$out/log/ref.$(art "$c").err") || {
            fail "$t $c reference: $(cat "$out/log/ref.$(art "$c").err")"; return
        }
        for p in o0 o2; do
            if [ "$p" = o0 ] && ! build "$t" o0 "$c"; then
                fail "$t $c build o0: $(first_error "$out/log/$t.o0.$(art "$c").log")"; return
            fi
            bin=$(artifact "$t" "$p" "$c")
            got=$(timeout 60 $eng "$bin" 2>"$out/log/$t.$p.$(art "$c").err"); rc=$?
            if [ "$rc" -ne 0 ]; then fail "$t $c $p: exit $rc"; return; fi
            if [ -s "$out/log/$t.$p.$(art "$c").err" ]; then fail "$t $c $p: wrote to stderr"; return; fi
            if [ "$got" != "$ref" ]; then fail "$t $c $p: mach says $got, C reference says $ref"; return; fi
        done
    fi

    # the -g build through the external verifier
    if [ "$dwarf" -eq 1 ] && { [ "$fmt" = elf ] || [ "$fmt" = macho ]; }; then
        if ! build "$t" g "$c"; then
            fail "$t $c build g: $(first_error "$out/log/$t.g.$(art "$c").log")"; return
        fi
        bin=$(artifact "$t" g "$c")
        if ! llvm-dwarfdump --verify "$bin" >"$out/log/$t.g.$(art "$c").verify" 2>&1; then
            fail "$t $c dwarfdump --verify: $(grep -m1 -E 'error|warning' "$out/log/$t.g.$(art "$c").verify")"; return
        fi
    fi
    passes=$((passes + 1))
}

for tool in llvm-objdump "${CC:-cc}"; do
    command -v "$tool" >/dev/null 2>&1 || { echo "run.sh: $tool is not on PATH" >&2; exit 2; }
done
got_major=$(llvm-objdump --version | sed -n 's/.*LLVM version \([0-9]*\).*/\1/p' | head -n1)
[ "$got_major" = "$objdump_major" ] ||
    echo "run.sh: warning: llvm-objdump is major $got_major, the goldens were blessed with $objdump_major"

if [ "$link" -eq 0 ]; then
echo "targets: $targets"
echo "cases:   $(echo $cases | wc -w)"
materialise
for t in $targets; do
    fmt=$(object_format "$t")
    if [ "$fmt" = spv ]; then
        for tool in spirv-val spirv-dis; do
            command -v $tool >/dev/null 2>&1 || { echo "run.sh: $t needs $tool on PATH" >&2; exit 2; }
        done
        $tool --version 2>/dev/null | grep -q "v$spirv_tools_version" ||
            echo "run.sh: warning: spirv-tools is not $spirv_tools_version, the spirv goldens were blessed with it"
    fi
    eng=$(engine "$t")
    case "$eng" in
        -) how="golden only" ;;
        '') how="golden + native differential" ;;
        *) how="golden + differential under $eng" ;;
    esac
    [ "$dwarf" -eq 1 ] && how="$how + dwarf"
    echo "target:  $t ($how)"
    for c in $cases; do run_case "$t" "$c"; done
done
fi

# the link cases: test/link/cases/<name>/ is a project, case.conf says which legs
# run it, what it builds and how it is checked, and expect*.txt is the recorded
# observable. see test/README.md.
link_cell() {
    dir=$1; leg=$2; profile=$3
    id=$(basename "$dir")
    label="$id [$leg/$profile]"
    build_target=${case_target:-$leg}
    eng=$(engine "$leg")
    case "$eng" in '') eng=native ;; *) eng="qemu:$eng" ;; esac
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
    if (cd "$dir" && "$mach" dep pull . && $buildcc build . --target "$build_target" --profile "$profile" $case_build_flags -o "out/link/prog$exe") >"$tmp/build.log" 2>&1; then
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
            if [ "$built" -eq 0 ]; then fail "$label build: $(first_error "$tmp/build.log")"; rm -rf "$tmp"; return; fi
            gbin=
            if [ "$case_gbuild" = yes ]; then
                gbin=$dir/out/link/prog-g$exe
                if ! (cd "$dir" && $buildcc build . --target "$build_target" --profile "$profile" $case_build_flags -g -o "out/link/prog-g$exe") >"$tmp/build-g.log" 2>&1; then
                    fail "$label build -g: $(first_error "$tmp/build-g.log")"; rm -rf "$tmp"; return
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
            elif [ "$case_run" = exec ]; then
                ${eng#native} "$bin" >"$tmp/out.txt" 2>"$tmp/err.txt"; rc=$?
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
    case_target=; case_build_flags=; case_self_host=; case_gbuild=no
    [ -f "$1/case.conf" ] || return 0
    while IFS= read -r line || [ -n "$line" ]; do
        case "$line" in ''|\#*) continue ;; esac
        key=$(echo "${line%%:*}" | tr -d '[:space:]')
        value=$(echo "${line#*:}" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')
        case "$key" in
            legs) case_legs=$value ;; skip) case_skip=$value ;; profiles) case_profiles=$value ;;
            run) case_run=$value ;; target) case_target=$value ;; build-flags) case_build_flags=$value ;;
            self-host) case_self_host=$value ;; gbuild) case_gbuild=$value ;;
            *) echo "run.sh: $1/case.conf: unknown key '$key'" >&2; exit 2 ;;
        esac
    done <"$1/case.conf"
}

if [ "$link" -eq 1 ]; then
    base_ld_library_path=${LD_LIBRARY_PATH:-}
    legs=
    for t in x86_64-linux aarch64-linux riscv64-linux x86_64-windows x86_64-darwin aarch64-darwin; do
        [ "$(engine "$t")" != - ] && legs="$legs $t"
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

echo "run.sh: $passes pass, $fails fail, $skips skip"
[ "$fails" -eq 0 ]
