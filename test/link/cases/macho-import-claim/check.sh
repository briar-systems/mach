#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_macho_imports <engine> <leg> <binary>
#
# Every `<dylib>:<symbol>` the image asks dyld to bind, from both the bind and the
# lazy-bind table, sorted. Mach-O records imports per dependency exactly as PE
# does, so this is the observable for attribution on darwin: a claim that failed
# to attribute does not merely bind to the wrong dylib, it fails the link, and a
# claim attributed to the wrong entry shows up here as the wrong dylib name.
#
# Both tables carry the dylib in the second-to-last column and the symbol in the
# last, so one rule reads either; a data row is recognized by its leading segment
# name, which no header line has.
#
# The sort is pinned to the C collation, as produce_pe_imports's is: a dylib list
# mixing cases (`libSystem`, `libcurses`) orders differently under a UTF-8 locale
# than under C, so an unpinned sort makes the golden depend on the runner's
# environment rather than on the emitted image.
produce_macho_imports() {
    _engine=$1
    _target=$2
    bin=$3

    binds=$(macho_objdump --macho --bind "$bin") || return 2
    lazy=$(macho_objdump --macho --lazy-bind "$bin") || return 2
    printf '%s\n%s\n' "$binds" "$lazy" |
        awk '$1 ~ /^__/ && NF >= 5 { print $(NF-1) ":" $NF }' |
        LC_ALL=C sort -u
}

produce_macho_imports "$@"
