#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_relro_fault <engine> <leg> <binary>
# runs the built binary (expected to write to a relocated constant's RELRO'd .rodata
# storage) and reports whether that write faulted. after the --pie startup mprotects
# the region read-only, the write must raise SIGSEGV, which surfaces as exit 128+11=139
# both natively and under qemu-user; any other status means the region stayed writable.
# the program's own stdout is discarded - the observable is purely the fault fact - so
# this is a runtime (exec-like) producer sharing one target-independent golden.
#
# on the expected fault the program's output is noise and stays out of the way. on any
# other status it is evidence - how far the program got before the write that should
# have faulted did not - so it is reported to stderr, leaving the observable the golden
# pins byte-identical (#2593).
produce_relro_fault() {
    engine=$1
    target=$2
    bin=$3
    out=$(mktemp)
    run_captured "$engine" "$target" "$bin" "$out" || { rm -f "$out"; return 1; }
    ec=$run_status
    rm -f "$out"
    if [ "$ec" -eq 139 ]; then
        echo "relro_write=faulted"
    else
        echo "relro_write=exit$ec"
        report_run_failure "relro-fault: expected the RELRO write to fault, but the program" \
            "$ec" "$run_out"
    fi
}

produce_relro_fault "$@"
