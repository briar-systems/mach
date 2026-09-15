#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_spirv_debug <engine> <leg> <binary>
# validate each shader module against vulkan1.3 and list its debug model: the file names
# (basenames, the directory is the checkout's), the OpSource count, every OpName and
# OpMemberName literal, and the source lines OpLine attributes instructions to per file.
produce_spirv_debug() {
    out_dir=$(dirname "$3")
    if ! command -v spirv-val >/dev/null 2>&1 || ! command -v spirv-dis >/dev/null 2>&1; then
        echo "link: spirv-debug: spirv-tools is not installed" >&2
        return 2
    fi
    n=0
    for m in $(find "$out_dir" -name '*.spv' | sort); do
        dis=$(spirv-dis --no-header --no-color --raw-id "$m") || return 1
        printf '%s\n' "$dis" | grep -q '^ *OpEntryPoint ' || continue
        spirv-val --target-env vulkan1.3 "$m" || return 1
        printf 'module=%s env=vulkan1.3 validator=clean\n' "${m#"$out_dir"/}"
        printf '%s\n' "$dis" | awk '
            $3 == "OpString" { s = $4; for (i = 5; i <= NF; i++) s = s " " $i; gsub(/"/, "", s); n = split(s, p, "/"); file[$1] = p[n]; printf "  string %s\n", p[n]; next }
            $1 == "OpSource" { sources++; printf "  source %s %s %s\n", $2, $3, file[$4]; next }
            $1 == "OpName" { printf "  name %s\n", $3; next }
            $1 == "OpMemberName" { printf "  member %s %s\n", $3, $4; next }
            $1 == "OpLine" { lines[file[$2] ":" $3] = 1; next }
            END {
                for (k in lines) print "  line " k | "sort -t: -k1,1 -k2,2n"
            }
        '
        n=$((n + 1))
    done
    if [ "$n" -eq 0 ]; then
        echo "link: spirv-debug: the build delivered no shader module" >&2
        return 2
    fi
    printf 'modules=%d\n' "$n"
}

produce_spirv_debug "$@"
