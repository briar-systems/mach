#!/usr/bin/env bash
# run.sh — the integration-test harness.
#
# usage: run.sh --target <name> [--runmode native|qemu] [--bless] [--filter <glob>] <compiler>
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
# from targets.conf, `--runmode` overriding it for this invocation only.
#
# LEG vs BUILD-TARGET. --target names the LEG: the runner the harness runs on (and
# the target an exec case builds + runs). a structural case (field / flat-loader)
# inspects or loads a binary host-side and does not execute it on the target's
# runner, so it builds a different format than the leg: case.conf `build-target`
# names the mach target to compile (defaulting to the leg), and the structural
# golden is keyed by build-target (expect.<build-target>.txt). this lets, e.g., a
# PE-ASLR or macho-PIE field case cross-build its format on the cheap linux leg and
# `od` it there, per-PR, with no runner of the format's own OS.
#
# NATIVE vs QEMU FIDELITY. a target's run-mode (targets.conf) is `native` (execute on
# the runner) or `qemu` (run the guest ELF under qemu-user). qemu-user does NOT fully
# model kernel memory semantics - in particular it does not fault an `mprotect` (or
# access) over address space the loader never mapped, and its guest page size need not
# match a real kernel's. so RELRO / mprotect-class runtime behavior (e.g. the static-PIE
# self-relocation re-protecting PT_GNU_RELRO) is only PROVEN by a `native` leg; a qemu
# leg can pass while the same binary crashes on real hardware (briar-systems/mach#1885,
# where a 64K-aligned aarch64 image faulted ENOMEM on a native 4K-page kernel that every
# qemu-aarch64 leg had reported green). keep the aarch64 int leg `native` for this
# reason - do not move it to qemu.
#
# --runmode OVERRIDES targets.conf FOR THIS INVOCATION ONLY; the file itself, the
# source of truth for CI's matrix, is never touched (#2314). It exists so a
# developer on an x86-64 host can exercise a leg targets.conf marks `native` -
# today, only `linux-arm64` - without hand-editing the SoT, which is exactly the
# accidental-commit risk #2314 named. It does NOT make a qemu run CI-equivalent:
# the RELRO/mprotect fidelity gap two paragraphs up applies exactly the same
# whether qemu-user runs because targets.conf says so or because `--runmode`
# said so. Use it to iterate locally; CI never passes it, so the authoritative
# lane - native on the real runner targets.conf names - is unaffected.
set -eu

usage() {
    echo "usage: run.sh --target <name> [--runmode native|qemu] [--bless] [--filter <glob>] <compiler>" >&2
    exit 2
}

target=
runmode_override=
bless=0
filter='*'
compiler=

while [ $# -gt 0 ]; do
    case "$1" in
        --target)  shift; [ $# -gt 0 ] || usage; target=$1 ;;
        --runmode) shift; [ $# -gt 0 ] || usage; runmode_override=$1
                   case "$runmode_override" in
                       native|qemu) : ;;
                       *) echo "run.sh: --runmode must be 'native' or 'qemu', got '$runmode_override'" >&2; usage ;;
                   esac ;;
        --filter)  shift; [ $# -gt 0 ] || usage; filter=$1 ;;
        --bless)   bless=1 ;;
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

# lock_commit <mach.lock> <name> — the commit mach.lock records for [dep.<name>],
# or empty when the file or the entry is absent.
lock_commit() {
    lockfile=$1
    want=$2
    [ -f "$lockfile" ] || return 0
    awk -v want="$want" '
        /^\[dep\./ { name = $0; sub(/^\[dep\./, "", name); sub(/\]$/, "", name); next }
        /^commit = / && name == want {
            sha = $0
            sub(/^commit = "/, "", sha); sub(/"$/, "", sha)
            print sha
            exit
        }
    ' "$lockfile"
}

# dep_commits <case-dir> — print "name@shortsha ..." for every git dep checked
# out under a case's dep/, or nothing when none exist.
#
# Ground-truthed from the CHECKOUT (`git rev-parse --short HEAD`), not from
# mach.lock: the lock is the RECORD of a resolution, the checkout is the
# ENTITY that was actually compiled against, and #2387's own observation was a
# case where those disagreed - lock said one commit, dep/mach-std's HEAD was on
# another (reproduced locally: a manual `git checkout` inside dep/mach-std,
# bypassing `mach dep`, leaves mach.lock exactly where it was). Reading the
# lock here would print the record, not the entity, and inherit that gap one
# level up. When the checkout and the lock disagree, this prints
# "name@<checkout>!lock=<locked>" instead of silently trusting either one.
#
# Traced (process_git_node in src/cli/cmd/dep.mach) and reproduced: `dep pull`
# itself repairs a drifted checkout LOUDLY every time it runs, restoring it to
# the lock and printing `repaired <name> (checkout drift: ...)`. Since run.sh
# always calls `dep pull` immediately before build in the same command, a case
# actually BUILT under this harness cannot disagree with its own lock by the
# time this function reads it - the mismatch marker exists as a tripwire for a
# future path that reads dep/ without first pulling (or a `dep pull` that
# somehow stopped repairing), not because the current flow produces one.
#
# A case's `ref = "branch/main"` deliberately tracks mach-std's latest RELEASE
# (moving `main` is the reviewed release event, not drift - see doc/cli.md's
# Lockfile section), so int/.gitignore excludes `mach.lock`: committing it here
# would freeze every case to whatever commit happened to be resolved when someone
# last regenerated it, contradicting that intent. But an ephemeral lock means
# nothing durable records which commit a given run actually compiled against -
# `mach dep pull`'s own lock write vanishes with the rest of the case's gitignored
# state. Printing the checkout into every PASS/FAIL/BLESS line is that record: it
# survives exactly as long as the CI log does, which is exactly as long as anyone
# would want to ask "what did this run compile against".
dep_commits() {
    dir=$1
    [ -d "$dir/dep" ] || return 0
    for depdir in "$dir"/dep/*/; do
        [ -d "${depdir}.git" ] || [ -f "${depdir}.git" ] || continue
        name=$(basename "$depdir")
        head=$(git -C "$depdir" rev-parse --short HEAD 2>/dev/null) || continue
        locked=$(lock_commit "$dir/mach.lock" "$name")
        if [ -n "$locked" ]; then
            case "$locked" in
                "$head"*) printf '%s@%s ' "$name" "$head" ;;
                *)        printf '%s@%s!lock=%s ' "$name" "$head" "$(printf '%s' "$locked" | cut -c1-7)" ;;
            esac
        else
            printf '%s@%s ' "$name" "$head"
        fi
    done
}

runmode=$(conf_runmode "$target") || {
    echo "run.sh: target '$target' is not in targets.conf" >&2
    exit 2
}
if [ -n "$runmode_override" ] && [ "$runmode_override" != "$runmode" ]; then
    echo "run.sh: --runmode overrides '$target' to $runmode_override (targets.conf says $runmode; not CI-equivalent, see NATIVE vs QEMU FIDELITY above)" >&2
    runmode=$runmode_override
fi

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
    case_self_host=false
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
                self-host)      case_self_host=$value ;;
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

    # the golden is shared across build-targets for target-independent observables (exec,
    # the relro-fault guard, and the panic-exit guard, whose outputs are all
    # target-independent; debuginfo, whose two facts — validator-clean and additive —
    # hold identically on every ELF ISA) and per-build-target for structural producers
    # (their fact is format-specific).
    case "$case_run" in
        exec|relro-fault|panic-exit|debuginfo) golden="$dir/expect.txt" ;;
        *)                                     golden="$dir/expect.$build_target.txt" ;;
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

        # the compiler that builds the case. normally the from-source compiler
        # itself; a self-host case first cross-builds that compiler to the leg
        # target and drives it under the leg's run-mode, so the case is compiled by
        # the target-hosted compiler (the riscv64 compiler under qemu-riscv64), not
        # the host compiler. this executes the compiler's own emitted code on the
        # target — the coverage running produced binaries can never reach.
        buildcc=$compiler
        if [ "$case_self_host" = true ]; then
            if ! (cd "$here/.." && "$compiler" build . --target "$target" --profile "$profile" -o "$tmp/selfhostcc") >"$tmp/selfhost.log" 2>&1; then
                echo "FAIL $label (self-host cross-build)"
                sed 's/^/    /' "$tmp/selfhost.log" >&2
                fails=$((fails + 1))
                rm -rf "$tmp" "$dir/out/int"
                continue
            fi
            if [ "$runmode" = qemu ]; then
                interp=$(qemu_bin "$target") || exit 1
                buildcc="$interp $tmp/selfhostcc"
            else
                buildcc=$tmp/selfhostcc
            fi
        fi

        if (cd "$dir" && "$compiler" dep pull && $buildcc build . --target "$build_target" --profile "$profile" $case_build_flags -o "$relbin") >"$tmp/build.log" 2>&1; then
            build_ok=1
        else
            build_ok=0
        fi

        # a label suffix naming what mach-std commit `dep pull` resolved (#2387),
        # once one exists to report; every PASS/FAIL line from here on carries it.
        deps=$(dep_commits "$dir")
        flabel=$label
        [ -n "$deps" ] && flabel="$label (${deps% })"

        # a build-fails case asserts the compile is rejected and takes the compiler's
        # 'error:' diagnostic as its observable (deterministic: no paths, just the
        # message text). every other mode requires a clean build and runs a producer
        # on the artifact.
        if [ "$case_run" = build-fails ]; then
            if [ "$build_ok" -eq 1 ]; then
                echo "FAIL $flabel (build succeeded; expected a link error)"
                fails=$((fails + 1))
                rm -rf "$tmp" "$dir/out/int"
                continue
            fi
            grep '^error:' "$tmp/build.log" >"$tmp/out.txt" || true
            if [ ! -s "$tmp/out.txt" ]; then
                echo "FAIL $flabel (build failed without an 'error:' diagnostic)"
                sed 's/^/    /' "$tmp/build.log" >&2
                fails=$((fails + 1))
                rm -rf "$tmp" "$dir/out/int"
                continue
            fi
        else
            if [ "$build_ok" -eq 0 ]; then
                echo "FAIL $flabel (build)"
                sed 's/^/    /' "$tmp/build.log" >&2
                fails=$((fails + 1))
                rm -rf "$tmp" "$dir/out/int"
                continue
            fi

            # the debuginfo producer inspects the artifact built with and without `-g`:
            # the default build above is the no-`-g` one; build the `-g` twin here (same
            # compiler / target / profile / flags, plus `-g`) and hand its path to the
            # producer as an extra argument. every other producer inspects only `$bin`.
            gbin=
            if [ "$case_run" = debuginfo ]; then
                gbin="$dir/out/int/prog-g$exe"
                if ! (cd "$dir" && $buildcc build . --target "$build_target" --profile "$profile" $case_build_flags -g -o "out/int/prog-g$exe") >"$tmp/build-g.log" 2>&1; then
                    echo "FAIL $flabel (build -g)"
                    sed 's/^/    /' "$tmp/build-g.log" >&2
                    fails=$((fails + 1))
                    rm -rf "$tmp" "$dir/out/int"
                    continue
                fi
            fi

            if produce "$case_run" "$runmode" "$target" "$bin" "$gbin" >"$tmp/out.txt" 2>"$tmp/err.txt"; then
                prc=0
            else
                prc=$?
            fi
            if [ "$prc" -ne 0 ]; then
                echo "FAIL $flabel (producer exit $prc)"
                sed 's/^/    /' "$tmp/err.txt" >&2
                fails=$((fails + 1))
                rm -rf "$tmp" "$dir/out/int"
                continue
            fi
        fi

        if [ "$bless" -eq 1 ]; then
            cp "$tmp/out.txt" "$golden"
            echo "BLESS $flabel -> ${golden#"$here"/}"
            rm -rf "$tmp" "$dir/out/int"
            continue
        fi

        if [ ! -f "$golden" ]; then
            echo "FAIL $flabel (no golden ${golden#"$here"/}; run with --bless)"
            fails=$((fails + 1))
            rm -rf "$tmp" "$dir/out/int"
            continue
        fi

        if diff -u "$golden" "$tmp/out.txt" >"$tmp/diff.txt" 2>&1; then
            echo "PASS $flabel"
        else
            echo "FAIL $flabel (diff)"
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
