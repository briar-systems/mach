#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_spirv_image <engine> <leg> <binary>
# validate the delivered module tree and report its image / sampler surface.
#
# THE OPERANDS ARE THE OBSERVABLE. `OpTypeImage` carries seven of them, and every
# one of the wrong values a bug produces is a module spirv-val accepts: a Dim of 2D
# where the source wrote `sampler3d`, an Arrayed of 0 where it wrote `array`, a
# Sampled of 2 (a storage image) where the sampled form was meant. So each is
# printed by name rather than counted, and so is the descriptor the handle is bound
# at - a set or binding that came out 0 binds the wrong descriptor and validates.
#
# Ids are replaced by the shape behind them throughout. An image type has no
# spirv-dis friendly name, so `%22` in a sample instruction says nothing on its own;
# resolving it to `sampledimage(image(float,2D,...))` is what makes "this sample
# reads THAT handle" an assertion rather than a claim about two numbers that
# renumber whenever anything else in the module changes.
produce_spirv_image() {
    out_dir=$(dirname "$3")
    if ! command -v spirv-val >/dev/null 2>&1; then
        echo "link: spirv-image: the validator is not installed (spirv-tools)" >&2
        return 2
    fi
    if ! command -v spirv-dis >/dev/null 2>&1; then
        echo "link: spirv-image: the disassembler is not installed (spirv-tools)" >&2
        return 2
    fi
    n=0
    for m in $(find "$out_dir" -name '*.spv' | sort); do
        dis=$(spirv-dis --no-header "$m") || return 1
        rel=${m#"$out_dir"/}
        if printf '%s\n' "$dis" | grep -q '^ *OpEntryPoint '; then
            kind=shader
            spirv-val --target-env vulkan1.3 "$m" || return 1
            env=vulkan1.3
        else
            kind=library
            spirv-val "$m" || return 1
            env=universal
        fi
        printf 'module=%s kind=%s env=%s validator=clean\n' "$rel" "$kind" "$env"
        printf '%s\n' "$dis" | awk '
            # the capabilities a dimensionality demands. a 1D or arrayed-cube image
            # declared without its capability is an invalid module, and one declared
            # WITH a capability nothing needs is a module a driver may refuse to
            # load, so both directions are in the observable.
            $1 == "OpCapability" { printf "  capability %s\n", $2; next }

            # the descriptor a handle is bound at. decorations precede the types and
            # variables they apply to, so one pass suffices.
            $1 == "OpDecorate" && $3 == "DescriptorSet" { dset[$2] = $4; next }
            $1 == "OpDecorate" && $3 == "Binding"       { dbind[$2] = $4; next }

            # "%4 = OpTypeImage %float 2D 0 0 0 1 Unknown" - all seven operands.
            $3 == "OpTypeImage" {
                d = "image(" $4 "," $5 ",depth=" $6 ",arrayed=" $7 ",ms=" $8 ",sampled=" $9 "," $10 ")"
                desc[$1] = d
                printf "  type %s\n", d
                next
            }
            $3 == "OpTypeSampledImage" {
                u = desc[$4]; if (u == "") { u = $4 }
                desc[$1] = "sampledimage(" u ")"
                printf "  type %s\n", desc[$1]
                next
            }
            $3 == "OpTypeSampler" { desc[$1] = "sampler"; printf "  type sampler\n"; next }

            # a pointer to a handle, so a variable can be reported by what it points
            # at rather than by spirv-dis, which names it after an id.
            $3 == "OpTypePointer" {
                u = desc[$5]
                if (u != "") { desc[$1] = "ptr(" $4 "," u ")"; ptr[$1] = u }
                next
            }

            # "%9 = OpVariable %_ptr_UniformConstant_7 UniformConstant" - only the
            # handle-typed ones; every other variable belongs to another feature.
            $3 == "OpVariable" && ptr[$4] != "" {
                printf "  binding set=%s binding=%s storage=%s type=%s\n",
                    (($1 in dset) ? dset[$1] : "<none>"),
                    (($1 in dbind) ? dbind[$1] : "<none>"),
                    $5, ptr[$4]
                next
            }

            # "%22 = OpLoad %5 %7" - remember the result type so a sample can be
            # reported by the handle it reads rather than by an operand id.
            $3 == "OpLoad" { rty[$1] = $4; next }

            $3 == "OpSampledImage" {
                rty[$1] = $4
                i = desc[rty[$5]]; if (i == "") { i = "<unknown>" }
                s = desc[rty[$6]]; if (s == "") { s = "<unknown>" }
                printf "  combine image=%s sampler=%s operands=%d\n", i, s, NF - 4
                next
            }
            $3 == "OpImageSampleImplicitLod" {
                h = desc[rty[$5]]; if (h == "") { h = "<unknown>" }
                printf "  sample result=%s handle=%s operands=%d\n", $4, h, NF - 4
                next
            }
        '
        n=$((n + 1))
    done
    if [ "$n" -eq 0 ]; then
        echo "link: spirv-image: the build delivered no .spv module" >&2
        return 2
    fi
    printf 'modules=%d\n' "$n"
}

produce_spirv_image "$@"
