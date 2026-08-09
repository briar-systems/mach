#!/usr/bin/env bash
# case.sh — the leg registry and the case manifest, the two things run.sh reads
# before it builds anything.
#
# the leg registry is test/engines.conf, shared with the codegen corpus. one file
# names every target this repo registers, how each executes and which runner it
# runs on, so a leg cannot exist for one suite and not the other and the two can
# never disagree about what a name means.

_case_lib_dir=$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
_engines_conf="$_case_lib_dir/../../engines.conf"

# engines_field <leg> <column> — one column of a leg's row. columns are numbered as
# engines.conf documents them: 1 name, 2 isa, 3 os, 4 abi, 5 of, 6 kind, 7 entry,
# 8 engine, 9 disasm, 10 runner.
engines_field() {
    awk -v want="$1" -v col="$2" '
        /^[[:space:]]*#/ || /^[[:space:]]*$/ { next }
        $1 == want { print $col; found = 1; exit }
        END { exit !found }
    ' "$_engines_conf"
}

# all_legs — every row that is a MACHINE: it declares a runner and it executes.
#
# a leg is somewhere a case runs, which is not the same set as the targets the
# registry carries. `engine none` says a row executes nothing, so spirv, mos6502 and
# riscv32 are columns the x86_64-linux machine cross-builds and reads, never machines
# of their own - a link case reaches them through case.conf's `target:`, which is the
# axis that exists for exactly this. a row whose runner is `-` has nowhere to run at
# all and is no leg either.
all_legs() {
    awk '
        /^[[:space:]]*#/ || /^[[:space:]]*$/ { next }
        $10 != "-" && $8 != "none" { print $1 }
    ' "$_engines_conf"
}

# leg_engine <leg> — the engine column: `native`, `qemu:<command>` or `none`.
leg_engine() { engines_field "$1" 8; }

# in_list <item> <space-separated-list>
in_list() {
    case " $2 " in *" $1 "*) return 0 ;; esac
    return 1
}

# read_case_conf <case-dir> — a case's defaults, then its case.conf overrides.
#
# sets case_legs, case_skip, case_profiles, case_run, case_target, case_build_flags,
# case_self_host and case_allow. `legs` names the machines a case runs on and
# `target` names the mach target it builds: a cross-compilation case builds a
# format its leg cannot execute, which is the whole reason the two are separate
# axes rather than one.
#
# `self-host` names the target in the REPO'S OWN mach.toml to cross-build the
# compiler for, which is a different vocabulary from this suite's legs and is
# written out rather than derived from one. the two schemes agree on four of six
# names and disagree on the rest, so a derivation would be a rule with exceptions
# and the first target that broke it would break silently.
read_case_conf() {
    dir=$1
    case_legs=all
    case_skip=
    case_profiles="debug release"
    case_run=exec
    case_target=
    case_build_flags=
    case_self_host=
    if [ -f "$dir/case.conf" ]; then
        while IFS= read -r line || [ -n "$line" ]; do
            case "$line" in ''|\#*) continue ;; esac
            key=${line%%:*}
            value=${line#*:}
            key=$(echo "$key" | tr -d '[:space:]')
            value=$(echo "$value" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')
            case "$key" in
                legs)        case_legs=$value ;;
                skip)        case_skip=$value ;;
                profiles)    case_profiles=$value ;;
                run)         case_run=$value ;;
                target)      case_target=$value ;;
                build-flags) case_build_flags=$value ;;
                self-host)   case_self_host=$value ;;
                *) echo "link: $dir/case.conf: unknown key '$key'" >&2; return 2 ;;
            esac
        done < "$dir/case.conf"
    fi
    if [ "$case_legs" = all ]; then
        case_allow=$(all_legs | tr '\n' ' ')
    else
        case_allow=$case_legs
    fi
}
