#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_exec <engine> <leg> <binary>
# runs the built binary and forwards its stdout as the observable. native mode runs
# it directly; qemu mode runs it under the matching qemu-user (qemu_bin). the
# producer's exit status is the program's, so a crash (nonzero) fails the case.
#
# on failure run.sh discards this producer's stdout and shows only its stderr, so a
# failing run reports the status and the program's own stdout there instead - without
# it a crashed exec case says only "producer exit 139" and throws away how far the
# program got (#2593).
produce_exec() {
    engine=$1
    target=$2
    bin=$3
    out=$(mktemp)
    err=$(mktemp)
    run_captured "$engine" "$target" "$bin" "$out" "$err" || { rm -f "$out" "$err"; return 1; }
    if [ "$run_status" -ne 0 ]; then
        report_run_failure "exec" "$run_status" "$run_out"
        [ -s "$err" ] && sed 's/^/    /' "$err" >&2
        rm -f "$out" "$err"
        return "$run_status"
    fi
    cat "$err" >&2
    cat "$out"
    rm -f "$out" "$err"
}

# produce_embed_dedup <engine> <leg> <binary>
# Count each embedded asset's byte sequence in the FINAL IMAGE, then run the
# program.
#
# The observable has to be the emitted bytes. Three modules embed byte-identical
# content, and #2518 already made their addresses compare equal WITHIN a module,
# so an in-program address comparison cannot see the defect #2541 is about: the
# object boundary lost the embed marker and the linker concatenated the same bytes
# once per module, leaving copies nobody references. Counting occurrences in the
# file is what distinguishes one placement from several.
#
# The fourth asset differs in its last byte only, so it also pins the other half of
# the contract: content that is not identical must stay distinct, and a scan that
# merged on length or section alone would report it missing.
produce_embed_dedup() {
    engine=$1
    target=$2
    bin=$3

    shared=9E2D41770BC35AE13684F21C60ABD94E73
    other=9E2D41770BC35AE13684F21C60ABD94E74

    shared_hits=$(count_byte_sequence "$bin" "$shared") || return 2
    other_hits=$(count_byte_sequence "$bin" "$other") || return 2

    [ "$shared_hits" -eq 1 ] || {
        echo "link: embed-dedup: the shared 17-byte asset occurs $shared_hits times, expected one placement for all three modules" >&2
        return 1
    }
    [ "$other_hits" -eq 1 ] || {
        echo "link: embed-dedup: the differing 17-byte asset occurs $other_hits times, expected exactly its own placement" >&2
        return 1
    }

    produce_exec "$engine" "$target" "$bin"
    echo "shared_placements=$shared_hits"
    echo "distinct_placements=$other_hits"
}

produce_embed_dedup "$@"
