#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_spirv_spec <engine> <leg> <binary>
# validate each module against vulkan1.3 and report its specialization constants:
# the SpecId, the opcode and type, the default literal, and how many instructions in
# a function body read the constant. a spec constant folded to its default is still a
# valid module, so the reads are what the golden holds.
produce_spirv_spec() {
    out_dir=$(dirname "$3")
    if ! command -v spirv-val >/dev/null 2>&1 || ! command -v spirv-dis >/dev/null 2>&1; then
        echo "link: spirv-spec: spirv-tools is not installed" >&2
        return 2
    fi
    n=0
    for m in $(find "$out_dir" -name '*.spv' | sort); do
        spirv-val --target-env vulkan1.3 "$m" || return 1
        printf 'module=%s env=vulkan1.3 validator=clean\n' "${m#"$out_dir"/}"
        spirv-dis --no-header --no-color --raw-id "$m" | awk '
            $1 == "OpDecorate" && $3 == "SpecId" { spec[$2] = $4; next }
            $3 == "OpTypeInt"   { ty[$1] = ($5 == 1 ? "i" : "u") $4; next }
            $3 == "OpTypeFloat" { ty[$1] = "f" $4; next }
            $3 == "OpTypeBool"  { ty[$1] = "bool"; next }
            $3 == "OpConstant"  { cval[$1] = $5; next }
            $3 ~ /^OpSpecConstant/ && ($1 in spec) {
                op[$1] = $3; sty[$1] = ty[$4]; def[$1] = ($5 == "" ? "-" : $5); next
            }
            $1 == "OpExecutionModeId" { mode = $0; next }
            $1 == "OpFunction" || $3 == "OpFunction" { body = 1 }
            body { for (k = 1; k <= NF; k++) if ($k in spec) reads[$k]++ }
            END {
                for (s in spec) order[spec[s]] = s
                for (i = 0; i < 64; i++) {
                    if (!(i in order)) continue
                    s = order[i]
                    printf "  spec id=%d %s %s default=%s reads=%d\n", i, op[s], sty[s], def[s], reads[s] + 0
                }
                if (mode != "") {
                    split(mode, f, " ")
                    line = "  mode " f[3]
                    for (k = 4; k in f; k++) {
                        if (f[k] in spec) line = line " spec(" spec[f[k]] ")"
                        else line = line " const(" cval[f[k]] ")"
                    }
                    print line
                }
            }
        '
        n=$((n + 1))
    done
    if [ "$n" -eq 0 ]; then
        echo "link: spirv-spec: the build delivered no .spv module" >&2
        return 2
    fi
    printf 'modules=%d\n' "$n"
}

produce_spirv_spec "$@"
