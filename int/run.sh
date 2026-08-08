#!/usr/bin/env bash
# run.sh — the integration-test harness.
#
# usage: run.sh --target <name> [--runmode native|qemu] [--deps float|pin] [--bless] [--filter <glob>] <compiler>
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
#
# DEPENDENCY RESOLUTION. a case declares `ref = "branch/main"` for mach-std, which
# tracks its latest RELEASE, and the harness resolves it fresh: that is deliberate,
# and it is what catches a downstream break early. `--deps pin` states the other
# intent instead, resolving every case to a recorded commit - for mach-std the one
# THIS REPO'S OWN mach.lock records, which is the one the compiler in the same
# checkout was built against - so a release-gate run is reproducible and a bisect
# over mach commits is sound (#2592). mach-std pins to the root lock rather than to
# a copy precisely so it cannot rot against the compiler's: bumping the dependency
# bumps both at once. but the root lock is written from the ROOT manifest, so it can
# only ever cover the compiler's own dependency closure, and int builds a larger
# one - int/deps.lock covers the difference, today mach-shader. lib/deps.sh holds
# the full account and lib/check-deps.sh enforces it on every PR (#2729).
#
# float resolves ONCE PER RUN, not once per case (#2619). the first case to declare a
# dep resolves it fresh and every case after it is handed that same commit, so a run
# is one coherent experiment: before this, each case resolved at the moment its own
# build started, and a suite spanning a mach-std merge compiled some cases against one
# standard library and some against another while still reporting a single verdict.
# the harness never maps a ref to a commit itself - it reuses the lock `mach dep pull`
# wrote for the first case, so resolution stays the compiler's job and there is no
# second implementation to drift from it.
#
# both modes write int/out/deps.txt naming the commit every case actually compiled
# against. the PASS/FAIL lines have carried that since #2387, but a log line only
# answers the question while the log exists, and "what did the run that cut this tag
# build against" is asked afterwards. CI uploads the file.
#
# `--runmode qemu` only reaches a leg qemu-user can actually load: qemu_bin() in
# lib/produce.sh names the ELF-only rule and the three targets that can never
# work under it (#2453) - passing `qemu` for one of those fails loudly with that
# reason once the case's producer runs, not silently as an `Exec format error`.
set -eu

usage() {
    echo "usage: run.sh --target <name> [--runmode native|qemu] [--deps float|pin] [--bless] [--filter <glob>] <compiler>" >&2
    exit 2
}

target=
runmode_override=
bless=0
filter='*'
deps_mode=float
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
        --deps)    shift; [ $# -gt 0 ] || usage; deps_mode=$1
                   case "$deps_mode" in
                       float|pin) : ;;
                       *) echo "run.sh: --deps must be 'float' or 'pin', got '$deps_mode'" >&2; usage ;;
                   esac ;;
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
. "$here/lib/case.sh"
. "$here/lib/produce.sh"
. "$here/lib/deps.sh"

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

# dep_field, lock_commit, case_deps and the pin sources live in lib/deps.sh, shared
# with check-deps.sh and update-deps.sh so the three agree on what a lock covers.

# prepare_case_deps <case-dir> — settle what the case's `dep pull` will resolve.
#
# A case declares `ref = "branch/main"`, which tracks mach-std's latest RELEASE, so
# the harness has always resolved it fresh and int/.gitignore refuses a committed
# lock. That early warning is worth keeping, but on a RELEASE GATE it means a tag can
# be cut against a dependency nobody recorded, and two runs of the same mach commit
# that straddle a mach-std merge are not comparable - which cost real time in the
# v4.7.1 cut (#2592).
#
# So the resolution is now a stated mode rather than an accident:
#
#   float — remove any lock first, so the ref genuinely resolves fresh. this is not a
#           no-op: `dep pull` HONORS a lock that is present even for a `branch/` ref
#           (it is the same mechanism that pins this repo's own mach-std), so without
#           the removal a tree that had ever run pinned would keep resolving that pin
#           silently, which is the defect this issue names, reintroduced locally.
#
#   pin   — write the lock from the pin sources, so a case resolves the same commit
#           the compiler in the same checkout was built against. for mach-std that
#           is THIS REPO'S OWN mach.lock, deliberately rather than a copy: a copy
#           would be a second thing to bump and could rot against the compiler's.
#           int/deps.lock covers only what the root manifest does not declare, which
#           the root lock therefore cannot record at all - lib/deps.sh's header has
#           the full account, and #2729 is what that gap cost.

# run_dep_commit <name> — the commit THIS RUN has already resolved for <name>, if any.
run_dep_commit() {
    [ -f "$run_deps" ] || return 0
    awk -v n="$1" '$1 == n { print $4; exit }' "$run_deps"
}

# case_lock_from <case-dir> <resolver> — write the case's mach.lock, taking each git
# dep's commit from `<resolver> <name>`.
#
# Returns 1 when the resolver has no commit for some git dep, naming it in
# `missing_dep` and leaving no lock behind, so a caller can either fail (pin) or fall
# back to resolving fresh (float). A case declaring no git dep correctly ends with no
# lock at all.
case_lock_from() {
    dir=$1
    resolver=$2
    missing_dep=
    tmplock=$dir/mach.lock.new
    echo "# written by int/run.sh; not tracked." >"$tmplock"
    written=0
    for name in $(case_deps "$dir"); do
        # only a git dep floats. a `path =` dep (surface/pe-import-claim-cascade's
        # `prov`) is a directory inside this repo, so it is already exactly as
        # reproducible as the checkout and has no commit to record.
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

prepare_case_deps() {
    dir=$1
    if [ "$deps_mode" = float ]; then
        # ONE RESOLUTION PER RUN, not one per case (#2619). The first case to declare
        # a dep resolves it fresh, which is float's whole point; every case after it
        # is handed that same commit. Before this, each case resolved independently at
        # the moment its own build started, so a suite run spanning a mach-std merge
        # compiled some cases against one standard library and some against another
        # and still reported one verdict.
        #
        # The harness never maps a ref to a commit itself - it only reuses what `mach
        # dep pull` wrote for the first case. Resolution stays the compiler's job, so
        # there is no second implementation to drift.
        if ! case_lock_from "$dir" run_dep_commit; then
            rm -f "$dir/mach.lock"
        fi
        return 0
    fi
    if ! case_lock_from "$dir" pin_dep_commit; then
        echo "run.sh: --deps pin: neither mach.lock nor int/deps.lock records a commit for git dep '$missing_dep', declared by ${dir#"$here"/}" >&2
        echo "run.sh: run 'bash int/lib/check-deps.sh' to see this without building, and update-deps.sh to fix it" >&2
        return 1
    fi
}

# record_run_deps <case-dir> — remember what this case's `dep pull` resolved, so every
# later case in the run is given the same commit.
#
# Reads the lock `dep pull` just wrote rather than the checked-out tree, so what is
# recorded is exactly what the compiler decided the ref meant. First writer wins: once
# a name is recorded the run holds that commit even if the remote moves mid-run, which
# is the whole point.
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
# state. Printing the checkout into every PASS/FAIL/BLESS line is one record.
#
# That line was once argued to survive "as long as the CI log does, which is
# exactly as long as anyone would want to ask". #2592 is the counterexample: the
# question was asked during the v4.7.1 cut, about a run whose log had to be dug
# out of scrollback, and answering it wrong sent the investigation at the release
# instead of at a mach-std merge. So the same fact is now also written to
# int/out/deps.txt, which CI uploads as an artifact of the run itself, and a
# release-gate run pins rather than floats (--deps pin, prepare_case_deps above).
#
# <fmt> is `short` for the PASS/FAIL label a human reads, or `full` for that
# manifest, whose whole purpose is to be usable to reproduce the resolution later.
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

runmode=$(conf_runmode "$target") || {
    echo "run.sh: target '$target' is not in targets.conf" >&2
    exit 2
}
if [ -n "$runmode_override" ] && [ "$runmode_override" != "$runmode" ]; then
    echo "run.sh: --runmode overrides '$target' to $runmode_override (targets.conf says $runmode; not CI-equivalent, see NATIVE vs QEMU FIDELITY above)" >&2
    runmode=$runmode_override
fi

# the run's dependency record. every PASS/FAIL line already names the resolved commit
# (#2387), but a log line is recoverable only from scrollback, and the question this
# answers - "what did the run that cut this tag actually compile against" - is asked
# after the fact, often once the log has rotated. this is a file CI can upload beside
# the rest of the run's artifacts (#2592). it lives under the gitignored int/out/.
manifest=$here/out/deps.txt
mkdir -p "$here/out"

# the run's own float resolution, one line per git dep: "<name> <url> <ref> <commit>".
# written by record_run_deps once the first case resolves, read by prepare_case_deps
# for every case after it. truncated per run, so a run never inherits the previous
# run's resolution - float still tracks the tip, it just does so once (#2619).
run_deps=$here/out/run-deps.txt
: >"$run_deps"

# name the resolution out loud, to stdout and into the manifest. a run that pinned
# and a run that floated differ in nothing a reader can see afterwards except this,
# and #2729 is the case where the pinned lane was believed to be running when it had
# never once started. under pin, every entry of both locks is listed: which file
# supplied which commit is the fact that makes a pinned run reproducible later.
deps_banner() {
    if [ "$deps_mode" = float ]; then
        echo "deps: float - every ref resolved fresh by 'mach dep pull', no lock consulted"
        return 0
    fi
    echo "deps: pin - commits taken from mach.lock, then int/deps.lock"
    for lockpath in "$root_lock" "$int_lock"; do
        label=mach.lock
        [ "$lockpath" = "$int_lock" ] && label=int/deps.lock
        if [ ! -f "$lockpath" ]; then
            echo "deps:   $label (absent)"
            continue
        fi
        for name in $(dep_names "$lockpath"); do
            echo "deps:   $label $name@$(lock_commit "$lockpath" "$name")"
        done
    done
}

deps_banner
{
    echo "# int dependency resolution"
    echo "# target: $target"
    echo "# deps-mode: $deps_mode"
    deps_banner | sed 's/^deps:/#/'
    echo "# columns: case profile dep@commit..."
} >"$manifest"

fails=0
ran=0

# iterate cases in a stable order across the two buckets.
for dir in "$here"/surface/$filter "$here"/regression/$filter; do
    [ -f "$dir/mach.toml" ] || continue
    case_id=${dir#"$here"/}

    # case defaults, overridden by case.conf if present, and the resolved
    # `targets: all`-or-explicit allowlist - shared with check-target-matrix.sh
    # (#2353) so the two never disagree about what a case.conf line means.
    read_case_conf "$dir"
    if ! in_list "$target" "$case_allow"; then continue; fi
    if in_list "$target" "$case_exempt"; then
        # named in the run accounting rather than vanishing silently (mach#2741):
        # an exempted leg used to leave no trace at all, so a run this leg was
        # never applicable to and a run it was quietly dropped from looked
        # identical. the reason itself lives only in case.conf's comment, same as
        # exempt always has - this just states that a reason exists and where.
        echo "SKIP $case_id [$target] (exempt, see case.conf)"
        continue
    fi

    # the mach target to compile: the build-target if set, else the leg itself.
    build_target=${case_build_target:-$target}

    # the golden is shared across build-targets for target-independent observables (exec,
    # the relro-fault guard, and the panic-exit guard, whose outputs are all
    # target-independent; debuginfo, whose two facts — validator-clean and additive —
    # hold identically on every ELF ISA) and per-build-target for structural producers
    # (their fact is format-specific).
    case "$case_run" in
        exec|relro-fault|panic-exit|debuginfo|varloc-fbreg) golden="$dir/expect.txt" ;;
        *)                                                 golden="$dir/expect.$build_target.txt" ;;
    esac

    # once per case, not per profile: `dep pull` honors the lock left by the first
    # profile, so deciding here is both what makes a case's two profiles agree on one
    # commit and what keeps the resolution cost at one per case, as it already was.
    prepare_case_deps "$dir" || exit 2

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

        # drop the step stamp cache so every `[step]` re-runs for this build.
        # mach skips a step whose stamp matches and whose outputs all exist, and
        # neither depends on the compiler, so a stamp written by an EARLIER
        # harness run made later runs skip the step engine entirely - a case that
        # asserts on step output kept passing against a compiler that could no
        # longer run a step at all, which is the exact defect #2578 is. CI never
        # saw it (fresh checkout, no stamps); only local iteration did, in the
        # direction that reports green. the outputs are left alone: with no stamp
        # the step always runs, and a step that fails is a build failure.
        rm -rf "$dir/out/.steps"

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

        # what this case resolved becomes the run's resolution for every case after
        # it. done after `dep pull` and regardless of whether the build itself
        # succeeded, since a failed build still resolved its dependencies (#2619).
        record_run_deps "$dir"

        # a label suffix naming what mach-std commit `dep pull` resolved (#2387),
        # once one exists to report; every PASS/FAIL line from here on carries it.
        deps=$(dep_commits "$dir")
        flabel=$label
        [ -n "$deps" ] && flabel="$label (${deps% })"

        full_deps=$(dep_commits "$dir" full)
        if [ -n "$full_deps" ]; then
            echo "$case_id $profile ${full_deps% }" >>"$manifest"
        fi

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

            # a debug-info producer inspects the artifact built with and without `-g`:
            # the default build above is the no-`-g` one; build the `-g` twin here (same
            # compiler / target / profile / flags, plus `-g`) and hand its path to the
            # producer as an extra argument. every other producer inspects only `$bin`.
            gbin=
            if [ "$case_run" = debuginfo ] || [ "$case_run" = varloc-fbreg ]; then
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
