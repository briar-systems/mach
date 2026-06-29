# produce.sh — observable producers, sourced by run.sh.
#
# a producer turns a built case artifact into a normalized text observable on
# stdout, which run.sh diffs against the golden. the producer is the only thing
# that varies between verification modes; the golden-diff core does not change.
#
# v1 implements the `exec` producer (run the program, observe its stdout). the
# `field` and `flat-loader` producers are stubs deferred to #1760.

# produce_exec <runmode> <target> <binary>
# runs the built binary and forwards its stdout as the observable. native mode
# runs it directly; qemu mode runs it under the matching qemu-user (the target's
# arch is its name suffix, e.g. linux-riscv64 -> qemu-riscv64). the producer's
# exit status is the program's, so a crash (nonzero) fails the case.
produce_exec() {
    runmode=$1
    target=$2
    bin=$3
    if [ "$runmode" = "qemu" ]; then
        "qemu-${target##*-}" "$bin"
    else
        "$bin"
    fi
}

# produce_field — coreutils-header structural read; deferred to #1760.
produce_field() {
    echo "int: 'field' producer is deferred to #1760" >&2
    return 2
}

# produce_flat_loader — freestanding flat-image load + run; deferred to #1760.
produce_flat_loader() {
    echo "int: 'flat-loader' producer is deferred to #1760" >&2
    return 2
}

# produce <run> <runmode> <target> <binary>
# dispatches to the producer named by <run>, forwarding the remaining arguments.
produce() {
    run=$1
    shift
    case "$run" in
        exec)        produce_exec "$@" ;;
        field)       produce_field "$@" ;;
        flat-loader) produce_flat_loader "$@" ;;
        *) echo "int: unknown run mode '$run'" >&2; return 2 ;;
    esac
}
