#!/usr/bin/env bash
# ci-legs.sh: the one place that decides which ci.yml legs a run needs (#3945).
#
#   ci-legs.sh all           every leg: a pull request into main and heavy=all
#   ci-legs.sh heavy <job>   the light tier plus one heavy job (workflow_dispatch)
#   ci-legs.sh paths         the legs the changed paths on stdin select (a pull
#                            request into dev), one path per line
#
# prints key=value lines for $GITHUB_OUTPUT, then one line per path naming the
# legs it selected on stderr. a path the table does not know selects every leg,
# so a new directory is over-tested until it gets a row, never under-tested.
#
# legs:
#   compiler            the fixpoint, the unit suite and the warm build path
#   corpus:<target>     that target's column of the corpus differential
#   qemu riscv32 spirv dwarf darwin release   the heavy job of that name
#   link:<leg>          the link cases on that host leg (x86_64-linux carries
#                       every cross-built format)
# docs runs on every pull request and is not a leg.
set -euo pipefail

corpus_all='x86_64-linux aarch64-linux riscv64-linux riscv64zkt-linux x86_64-windows x86_64-darwin aarch64-darwin riscv32'
link_all='x86_64-linux aarch64-linux x86_64-windows'

legs=
add() { for l in "$@"; do legs="$legs $l"; done; }
corpus() { for t in "$@"; do add "corpus:$t"; done; }
link() { for t in "$@"; do add "link:$t"; done; }
every_corpus() { corpus $corpus_all; }
every_link() { link $link_all; add darwin; }
x64() { corpus x86_64-linux x86_64-windows x86_64-darwin; }
arm64() { corpus aarch64-linux aarch64-darwin; }
riscv() { corpus riscv64-linux riscv64zkt-linux riscv32; add qemu riscv32; }
# column <target>: one test/run.sh column with whichever job executes it
column() {
    case "$1" in
        spirv) add spirv ;;
        riscv32) corpus riscv32; add riscv32 ;;
        riscv64*) corpus "$1"; add qemu ;;
        *) corpus "$1" ;;
    esac
}
everything() { add compiler qemu riscv32 spirv dwarf release; every_corpus; every_link; }
light() { add compiler; every_corpus; }

# the table: first match wins, so a narrow row sits above the wider one it
# refines. every src/ row adds compiler, since every file there is the compiler.
select_path() {
    case "$1" in
        doc/*|*.md|LICENSE|.github/assets/*|.agents/*) ;;
        dist/*) ;;

        src/lang/be/codegen/dwarf.mach|src/lang/be/codegen/debug_input.mach|\
        src/lang/be/linker/debug.mach|src/lang/target/of/macho/dwarf.mach)
            add compiler dwarf ;;
        src/lang/target/isa/spirv/*|src/lang/target/isa/spirv.mach|\
        src/lang/target/abi/spirv.mach|src/lang/target/of/spv.mach)
            add compiler spirv ;;

        src/lang/target/isa/x64/*|src/lang/target/isa/x64.mach)
            add compiler; x64 ;;
        src/lang/target/isa/arm64/*|src/lang/target/isa/arm64.mach)
            add compiler; arm64 ;;
        src/lang/target/isa/riscv/*|src/lang/target/isa/riscv.mach|src/lang/target/abi/riscv.mach)
            add compiler; riscv ;;
        src/lang/target/abi/sysv.mach)
            add compiler; corpus x86_64-linux x86_64-darwin ;;
        src/lang/target/abi/aapcs64.mach)
            add compiler; arm64 ;;
        src/lang/target/abi/win64.mach)
            add compiler; corpus x86_64-windows ;;

        src/lang/target/os/linux.mach)
            add compiler; corpus x86_64-linux aarch64-linux riscv64-linux riscv64zkt-linux; link x86_64-linux aarch64-linux ;;
        src/lang/target/os/windows.mach)
            add compiler; corpus x86_64-windows; link x86_64-linux x86_64-windows ;;
        src/lang/target/os/darwin.mach)
            add compiler darwin; corpus x86_64-darwin aarch64-darwin; link x86_64-linux ;;
        src/lang/target/os/freestanding.mach)
            add compiler spirv riscv32; corpus riscv32 ;;

        src/lang/target/of/elf.mach)
            add compiler; link x86_64-linux aarch64-linux ;;
        src/lang/target/of/coff/*|src/lang/target/of/coff.mach|src/lang/target/of/coff_unwind_runtime.mach|\
        src/lang/target/of/rsrc.mach)
            add compiler; link x86_64-linux x86_64-windows ;;
        src/lang/target/of/macho/*|src/lang/target/of/macho.mach)
            add compiler darwin; link x86_64-linux ;;
        src/lang/target/of/*|src/lang/target/of.mach|src/lang/be/linker/*|src/lang/be/linker.mach|src/lang/be/obj.mach)
            add compiler; every_link ;;

        src/lang/target/*|src/lang/target.mach|src/lang/be/*|src/lang/me/*)
            add compiler; every_corpus ;;
        src/*|mach.toml|dep/*|.gitmodules|test/fuzz/*)
            add compiler ;;

        test/cases/SKIPS.*|test/cases/NORUN.*|test/cases/ONLY.*)
            column "${1#test/cases/*.}" ;;
        test/cases/*|test/ref/*|test/lib/*)
            every_corpus; add qemu riscv32 ;;
        test/link/*)
            every_link ;;

        *)
            everything ;;
    esac
}

case "${1:-}" in
    all) everything ;;
    heavy)
        light
        case "${2:-none}" in
            none) ;;
            all) everything ;;
            link) every_link ;;
            qemu|spirv|riscv32|dwarf|darwin|release) add "$2" ;;
            *) echo "ci-legs: unknown heavy job '${2:-}'" >&2; exit 2 ;;
        esac ;;
    paths)
        while IFS= read -r p; do
            [ -n "$p" ] || continue
            before=$legs
            select_path "$p"
            echo "$p:${legs#"$before"}" >&2
        done ;;
    *) echo "usage: ci-legs.sh all | heavy <job> | paths < changed-paths" >&2; exit 2 ;;
esac

has() { case " $legs " in *" $1 "*) return 0 ;; esac; return 1; }
flag() { if has "$1"; then echo "$1=true"; else echo "$1=false"; fi; }
# json_list <prefix> <members...>: the members selected under prefix, in table order
json_list() {
    prefix=$1; shift
    out=
    for m in "$@"; do has "$prefix:$m" && out="$out,\"$m\""; done
    echo "[${out#,}]"
}

corpus_json=$(json_list corpus $corpus_all)
link_json=$(json_list link $link_all)
for leg in compiler qemu riscv32 spirv dwarf darwin release; do flag "$leg"; done
echo "corpus=$corpus_json"
echo "link=$link_json"
# build carries the compiler artifact every leg but docs, darwin and release reads
if has compiler || has qemu || has riscv32 || has spirv || has dwarf ||
   [ "$corpus_json" != '[]' ] || [ "$link_json" != '[]' ]; then
    echo build=true
else
    echo build=false
fi
