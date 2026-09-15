#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_required_embed <engine> <leg> <binary> <g-binary> <profile>
# validate the shader at the path `{artifact.shader-tint.out}` names, against the
# environment its target pins, then run the host program and check that the bytes it
# embedded are that file: same length and same byte sum. the length and sum move with
# codegen, so they are compared here rather than recorded in the golden.
produce_required_embed() {
    engine=$1
    target=$2
    bin=$3
    profile=${5:-debug}
    case_dir=$(cd "$(dirname "$0")" && pwd)
    module=$case_dir/out/link/spirv/$profile/spv/tint.spv

    if ! command -v spirv-val >/dev/null 2>&1; then
        echo "link: required-embed: the validator is not installed (spirv-tools)" >&2
        return 2
    fi
    [ -s "$module" ] || {
        echo "link: required-embed: no module at $module, the path the template names" >&2
        return 1
    }
    spirv-val --target-env vulkan1.0 "$module" || return 1

    out=$(mktemp)
    run_captured "$engine" "$target" "$bin" "$out" || { rm -f "$out"; return 1; }
    if [ "$run_status" -ne 0 ]; then
        report_run_failure "exec" "$run_status" "$run_out"
        rm -f "$out"
        return "$run_status"
    fi
    grep '^magic=' "$out"
    len=$(sed -n 's/^len=//p' "$out")
    sum=$(sed -n 's/^sum=//p' "$out")
    rm -f "$out"

    file_len=$(wc -c <"$module" | tr -d ' ')
    file_sum=$(od -An -v -tu1 "$module" | awk '{ for (i = 1; i <= NF; i++) s += $i } END { print s + 0 }')
    if [ "$len" = "$file_len" ] && [ "$sum" = "$file_sum" ]; then
        echo "embedded=delivered"
    else
        echo "link: required-embed: embedded len=$len sum=$sum, delivered len=$file_len sum=$file_sum" >&2
        return 1
    fi
    echo "delivered=clean env=vulkan1.0"
}

produce_required_embed "$@"
