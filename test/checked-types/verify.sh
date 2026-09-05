#!/usr/bin/env sh
set -eu

here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
root=$(CDPATH= cd -- "$here/../.." && pwd)

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

compiler=${MACH_CHECKED_COMPILER:-}
if [ -z "$compiler" ]; then
    hd=$(host_dir) || {
        echo "checked-types: no default compiler path for $(uname -s)/$(uname -m); set MACH_CHECKED_COMPILER" >&2
        exit 2
    }
    compiler=$root/out/$hd/debug/bin/mach
    [ -f "$compiler" ] || compiler=$compiler.exe
fi
case "$compiler" in
    /*) : ;;
    *) compiler=$(CDPATH= cd -- "$(dirname -- "$compiler")" && pwd)/$(basename -- "$compiler") ;;
esac
if [ ! -f "$compiler" ]; then
    echo "checked-types: the compiler under test is not at '$compiler'; build it or set MACH_CHECKED_COMPILER" >&2
    exit 2
fi

if [ ! -e "$here/provider/src" ]; then
    ln -s ../../../src "$here/provider/src"
fi
"$compiler" dep pull "$here" >/dev/null

verify_rejection() {
    artifact=$1
    first_type=$2
    second_type=$3
    if output=$("$compiler" build "$here" --lib "$artifact" --emit obj 2>&1); then
        echo "checked-types: $artifact unexpectedly compiled" >&2
        exit 1
    fi
    echo "$output" | grep -F "type mismatch" >/dev/null
    echo "$output" | grep -F "$first_type" >/dev/null
    echo "$output" | grep -F "$second_type" >/dev/null
}

verify_rejection cross-unit-alignment "CheckedAlignment[Bytes]" "CheckedAlignment[Words]"
verify_rejection cross-domain-id "CheckedCount[NodeDomain]" "CheckedCount[BlockDomain]"
