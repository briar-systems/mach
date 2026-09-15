#!/usr/bin/env bash
. "$(dirname "$0")/common.sh"

# produce_spirv_val_pinned <engine> <leg> <binary>
# validate each module against the Vulkan environment its version word pins, then
# report the forms that depend on that version.
#
# THE VERSION DECIDES THE RULES. a module a `vulkan1.0` target writes is SPIR-V 1.0,
# and validating it as vulkan1.3 applies 1.6 rules that accept an interface list and
# a storage class 1.0 forbids, so the environment is read from the module rather than
# assumed. the forms are printed as well as validated: an entry point that lists no
# descriptors and a storage buffer spelled either way are both valid in some version,
# so only the listing shows the emitter chose the one this version requires (#3399).
produce_spirv_val_pinned() {
    out_dir=$(dirname "$3")
    if ! command -v spirv-val >/dev/null 2>&1 || ! command -v spirv-dis >/dev/null 2>&1; then
        echo "link: spirv-val-pinned: spirv-tools is not installed" >&2
        return 2
    fi
    n=0
    for m in $(find "$out_dir" -name '*.spv' | sort); do
        word=$(od -An -tx4 -j4 -N4 "$m" | tr -d ' ')
        case "$word" in
            00010000) env=vulkan1.0 ;;
            00010300) env=vulkan1.1 ;;
            00010500) env=vulkan1.2 ;;
            00010600) env=vulkan1.3 ;;
            *) echo "link: spirv-val-pinned: version word $word pins no Vulkan environment" >&2; return 1 ;;
        esac
        spirv-val --target-env "$env" "$m" || return 1
        printf 'module=%s env=%s validator=clean\n' "${m#"$out_dir"/}" "$env"
        spirv-dis --no-header --no-color --raw-id "$m" | awk '
            $1 == "OpDecorate" && ($3 == "Block" || $3 == "BufferBlock") { block[$2] = $3; next }
            $3 == "OpTypeStruct" { if ($1 in block) { form[$1] = block[$1] } next }
            $3 == "OpTypePointer" { if ($5 in form) { pform[$1] = form[$5] } next }
            $3 == "OpVariable" {
                class[$1] = $5
                if ($4 in pform) { printf "  block storage=%s decoration=%s\n", $5, pform[$4] }
                next
            }
            $1 == "OpEntryPoint" { entries[++ne] = $0; next }
            END {
                for (k = 1; k <= ne; k++) {
                    split(entries[k], f, " ")
                    line = "  entry " f[2] " " f[4] " interface="
                    sep = ""
                    for (j = 5; j in f; j++) { line = line sep class[f[j]]; sep = "," }
                    print line
                }
            }
        '
        n=$((n + 1))
    done
    if [ "$n" -eq 0 ]; then
        echo "link: spirv-val-pinned: the build delivered no .spv module" >&2
        return 2
    fi
    printf 'modules=%d\n' "$n"
}

produce_spirv_val_pinned "$@"
