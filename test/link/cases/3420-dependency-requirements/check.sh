#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_dependency_requirements <engine> <leg> <binary> <g-binary> <profile>
# validate both delivered modules, then run the program and check that the bytes each
# dependency embedded are its own module: same length and same byte sum, and the two
# modules differ. lengths and sums move with codegen, so they are compared here rather
# than recorded in the golden.
produce_dependency_requirements() {
    engine=$1
    target=$2
    bin=$3
    profile=${5:-debug}
    case_dir=$(cd "$(dirname "$0")" && pwd)
    home=$case_dir/out/link/spirv/$profile/dep

    if ! command -v spirv-val >/dev/null 2>&1; then
        echo "link: dependency-requirements: the validator is not installed (spirv-tools)" >&2
        return 2
    fi
    for owner in boom shader; do
        module=$home/$owner/spv/quad.spv
        [ -s "$module" ] || {
            echo "link: dependency-requirements: no module at $module, the path $owner's manifest names" >&2
            return 1
        }
        spirv-val "$module" || return 1
    done

    out=$(mktemp)
    run_captured "$engine" "$target" "$bin" "$out" || { rm -f "$out"; return 1; }
    if [ "$run_status" -ne 0 ]; then
        report_run_failure "exec" "$run_status" "$run_out"
        rm -f "$out"
        return "$run_status"
    fi

    for owner in boom shader; do
        module=$home/$owner/spv/quad.spv
        len=$(sed -n "s/^$owner len=\([0-9]*\) sum=.*/\1/p" "$out")
        sum=$(sed -n "s/^$owner len=[0-9]* sum=\([0-9]*\)/\1/p" "$out")
        file_len=$(wc -c <"$module" | tr -d ' ')
        file_sum=$(od -An -v -tu1 "$module" | awk '{ for (i = 1; i <= NF; i++) s += $i } END { print s + 0 }')
        if [ "$len" != "$file_len" ] || [ "$sum" != "$file_sum" ]; then
            echo "link: dependency-requirements: $owner embedded len=$len sum=$sum, delivered len=$file_len sum=$file_sum" >&2
            rm -f "$out"
            return 1
        fi
        echo "$owner=delivered"
    done

    if cmp -s "$home/boom/spv/quad.spv" "$home/shader/spv/quad.spv"; then
        echo "link: dependency-requirements: both dependencies delivered the same module" >&2
        rm -f "$out"
        return 1
    fi
    rm -f "$out"
    echo "homes=distinct"

    # the step boom's default library requires runs for the consumer, homed in the
    # consumer's output tree
    stamp=$case_dir/out/link/$target/$profile/stamp/boom.txt
    [ -f "$stamp" ] || {
        echo "link: dependency-requirements: no stamp at $stamp, the file boom's required step writes" >&2
        return 1
    }
    echo "stamp=$(cat "$stamp")"
}

produce_dependency_requirements "$@"
