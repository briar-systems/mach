#!/usr/bin/env bash
# run.sh — the integration-test harness.
#
# usage: run.sh --target <name> [--bless] [--filter <glob>] <compiler>
#
# for each case directory under int/{surface,regression}/ that holds a mach.toml,
# the harness loads its defaults (overridable by an optional line-based case.conf),
# and for each applicable profile builds the case with the given compiler, runs the
# case's producer to obtain a normalized text observable, and diffs it against the
# golden. --bless writes the observable to the golden instead of diffing. the exit
# status is nonzero if any case×target×profile fails.
#
# the harness is the same on every OS (git-bash on windows, bash on linux/macOS);
# the only target-specific knowledge it holds is the run-mode looked up per target
# from targets.conf.
#
# LEG vs BUILD-TARGET. --target names the LEG: the runner the harness runs on (and
# the target an exec case builds + runs). a structural case (field / flat-loader)
# inspects or loads a binary host-side and does not execute it on the target's
# runner, so it builds a different format than the leg: case.conf `build-target`
# names the mach target to compile (defaulting to the leg), and the structural
# golden is keyed by build-target (expect.<build-target>.txt). this lets, e.g., a
# PE-ASLR or macho-PIE field case cross-build its format on the cheap linux leg and
# `od` it there, per-PR, with no runner of the format's own OS.
set -eu

usage() {
    echo "usage: run.sh --target <name> [--bless] [--filter <glob>] <compiler>" >&2
    exit 2
}

target=
bless=0
filter='*'
compiler=

while [ $# -gt 0 ]; do
    case "$1" in
        --target) shift; [ $# -gt 0 ] || usage; target=$1 ;;
        --filter) shift; [ $# -gt 0 ] || usage; filter=$1 ;;
        --bless)  bless=1 ;;
        -h|--help) usage ;;
        -*) echo "run.sh: unknown flag '$1'" >&2; usage ;;
        *)  [ -z "$compiler" ] || { echo "run.sh: unexpected argument '$1'" >&2; usage; }
            compiler=$1 ;;
    esac
    shift
done

[ -n "$target" ] || { echo "run.sh: --target is required" >&2; usage; }
[ -n "$compiler" ] || { echo "run.sh: a compiler path is required" >&2; usage; }

here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
conf="$here/targets.conf"
. "$here/lib/produce.sh"

# resolve the compiler to an absolute path; cases are built from their own dirs.
case "$compiler" in
    /*) : ;;
    *)  compiler=$(CDPATH= cd -- "$(dirname -- "$compiler")" && pwd)/$(basename -- "$compiler") ;;
esac

# the executable suffix is a property of the host that runs the artifact: native
# legs execute on the runner itself (windows -> .exe), qemu legs load a guest ELF
# (no suffix). keying off the host, not the target, keeps target handling to the
# run-mode lookup alone.
exe=
case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*) exe=.exe ;;
esac

# accept a compiler given without its host suffix (CI passes ./m uniformly; on
# windows the file is m.exe).
if [ ! -f "$compiler" ] && [ -f "$compiler$exe" ]; then
    compiler=$compiler$exe
fi

# conf_runmode <target> — print the run-mode column for a target, or fail.
conf_runmode() {
    while read -r name runner runmode rowcadence rest; do
        case "$name" in ''|\#*) continue ;; esac
        if [ "$name" = "$1" ]; then echo "$runmode"; return 0; fi
    done < "$conf"
    return 1
}

# all_targets — print every target name declared in targets.conf.
all_targets() {
    while read -r name runner runmode rowcadence rest; do
        case "$name" in ''|\#*) continue ;; esac
        echo "$name"
    done < "$conf"
}

# in_list <item> <space-separated-list>
in_list() {
    case " $2 " in *" $1 "*) return 0 ;; esac
    return 1
}

runmode=$(conf_runmode "$target") || {
    echo "run.sh: target '$target' is not in targets.conf" >&2
    exit 2
}

fails=0
ran=0

# iterate cases in a stable order across the two buckets.
for dir in "$here"/surface/$filter "$here"/regression/$filter; do
    [ -f "$dir/mach.toml" ] || continue
    case_id=${dir#"$here"/}

    # case defaults; case.conf overrides any of them (line-based `key: value`).
    case_targets=all
    case_exempt=
    case_profiles="debug release"
    case_run=exec
    case_build_target=
    case_build_flags=
    if [ -f "$dir/case.conf" ]; then
        while IFS= read -r line || [ -n "$line" ]; do
            case "$line" in ''|\#*) continue ;; esac
            key=${line%%:*}
            value=${line#*:}
            key=$(echo "$key" | tr -d '[:space:]')
            value=$(echo "$value" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')
            case "$key" in
                targets)        case_targets=$value ;;
                exempt)         case_exempt=$value ;;
                profiles)       case_profiles=$value ;;
                run)            case_run=$value ;;
                build-target)   case_build_target=$value ;;
                build-flags)    case_build_flags=$value ;;
                *) echo "run.sh: $case_id/case.conf: unknown key '$key'" >&2; exit 2 ;;
            esac
        done < "$dir/case.conf"
    fi

    # resolve the applicable target set: the allowlist (all, or an explicit list)
    # minus the exempt list.
    if [ "$case_targets" = all ]; then
        allow=$(all_targets | tr '\n' ' ')
    else
        allow=$case_targets
    fi
    if ! in_list "$target" "$allow"; then continue; fi
    if in_list "$target" "$case_exempt"; then continue; fi

    # the mach target to compile: the build-target if set, else the leg itself.
    build_target=${case_build_target:-$target}

    # the golden is shared across build-targets for runtime observables (exec and the
    # relro-fault guard, whose output is target-independent) and per-build-target for
    # structural producers (their fact is format-specific).
    case "$case_run" in
        exec|relro-fault) golden="$dir/expect.txt" ;;
        *)                golden="$dir/expect.$build_target.txt" ;;
    esac

    for profile in $case_profiles; do
        ran=$((ran + 1))
        label="$case_id [$target/$profile]"

        # the build artifact goes under the case's gitignored out/ via a path
        # relative to the case dir: a native windows compiler resolves it against
        # its cwd, where an absolute MSYS scratch path (/tmp/...) would not. the
        # mktemp scratch holds only logs and captured output, read by bash alone.
        tmp=$(mktemp -d)
        relbin="out/int/prog$exe"
        bin="$dir/$relbin"
        rm -rf "$dir/out/int"
        mkdir -p "$dir/out/int"

        if ! (cd "$dir" && "$compiler" dep pull && "$compiler" build . --target "$build_target" --profile "$profile" $case_build_flags -o "$relbin") >"$tmp/build.log" 2>&1; then
            echo "FAIL $label (build)"
            sed 's/^/    /' "$tmp/build.log" >&2
            fails=$((fails + 1))
            rm -rf "$tmp" "$dir/out/int"
            continue
        fi

        if produce "$case_run" "$runmode" "$target" "$bin" >"$tmp/out.txt" 2>"$tmp/err.txt"; then
            prc=0
        else
            prc=$?
        fi
        if [ "$prc" -ne 0 ]; then
            echo "FAIL $label (producer exit $prc)"
            sed 's/^/    /' "$tmp/err.txt" >&2
            fails=$((fails + 1))
            rm -rf "$tmp" "$dir/out/int"
            continue
        fi

        if [ "$bless" -eq 1 ]; then
            cp "$tmp/out.txt" "$golden"
            echo "BLESS $label -> ${golden#"$here"/}"
            rm -rf "$tmp" "$dir/out/int"
            continue
        fi

        if [ ! -f "$golden" ]; then
            echo "FAIL $label (no golden ${golden#"$here"/}; run with --bless)"
            fails=$((fails + 1))
            rm -rf "$tmp" "$dir/out/int"
            continue
        fi

        if diff -u "$golden" "$tmp/out.txt" >"$tmp/diff.txt" 2>&1; then
            echo "PASS $label"
        else
            echo "FAIL $label (diff)"
            sed 's/^/    /' "$tmp/diff.txt" >&2
            fails=$((fails + 1))
        fi
        rm -rf "$tmp" "$dir/out/int"
    done
done

if [ "$ran" -eq 0 ]; then
    echo "run.sh: no cases matched --filter '$filter' for target '$target'" >&2
    exit 1
fi

if [ "$fails" -ne 0 ]; then
    echo "int: $fails failure(s) on $target"
    exit 1
fi
echo "int: all cases passed on $target"
