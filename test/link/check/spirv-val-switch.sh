#!/usr/bin/env bash
. "$(dirname "$0")/common.sh"

# produce_spirv_val_switch <engine> <leg> <binary>
# as produce_spirv_val_vulkan, then the number of OpSwitch instructions in the
# delivered module: a structured target's exit dispatch is a switch construct, so
# the count shows the case still reaches the dispatch rather than validating a
# module that no longer needs one (#3371)
produce_spirv_val_switch() {
    spirv_val_env "$3" vulkan1.3 || return $?
    if ! command -v spirv-dis >/dev/null 2>&1; then
        echo "link: spirv-val-switch: spirv-dis is not installed (spirv-tools)" >&2
        return 2
    fi
    printf 'switches=%d\n' "$(spirv-dis "$3" | grep -c ' OpSwitch ')"
}

produce_spirv_val_switch "$@"
