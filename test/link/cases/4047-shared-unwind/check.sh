#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_shared_unwind <engine> <leg> <library>
# links a C consumer against the built library with the host toolchain, runs it
# and reports the frame libgcc's unwinder found each function in
produce_shared_unwind() {
    b=$3
    here=$(dirname "$0")
    command -v cc >/dev/null 2>&1 || { echo "link: 4047-shared-unwind: cc not found" >&2; return 2; }
    tmp=$(mktemp -d)
    if ! cc -O1 -fno-omit-frame-pointer -o "$tmp/consumer" "$here/consume.c" "$b" -Wl,-rpath,"$(dirname "$b")" >"$tmp/link.log" 2>&1; then
        echo "link: 4047-shared-unwind: the consumer did not link" >&2
        sed 's/^/    /' "$tmp/link.log" >&2
        rm -rf "$tmp"
        return 2
    fi
    run_captured "$1" "$2" "$tmp/consumer" "$tmp/out.txt"
    if [ "$run_status" -ne 0 ]; then
        echo "link: 4047-shared-unwind: the consumer $(describe_exit "$run_status")" >&2
        sed 's/^/    /' "$tmp/out.txt" >&2
        rm -rf "$tmp"
        return 2
    fi
    cat "$tmp/out.txt"
    rm -rf "$tmp"
}

produce_shared_unwind "$@"
