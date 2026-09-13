#!/usr/bin/env bash
. "$(dirname "$0")/common.sh"

# produce_flat_loader <engine> <leg> <binary>
# loads the freestanding raw image through the C loader beside this script and
# reports the image's exit status as the observable. any stdout the image writes
# flows first. a loader-infrastructure failure (no cc, mmap denied) returns nonzero.
produce_flat_loader() {
    bin=$3
    loader=$(mktemp -d)/flat_loader
    if ! cc -O2 -o "$loader" "$(dirname "$0")/flat_loader.c" 2>&1; then
        echo "link: flat-loader: could not build the C loader (cc required)" >&2
        return 2
    fi
    if "$loader" "$bin"; then ec=0; else ec=$?; fi
    rm -rf "$(dirname "$loader")"
    printf 'exit=%d\n' "$ec"
}

produce_flat_loader "$@"
