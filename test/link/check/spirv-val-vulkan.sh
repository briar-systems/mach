#!/usr/bin/env bash
. "$(dirname "$0")/common.sh"

# spirv_val_env <binary> <target-env>
# shared body: glob the case's output root (a finished-module target has no linked
# binary, so the delivery is the module tree) and run spirv-val over each `.spv`.
# an EXTERNAL validator is the point — mach reading back its own bytes proves
# self-consistency, not validity. the observable is the module count plus the
# verdict, so a build that silently stopped emitting fails on the count rather than
# passing vacuously.
spirv_val_env() {
    out_dir=$(dirname "$1")
    env_arg=$2
    if ! command -v spirv-val >/dev/null 2>&1; then
        echo "link: spirv-val: the validator is not installed (spirv-tools)" >&2
        return 2
    fi
    n=0
    for m in $(find "$out_dir" -name '*.spv' | sort); do
        if [ -n "$env_arg" ]; then
            spirv-val --target-env "$env_arg" "$m" || return 1
        else
            spirv-val "$m" || return 1
        fi
        n=$((n + 1))
    done
    if [ "$n" -eq 0 ]; then
        echo "link: spirv-val: the build delivered no .spv module" >&2
        return 2
    fi
    if [ -n "$env_arg" ]; then
        printf 'modules=%d validator=clean env=%s\n' "$n" "$env_arg"
        return 0
    fi
    printf 'modules=%d validator=clean\n' "$n"
}

# produce_spirv_val_vulkan <engine> <leg> <binary>
# as produce_spirv_val, but validates against the VULKAN environment rather than
# the universal one. the two are different contracts and a module cannot satisfy
# both: a library module declares the Linkage capability so a consumer can find its
# exported functions, and Vulkan forbids that capability outright; a shader module
# carries entry points and no linkage at all. so a case picks the environment its
# module is actually meant for, and the stricter Vulkan rules — the fragment
# stage's mandatory origin, the compute stage's mandatory workgroup size — are
# genuinely checked rather than skipped by validating everything universally.
produce_spirv_val_vulkan() {
    spirv_val_env "$3" vulkan1.3
}

produce_spirv_val_vulkan "$@"
