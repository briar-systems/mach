#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_import_address <engine> <leg> <binary>
# the program's own output, run under the leg's engine. the image is dynamic, so
# under qemu the loader comes from the leg's sysroot: MACH_RISCV64_SYSROOT, the
# one cc.sh compiles against, or where Ubuntu's libc6-<arch>-cross installs it,
# which ci.yml installs for riscv64
produce_import_address() {
    out=$(mktemp)
    case "$1" in
        native) "$3" >"$out" 2>&1; status=$? ;;
        qemu:*)
            case "$2" in
                riscv64-linux) prefix=${MACH_RISCV64_SYSROOT:-/usr/riscv64-linux-gnu} ;;
                *) echo "link: import-address: no loader prefix known for leg '$2'" >&2; rm -f "$out"; return 2 ;;
            esac
            if [ ! -d "$prefix" ]; then
                echo "link: import-address: no $2 loader at '$prefix'; set MACH_RISCV64_SYSROOT to a sysroot" >&2
                rm -f "$out"; return 2
            fi
            "${1#qemu:}" -L "$prefix" "$3" >"$out" 2>&1; status=$?
            ;;
        *) echo "link: import-address: '$2' declares engine '$1', which executes nothing" >&2; rm -f "$out"; return 2 ;;
    esac
    if [ "$status" -ne 0 ]; then
        report_run_failure "import-address" "$status" "$(cat "$out")"; rm -f "$out"; return 1
    fi
    cat "$out"
    rm -f "$out"
}

produce_import_address "$@"
