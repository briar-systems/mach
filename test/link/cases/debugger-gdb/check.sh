#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_gdb_session <engine> <leg> <nog_binary> <g_binary> <profile>
# the BEHAVIOURAL debugger observable (#2756): test/link/cases/debuginfo proves the `-g`
# image is structurally valid; it cannot prove gdb reports the right thing when a
# user actually breaks into it. this producer drives one real `gdb --batch` session
# over the `-g` artifact and normalizes its transcript into stop/frame/value facts -
# every one of them read off the fixture by hand before it went into the golden (see
# test/link/cases/debugger-gdb/src/main.mach and case.conf for the arithmetic).
#
# a missing gdb is a hard error, the same contract produce_debuginfo already uses for
# its own validators (llvm-dwarfdump, addr2line, ...): every runner this case's
# case.conf names (`linux` only - see that file for why the others are not) is
# expected to carry one, so a silent skip would hide a real coverage gap instead of
# reporting it.
#
# gdb's own text is not the observable: it carries a pid in the exit line, a
# `Breakpoint N` or `Breakpoint N.M` counter that depends on how many candidate
# addresses gdb resolved for a source line (an inlined body and its dead out-of-line
# twin both claim the same line, so this varies between profiles for reasons that
# have nothing to do with correctness), and - a real gap this case does NOT assert
# on - the caller frame's OWN argument list, which this compiler currently populates
# from unrelated inlined locals rather than `main`'s real parameters (found while
# building this case; tracked separately, not asserted here because it is not what
# #2756 asks this case to prove). the gdb script below prints a `SENTINEL:<name>`
# echo ahead of each fact so the normalizer below can key off it instead of gdb's own
# formatting, and extracts only the function name and source line from a frame
# line, never its argument list.
#
# THE #2779 PROBES ARE DEBUG-ONLY. Both bugs #2779 fixed are specific to opt0's own
# location and line-table construction and do not reproduce at opt2 (verified by
# hand: `dies`/`staysalive` auto-inline at release exactly like `addone` does, and a
# breakpoint's `next` there steps clean out to the caller frame - a same-frame
# before/after `v` comparison is meaningless once that happens, the same reason
# `addone` itself never carried this probe). So the golden this run compares against
# is now PER-PROFILE (`expect.$profile.txt`, see run.sh), and only a debug-profile
# session sets the extra breakpoints / walks the extra steps below - a release
# session's transcript, and its golden, are exactly what they were before #2779.
produce_gdb_session() {
    g=$4
    profile=${5:-}
    command -v gdb >/dev/null 2>&1 || {
        echo "link: gdb-session: gdb not found (install the 'gdb' package)" >&2; return 2
    }

    gdbtmp=$(mktemp -d)
    script=$gdbtmp/session.gdb
    {
        cat <<'GDBEOF'
set debuginfod enabled off
set pagination off
break main.mach:11
break main.mach:20
break main.mach:27
GDBEOF
        # line 22 is accumulate's own `ret total;` (Bug B); lines 39 / 50 are
        # `dies` / `staysalive`'s `val r: i64 = v + 1;` (Bug A pair). set before
        # `run` like every other breakpoint so their numbering (4/5/6) is stable
        # regardless of when execution first reaches them.
        if [ "$profile" = debug ]; then
            cat <<'GDBEOF'
break main.mach:22
break main.mach:39
break main.mach:50
GDBEOF
        fi
        cat <<'GDBEOF'
run
echo SENTINEL:addone_stop\n
frame 0
echo SENTINEL:addone_v\n
print v
echo SENTINEL:addone_caller\n
frame 1
continue
continue
continue
continue
continue
echo SENTINEL:accum_total\n
print total
echo SENTINEL:accum_i\n
print i
echo SENTINEL:step1\n
step
echo SENTINEL:step2\n
step
delete 2
continue
GDBEOF
        # Bug B: this `continue` reaches accumulate's OWN ret statement before
        # deadlocal's breakpoint, iff line 22 resolves to its real address rather
        # than accumulate's entry (the bug: a duplicate row AT the entry, which
        # this call already passed long before reaching this line, so a broken
        # build never stops here at all and falls through straight to deadlocal -
        # a wrong `accum_ret_stop_func` is exactly as loud a failure as a wrong
        # `accum_ret_total`).
        if [ "$profile" = debug ]; then
            cat <<'GDBEOF'
echo SENTINEL:accum_ret_stop\n
frame 0
echo SENTINEL:accum_ret_total\n
print total
continue
GDBEOF
        fi
        cat <<'GDBEOF'
echo SENTINEL:dead_stop\n
frame 0
echo SENTINEL:dead_v\n
print v
echo SENTINEL:dead_unused\n
print unused
echo SENTINEL:dead_caller\n
frame 1
delete 3
continue
GDBEOF
        # `delete 3` above (line 27, deadlocal's `ret`) before the `continue` that
        # runs past it: at release, adding `dies` / `staysalive` to this file gives
        # the optimizer more to fold, and it can unify a fragment of one of their
        # tail sequences with deadlocal's own - gdb then resolves breakpoint 3 to a
        # THIRD location inside `main`'s inlined call to `dies`, which stops the
        # session there instead of letting the program finish (found by hand while
        # adding these probes: the release transcript went from "exited normally"
        # to no exit line and empty stdout at all, with no other change). deadlocal
        # is called exactly once, so this deletion loses no coverage - the same
        # reasoning `delete 2` already applies to the loop breakpoint above.
        #
        # Bug A pair: `next` past `val r = v + 1;` and re-read `v`. `dies` must
        # report it unavailable once `r`'s storage takes over; `staysalive` - v
        # read again on its own `ret` line - must keep reading the real value.
        # Asserted together on purpose (see main.mach): a fix that reported
        # `<optimized out>` for BOTH would pass either probe run alone.
        if [ "$profile" = debug ]; then
            cat <<'GDBEOF'
echo SENTINEL:dies_v_before\n
print v
next
echo SENTINEL:dies_v_after\n
print v
continue
echo SENTINEL:stays_v_before\n
print v
next
echo SENTINEL:stays_v_after\n
print v
continue
GDBEOF
        fi
    } >"$script"

    transcript=$gdbtmp/transcript.txt
    gdb --batch -q -x "$script" "$g" >"$transcript" 2>&1

    # the program's own stdout is ground truth for the values gdb is asked to read
    # back: a=addone's result, b=accumulate's, c=deadlocal's, d=dies's, e=staysalive's
    # (main.mach calls all five at both profiles, so this line needs no profile
    # branch even though only debug walks the gdb-side d/e probes below). cross-
    # checking it against the golden's hand-computed values is what would catch a
    # normalizer bug that made every gdb-side assertion vacuously agree with itself.
    stdout=$(grep -E '^[a-e]=[0-9]+$' "$transcript" | paste -sd, -)
    echo "program_stdout=$stdout"
    if grep -q 'exited normally' "$transcript"; then
        echo "exit=normal"
    else
        echo "exit=abnormal"
    fi

    # state machine over the transcript: a `SENTINEL:<name>` line names the fact the
    # NEXT matching line carries. frame lines (`_stop`/`_caller`) yield two facts,
    # function and line, deliberately excluding the argument list (see header). a
    # `print` result is the text after `$N = `. a `step` target is the source line
    # number gdb echoes ahead of the line's own text.
    cur=
    while IFS= read -r line; do
        case "$line" in
            SENTINEL:*) cur=${line#SENTINEL:}; continue ;;
        esac
        [ -n "$cur" ] || continue
        case "$cur" in
            *_stop|*_caller)
                m=$(printf '%s\n' "$line" | sed -E -n 's/^#[01]  (0x[0-9a-f]+ in )?([A-Za-z_][A-Za-z0-9_]*) \(.*\) at .*:([0-9]+)$/\2 \3/p')
                if [ -n "$m" ]; then
                    echo "${cur}_func=${m% *}"
                    echo "${cur}_line=${m#* }"
                    cur=
                fi
                ;;
            step1|step2)
                m=$(printf '%s\n' "$line" | sed -E -n 's/^([0-9]+)\t.*/\1/p')
                if [ -n "$m" ]; then
                    echo "${cur}_line=$m"
                    cur=
                fi
                ;;
            *)
                m=$(printf '%s\n' "$line" | sed -E -n 's/^\$[0-9]+ = (.*)$/\1/p')
                if [ -n "$m" ]; then
                    echo "${cur}=$m"
                    cur=
                fi
                ;;
        esac
    done <"$transcript"

    rm -rf "$gdbtmp"
}

produce_gdb_session "$@"
