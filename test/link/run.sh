#!/usr/bin/env bash
# run.sh — the cross-compilation and linking suite.
#
# usage: run.sh [--leg <name>]... [--case <name>]... [--deps float|pin] [--bless] [--matrix]
#
# ONE QUESTION: does mach produce a correct IMAGE for a target, and link it against
# what the platform actually provides? that is the question a checksum cannot ask.
# the codegen corpus (test/) executes cases and compares them against a C anchor, so
# it settles what the compiler computes; nothing it does looks at an import table, a
# GOT slot, a load command, a section type, a PT_GNU_RELRO span or a foreign object.
# a case belongs here only when its fact is one of those - cross-compilation to a
# format this host cannot execute, or linking against something a foreign toolchain
# produced. anything checkable by building and looking on a developer's own machine
# is checked that way and never enters this directory.
#
# THE LEG REGISTRY IS test/engines.conf, shared with the corpus. a leg names the
# MACHINE a case runs on; `target:` in case.conf names the mach target it BUILDS.
# those are two axes because the whole point of a cross-compilation case is that
# they differ: a PE import table or a Mach-O bind table is cross-built on the cheap
# linux runner and read there host-side, with no runner of that format's own OS.
#
# NATIVE vs QEMU FIDELITY. the engine column decides how a case executes. qemu-user
# does not fully model kernel memory semantics: it does not fault an mprotect over
# address space the loader never mapped, and its guest page size need not match a
# real kernel's. so RELRO / mprotect runtime behaviour is proven only by a native
# leg. a qemu leg can pass while the same binary crashes on hardware (#1885, where a
# 64K-aligned aarch64 image faulted ENOMEM on a native 4K-page kernel every
# qemu-aarch64 leg had reported green). the aarch64-linux row is native for that
# reason. there is no flag to override an engine: a run that says it exercised a leg
# has to have exercised it.
#
# DEPENDENCY RESOLUTION. a case declares `ref = "branch/main"` for mach-std, which
# tracks its latest RELEASE, and `--deps float` resolves it fresh - that is what
# catches a downstream break on the pull request that would first trip over it.
# `--deps pin` states the other intent, resolving every case to the commit THIS
# REPO'S OWN mach.lock records, which is the one the compiler in the same checkout
# was built against, so a release-gate run is reproducible and a bisect over mach
# commits is sound (#2592). float resolves ONCE PER RUN, not once per case (#2619):
# before that, a suite spanning a mach-std merge compiled some cases against one
# standard library and some against another and still reported a single verdict.
#
# NOT SAFE TO RUN CONCURRENTLY AGAINST ONE CHECKOUT. a case builds into its own
# directory, because the output path is a manifest key rather than something the
# command line can redirect, so two invocations race on the same per-case out/ and
# can produce a wrong result including a spurious PASS. a second checkout is the
# answer; a lock here would only serialize what is already serial.
set -eu

here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo=$(CDPATH= cd -- "$here/../.." && pwd)
. "$here/lib/case.sh"
. "$here/lib/produce.sh"
. "$here/lib/deps.sh"

usage() {
    echo "usage: run.sh [--leg <name>]... [--case <name>]... [--deps float|pin] [--bless] [--matrix]" >&2
    exit 2
}

want_legs=
want_cases=
bless=0
matrix_only=0
deps_mode=float

while [ $# -gt 0 ]; do
    case "$1" in
        --leg)   shift; [ $# -gt 0 ] || usage; want_legs="$want_legs $1" ;;
        --case)  shift; [ $# -gt 0 ] || usage; want_cases="$want_cases $1" ;;
        --deps)  shift; [ $# -gt 0 ] || usage; deps_mode=$1
                 case "$deps_mode" in float|pin) : ;; *) echo "run.sh: --deps must be float or pin, got '$deps_mode'" >&2; usage ;; esac ;;
        --bless) bless=1 ;;
        --matrix) matrix_only=1 ;;
        -h|--help) usage ;;
        *) echo "run.sh: unexpected argument '$1'" >&2; usage ;;
    esac
    shift
done

out=${MACH_LINK_OUT:-$here/out}
mkdir -p "$out"
matrix=$out/matrix.tsv

if [ "$matrix_only" -eq 1 ]; then
    if [ ! -f "$matrix" ]; then
        echo "run.sh: no matrix at $matrix; run the suite first" >&2
        exit 2
    fi
    cat "$matrix"
    exit 0
fi

# blessing rewrites the recorded answers, so it is a local act with a human reading
# the diff. a CI run that could bless would record whatever it produced as intended.
if [ "$bless" -eq 1 ] && [ -n "${CI:-}" ]; then
    echo "run.sh: --bless refuses to run under CI: a golden nobody read is not a golden" >&2
    exit 2
fi

# the compiler under test. never a `mach` on PATH, which is usually an older
# release, and the path and version are printed so a result can never be attributed
# to a binary nobody named.
host_dir() {
    case "$(uname -s)/$(uname -m)" in
        Linux/x86_64)   echo linux-x86_64 ;;
        Linux/aarch64)  echo linux-aarch64 ;;
        Darwin/x86_64)  echo darwin-x86_64 ;;
        Darwin/arm64)   echo darwin-aarch64 ;;
        MINGW*|MSYS*|CYGWIN*) echo windows-x86_64 ;;
        *) return 1 ;;
    esac
}

compiler=${MACH_LINK_MACH:-}
if [ -z "$compiler" ]; then
    hd=$(host_dir) || {
        echo "run.sh: no default compiler path for host $(uname -s)/$(uname -m); set MACH_LINK_MACH" >&2
        exit 2
    }
    compiler=$repo/out/$hd/debug/bin/mach
    [ -f "$compiler" ] || compiler=$compiler.exe
fi
case "$compiler" in
    /*) : ;;
    *)  compiler=$(CDPATH= cd -- "$(dirname -- "$compiler")" && pwd)/$(basename -- "$compiler") ;;
esac
if [ ! -f "$compiler" ]; then
    echo "run.sh: the compiler under test is not at '$compiler'. build it, or set MACH_LINK_MACH. this suite never falls back to a mach on PATH." >&2
    exit 2
fi

# the executable suffix is a property of the host that runs the artifact: a native
# leg executes on the runner itself (windows -> .exe), a qemu leg loads a guest ELF.
exe=
case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*) exe=.exe ;;
esac

echo "compiler: $compiler"
"$compiler" info | sed -n '1p' | sed 's/^/compiler: /'

# the legs this run covers. named explicitly, or every leg this host can serve: a
# native leg needs the host's own os and isa, a qemu leg needs its interpreter
# installed. the same rule the corpus driver applies, so the two agree about what a
# machine can be asked for.
host_os=$(uname -s | tr 'A-Z' 'a-z')
case "$host_os" in mingw*|msys*|cygwin*) host_os=windows ;; esac
host_isa=$(uname -m)
case "$host_isa" in arm64) host_isa=aarch64 ;; amd64) host_isa=x86_64 ;; esac

serves_leg() {
    _sl_engine=$(leg_engine "$1")
    case "$_sl_engine" in
        qemu:*) command -v "${_sl_engine#qemu:}" >/dev/null 2>&1 && return 0
                echo "not selected: $1 (engine ${_sl_engine#qemu:} is not installed on this host)"
                return 1 ;;
    esac
    _sl_os=$(engines_field "$1" 3)
    _sl_isa=$(engines_field "$1" 2)
    if [ "$_sl_os" = "$host_os" ] && [ "$_sl_isa" = "$host_isa" ]; then return 0; fi
    echo "not selected: $1 (engine native needs a $_sl_os/$_sl_isa runner, this host is $host_os/$host_isa)"
    return 1
}

legs=
if [ -n "$want_legs" ]; then
    for leg in $want_legs; do
        if ! engines_field "$leg" 1 >/dev/null; then
            echo "run.sh: no such leg in test/engines.conf: $leg" >&2
            exit 2
        fi
        legs="$legs $leg"
    done
else
    for leg in $(all_legs); do
        if serves_leg "$leg"; then legs="$legs $leg"; fi
    done
fi
if [ -z "$legs" ]; then
    echo "run.sh: this host serves no leg in test/engines.conf" >&2
    exit 2
fi
echo "legs:$legs"

# every case a run will build must declare the [target.<t>] block it names, and a
# pinned run must be able to pin every git dep any case declares. both are static,
# both fail the whole run rather than one case, and both used to be separate scripts
# a workflow could stop calling - #2353 and #2729 are each a year of a lane that was
# believed to be running and had never once started. checked here so nothing can
# reach the build without them holding.
preflight_fails=0
preflight_checked=0
for dir in "$here"/cases/*/; do
    dir=${dir%/}
    [ -f "$dir/mach.toml" ] || continue
    read_case_conf "$dir"
    for leg in $case_allow; do
        in_list "$leg" "$case_skip" && continue
        t=${case_target:-$leg}
        preflight_checked=$((preflight_checked + 1))
        if ! grep -q "^\[target\.$t\]" "$dir/mach.toml"; then
            echo "run.sh: $(basename "$dir"): leg '$leg' builds target '$t', which its mach.toml does not declare" >&2
            preflight_fails=$((preflight_fails + 1))
        fi
    done
done
if [ "$preflight_checked" -eq 0 ]; then
    echo "run.sh: no case x leg pair was checked, which is a broken suite rather than a clean one" >&2
    exit 2
fi
if [ "$deps_mode" = pin ]; then
    while read -r c d u; do
        [ -n "$c" ] || continue
        echo "run.sh: $c declares git dep '$d' ($u), which the repo's own mach.lock does not record, so a pinned run cannot resolve it" >&2
        preflight_fails=$((preflight_fails + 1))
    done <<EOF
$(unpinnable_deps)
EOF
fi
[ "$preflight_fails" -eq 0 ] || exit 2
echo "preflight: $preflight_checked case x leg pair(s) declare their target block"

# the run's own float resolution, one line per git dep: "<name> <url> <ref> <commit>".
# written by record_run_deps once the first case resolves, read by prepare_case_deps
# for every case after it. truncated per run, so a run never inherits the previous
# run's resolution - float still tracks the tip, it just does so once (#2619).
run_deps=$out/run-deps.txt
: >"$run_deps"

# what every case actually compiled against, as a file rather than only as a log
# line: "what did the run that cut this tag build against" is asked after the fact,
# often once the log has rotated (#2592). CI uploads it.
manifest=$out/deps.txt

run_dep_commit() {
    [ -f "$run_deps" ] || return 0
    awk -v n="$1" '$1 == n { print $4; exit }' "$run_deps"
}

# case_lock_from <case-dir> <resolver> — write the case's mach.lock, taking each git
# dep's commit from `<resolver> <name>`. returns 1 when the resolver has no commit
# for some git dep, naming it in `missing_dep` and leaving no lock behind, so a
# caller can either fail or fall back to resolving fresh. a case declaring no git dep
# correctly ends with no lock at all.
case_lock_from() {
    dir=$1
    resolver=$2
    missing_dep=
    tmplock=$dir/mach.lock.new
    echo "# written by test/link/run.sh; not tracked." >"$tmplock"
    written=0
    for name in $(case_deps "$dir"); do
        url=$(dep_field "$dir/mach.toml" "$name" git)
        if [ -z "$url" ]; then continue; fi
        commit=$("$resolver" "$name")
        if [ -z "$commit" ]; then
            missing_dep=$name
            rm -f "$tmplock"
            return 1
        fi
        printf '\n[dep.%s]\nurl = "%s"\nref = "%s"\ncommit = "%s"\n' \
            "$name" "$url" "$(dep_field "$dir/mach.toml" "$name" ref)" "$commit" >>"$tmplock"
        written=$((written + 1))
    done
    if [ "$written" -eq 0 ]; then
        rm -f "$tmplock" "$dir/mach.lock"
        return 0
    fi
    mv "$tmplock" "$dir/mach.lock"
}

# prepare_case_deps <case-dir> — settle what the case's `dep pull` will resolve.
#
# float removes any lock first, so the ref genuinely resolves fresh. that is not a
# no-op: `dep pull` HONORS a lock that is present even for a `branch/` ref, so a tree
# that had ever run pinned would keep resolving that pin silently.
prepare_case_deps() {
    dir=$1
    if [ "$deps_mode" = float ]; then
        if ! case_lock_from "$dir" run_dep_commit; then
            rm -f "$dir/mach.lock"
        fi
        return 0
    fi
    case_lock_from "$dir" pin_dep_commit
}

# record_run_deps <case-dir> — remember what this case's `dep pull` resolved, so
# every later case in the run is handed the same commit. reads the lock `dep pull`
# just wrote rather than the checked-out tree, so what is recorded is exactly what
# the compiler decided the ref meant. first writer wins.
record_run_deps() {
    dir=$1
    if [ "$deps_mode" != float ]; then return 0; fi
    if [ ! -f "$dir/mach.lock" ]; then return 0; fi
    for name in $(case_deps "$dir"); do
        url=$(dep_field "$dir/mach.toml" "$name" git)
        if [ -z "$url" ]; then continue; fi
        if [ -n "$(run_dep_commit "$name")" ]; then continue; fi
        commit=$(lock_commit "$dir/mach.lock" "$name")
        if [ -z "$commit" ]; then continue; fi
        printf '%s %s %s %s\n' \
            "$name" "$url" "$(dep_field "$dir/mach.toml" "$name" ref)" "$commit" >>"$run_deps"
    done
}

# dep_commits <case-dir> [full] — "name@sha" per git dep checked out under a case's
# dep/, ground-truthed from the CHECKOUT rather than from mach.lock: the lock is the
# RECORD of a resolution and the checkout is the entity that was compiled against,
# and #2387 was a case where those disagreed. a disagreement prints
# "name@<checkout>!lock=<locked>" rather than silently trusting either one.
dep_commits() {
    dir=$1
    fmt=${2:-short}
    [ -d "$dir/dep" ] || return 0
    for depdir in "$dir"/dep/*/; do
        [ -d "${depdir}.git" ] || [ -f "${depdir}.git" ] || continue
        name=$(basename "$depdir")
        full=$(git -C "$depdir" rev-parse HEAD 2>/dev/null) || continue
        short=$(git -C "$depdir" rev-parse --short HEAD 2>/dev/null) || continue
        if [ "$fmt" = full ]; then head=$full; else head=$short; fi
        locked=$(lock_commit "$dir/mach.lock" "$name")
        if [ -n "$locked" ] && [ "$locked" != "$full" ]; then
            printf '%s@%s!lock=%s ' "$name" "$head" "$(printf '%s' "$locked" | cut -c1-7)"
        else
            printf '%s@%s ' "$name" "$head"
        fi
    done
}

# name the resolution out loud. a run that pinned and a run that floated differ in
# nothing a reader can see afterwards except this, and #2729 is the case where the
# pinned lane was believed to be running when it had never once started.
deps_banner() {
    if [ "$deps_mode" = float ]; then
        echo "deps: float - every ref resolved fresh by 'mach dep pull', no lock consulted"
        return 0
    fi
    echo "deps: pin - commits taken from the repo's own mach.lock"
    for name in $(dep_names "$root_lock"); do
        echo "deps:   mach.lock $name@$(lock_commit "$root_lock" "$name")"
    done
}

deps_banner
{
    echo "# link suite dependency resolution"
    echo "# legs:$legs"
    echo "# deps-mode: $deps_mode"
    deps_banner | sed 's/^deps:/#/'
    echo "# columns: case profile dep@commit..."
} >"$manifest"

# the coverage matrix: one row per cell the run actually reached, naming the engine
# that produced it so a qemu result can never be read as native ABI evidence. it is
# written as the run goes, so a run that dies partway still leaves what it covered.
printf 'case\tleg\tprofile\ttarget\tengine\trun\tresult\n' >"$matrix"

fails=0
ran=0

for dir in "$here"/cases/*/; do
    dir=${dir%/}
    [ -f "$dir/mach.toml" ] || continue
    case_id=$(basename "$dir")

    if [ -n "$want_cases" ] && ! in_list "$case_id" "$want_cases"; then continue; fi

    read_case_conf "$dir"

    for leg in $legs; do
        in_list "$leg" "$case_allow" || continue
        if in_list "$leg" "$case_skip"; then
            # named in the accounting rather than vanishing: a run a leg was never
            # applicable to and a run it was quietly dropped from used to look
            # identical (#2741). the reason lives in case.conf's comment.
            echo "SKIP $case_id [$leg] (skip, see case.conf)"
            printf '%s\t%s\t-\t-\t-\t%s\tSKIP\n' "$case_id" "$leg" "$case_run" >>"$matrix"
            continue
        fi

        engine=$(leg_engine "$leg")
        build_target=${case_target:-$leg}

        # once per case-leg, not per profile: `dep pull` honors the lock the first
        # profile left, so deciding here is what makes a case's two profiles agree
        # on one commit.
        prepare_case_deps "$dir" || {
            echo "run.sh: $case_id: cannot resolve dependencies" >&2
            exit 2
        }

        for profile in $case_profiles; do
            ran=$((ran + 1))
            label="$case_id [$leg/$profile]"

            # the golden is shared across build-targets for target-independent
            # observables (exec, the relro-fault guard, and debuginfo / varloc-fbreg
            # / symtab, whose facts hold identically on every ELF ISA), per-profile
            # for gdb-session (its stop/frame/value facts are a real function of the
            # active profile's own codegen, #2779), and per-build-target for
            # structural producers, whose fact is format-specific.
            case "$case_run" in
                exec|relro-fault|debuginfo|varloc-fbreg|symtab) golden="$dir/expect.txt" ;;
                gdb-session) golden="$dir/expect.$profile.txt" ;;
                *)           golden="$dir/expect.$build_target.txt" ;;
            esac

            tmp=$(mktemp -d)
            relbin="out/link/prog$exe"
            bin="$dir/$relbin"
            rm -rf "$dir/out/link"
            mkdir -p "$dir/out/link"

            # drop the step stamp cache so every `[step]` re-runs for this build.
            # mach skips a step whose stamp matches and whose outputs all exist, and
            # neither depends on the compiler, so a stamp an earlier run wrote made
            # later runs skip the step engine entirely - a case asserting on step
            # output kept passing against a compiler that could no longer run a step
            # at all (#2578). the outputs are left alone: with no stamp the step
            # always runs, and a step that fails is a build failure.
            rm -rf "$dir/out/.steps"

            # normally the from-source compiler itself. a self-host case first
            # cross-builds that compiler to the leg's target and drives it under the
            # leg's engine, so the case is compiled by the target-hosted compiler -
            # the coverage running produced binaries can never reach.
            buildcc=$compiler
            if [ -n "$case_self_host" ]; then
                if ! (cd "$repo" && "$compiler" build . --target "$case_self_host" --profile "$profile" -o "$tmp/selfhostcc") >"$tmp/selfhost.log" 2>&1; then
                    echo "FAIL $label (self-host cross-build)"
                    sed 's/^/    /' "$tmp/selfhost.log" >&2
                    fails=$((fails + 1))
                    printf '%s\t%s\t%s\t%s\t%s\t%s\tFAIL\n' "$case_id" "$leg" "$profile" "$build_target" "$engine" "$case_run" >>"$matrix"
                    rm -rf "$tmp" "$dir/out/link"
                    continue
                fi
                case "$engine" in
                    qemu:*) buildcc="${engine#qemu:} $tmp/selfhostcc" ;;
                    *)      buildcc=$tmp/selfhostcc ;;
                esac
            fi

            if (cd "$dir" && "$compiler" dep pull && $buildcc build . --target "$build_target" --profile "$profile" $case_build_flags -o "$relbin") >"$tmp/build.log" 2>&1; then
                build_ok=1
            else
                build_ok=0
            fi

            # what this case resolved becomes the run's resolution for every case
            # after it, whether or not the build itself succeeded (#2619).
            record_run_deps "$dir"

            deps=$(dep_commits "$dir")
            flabel=$label
            [ -n "$deps" ] && flabel="$label (${deps% })"
            full_deps=$(dep_commits "$dir" full)
            [ -n "$full_deps" ] && echo "$case_id $profile ${full_deps% }" >>"$manifest"

            # a build-fails case asserts the compile is REJECTED and takes the
            # compiler's 'error:' diagnostic as its observable. every other mode
            # requires a clean build and runs a producer on the artifact.
            if [ "$case_run" = build-fails ]; then
                if [ "$build_ok" -eq 1 ]; then
                    echo "FAIL $flabel (build succeeded; expected a link error)"
                    fails=$((fails + 1))
                    printf '%s\t%s\t%s\t%s\t%s\t%s\tFAIL\n' "$case_id" "$leg" "$profile" "$build_target" "$engine" "$case_run" >>"$matrix"
                    rm -rf "$tmp" "$dir/out/link"
                    continue
                fi
                grep '^error:' "$tmp/build.log" >"$tmp/out.txt" || true
                if [ ! -s "$tmp/out.txt" ]; then
                    echo "FAIL $flabel (build failed without an 'error:' diagnostic)"
                    sed 's/^/    /' "$tmp/build.log" >&2
                    fails=$((fails + 1))
                    printf '%s\t%s\t%s\t%s\t%s\t%s\tFAIL\n' "$case_id" "$leg" "$profile" "$build_target" "$engine" "$case_run" >>"$matrix"
                    rm -rf "$tmp" "$dir/out/link"
                    continue
                fi
            else
                if [ "$build_ok" -eq 0 ]; then
                    echo "FAIL $flabel (build)"
                    sed 's/^/    /' "$tmp/build.log" >&2
                    fails=$((fails + 1))
                    printf '%s\t%s\t%s\t%s\t%s\t%s\tFAIL\n' "$case_id" "$leg" "$profile" "$build_target" "$engine" "$case_run" >>"$matrix"
                    rm -rf "$tmp" "$dir/out/link"
                    continue
                fi

                # a debug-info producer inspects the artifact built with and without
                # `-g`: the default build above is the no-`-g` one, and its `-g` twin
                # is built here with the same compiler, target, profile and flags.
                gbin=
                if [ "$case_run" = debuginfo ] || [ "$case_run" = varloc-fbreg ] || [ "$case_run" = gdb-session ]; then
                    gbin="$dir/out/link/prog-g$exe"
                    if ! (cd "$dir" && $buildcc build . --target "$build_target" --profile "$profile" $case_build_flags -g -o "out/link/prog-g$exe") >"$tmp/build-g.log" 2>&1; then
                        echo "FAIL $flabel (build -g)"
                        sed 's/^/    /' "$tmp/build-g.log" >&2
                        fails=$((fails + 1))
                        printf '%s\t%s\t%s\t%s\t%s\t%s\tFAIL\n' "$case_id" "$leg" "$profile" "$build_target" "$engine" "$case_run" >>"$matrix"
                        rm -rf "$tmp" "$dir/out/link"
                        continue
                    fi
                fi

                if produce "$case_run" "$engine" "$leg" "$bin" "$gbin" "$profile" >"$tmp/out.txt" 2>"$tmp/err.txt"; then
                    prc=0
                else
                    prc=$?
                fi
                if [ "$prc" -ne 0 ]; then
                    echo "FAIL $flabel (producer exit $prc)"
                    sed 's/^/    /' "$tmp/err.txt" >&2
                    fails=$((fails + 1))
                    printf '%s\t%s\t%s\t%s\t%s\t%s\tFAIL\n' "$case_id" "$leg" "$profile" "$build_target" "$engine" "$case_run" >>"$matrix"
                    rm -rf "$tmp" "$dir/out/link"
                    continue
                fi
            fi

            if [ "$bless" -eq 1 ]; then
                if [ -f "$golden" ]; then
                    diff -u "$golden" "$tmp/out.txt" | sed 's/^/    /' || true
                fi
                cp "$tmp/out.txt" "$golden"
                echo "BLESS $flabel -> ${golden#"$here"/}"
                printf '%s\t%s\t%s\t%s\t%s\t%s\tBLESS\n' "$case_id" "$leg" "$profile" "$build_target" "$engine" "$case_run" >>"$matrix"
                rm -rf "$tmp" "$dir/out/link"
                continue
            fi

            if [ ! -f "$golden" ]; then
                echo "FAIL $flabel (no golden ${golden#"$here"/}; run with --bless)"
                fails=$((fails + 1))
                printf '%s\t%s\t%s\t%s\t%s\t%s\tEMPTY\n' "$case_id" "$leg" "$profile" "$build_target" "$engine" "$case_run" >>"$matrix"
                rm -rf "$tmp" "$dir/out/link"
                continue
            fi

            if diff -u "$golden" "$tmp/out.txt" >"$tmp/diff.txt" 2>&1; then
                echo "PASS $flabel"
                printf '%s\t%s\t%s\t%s\t%s\t%s\tPASS\n' "$case_id" "$leg" "$profile" "$build_target" "$engine" "$case_run" >>"$matrix"
            else
                echo "FAIL $flabel (diff)"
                sed 's/^/    /' "$tmp/diff.txt" >&2
                fails=$((fails + 1))
                printf '%s\t%s\t%s\t%s\t%s\t%s\tFAIL\n' "$case_id" "$leg" "$profile" "$build_target" "$engine" "$case_run" >>"$matrix"
            fi
            rm -rf "$tmp" "$dir/out/link"
        done
    done
done

if [ "$ran" -eq 0 ]; then
    echo "run.sh: no case matched --case '$want_cases' on legs$legs" >&2
    exit 1
fi

echo "matrix: $matrix"
passes=$(awk -F'\t' '$7 == "PASS"' "$matrix" | wc -l | tr -d ' ')
skips=$(awk -F'\t' '$7 == "SKIP"' "$matrix" | wc -l | tr -d ' ')
echo "link: $passes pass / $fails fail / $skips skip over $ran cell(s)"

if [ "$fails" -ne 0 ]; then
    exit 1
fi
