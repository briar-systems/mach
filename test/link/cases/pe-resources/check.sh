#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# pe_resource_leaf <file> <section-table> <nsec> <resource-base-off> <type> <name>
# print "payload-file-offset size" for language id 0, or fail when absent.
pe_resource_leaf() {
    bin=$1; sec=$2; nsec=$3; root=$4; want_type=$5; want_name=$6
    type_count=$(read_le_uint "$bin" $((root + 14)) 2)
    ti=0
    while [ "$ti" -lt "$type_count" ]; do
        te=$((root + 16 + ti * 8))
        tid=$(read_le_uint "$bin" "$te" 4)
        if [ "$tid" -eq "$want_type" ]; then
            tchild=$(read_le_uint "$bin" $((te + 4)) 4)
            [ $((tchild & 0x80000000)) -ne 0 ] || return 1
            l2=$((root + (tchild & 0x7fffffff)))
            name_count=$(read_le_uint "$bin" $((l2 + 14)) 2)
            ni=0
            while [ "$ni" -lt "$name_count" ]; do
                ne=$((l2 + 16 + ni * 8))
                nid=$(read_le_uint "$bin" "$ne" 4)
                if [ "$nid" -eq "$want_name" ]; then
                    nchild=$(read_le_uint "$bin" $((ne + 4)) 4)
                    [ $((nchild & 0x80000000)) -ne 0 ] || return 1
                    l3=$((root + (nchild & 0x7fffffff)))
                    lang_count=$(read_le_uint "$bin" $((l3 + 14)) 2)
                    [ "$lang_count" -gt 0 ] || return 1
                    le=$((l3 + 16))
                    [ "$(read_le_uint "$bin" "$le" 4)" -eq 0 ] || return 1
                    de_rel=$(read_le_uint "$bin" $((le + 4)) 4)
                    [ $((de_rel & 0x80000000)) -eq 0 ] || return 1
                    de=$((root + de_rel))
                    data_rva=$(read_le_uint "$bin" "$de" 4)
                    data_size=$(read_le_uint "$bin" $((de + 4)) 4)
                    data_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$data_rva") || return 1
                    echo "$data_off $data_size"
                    return 0
                fi
                ni=$((ni + 1))
            done
        fi
        ti=$((ti + 1))
    done
    return 1
}

# independently validate the PE resources produced from the fixture manifest
produce_pe_resources() {
    bin=$3
    elfanew=$(read_le_uint "$bin" 60 4)
    nsec=$(read_le_uint "$bin" $((elfanew + 6)) 2)
    optsize=$(read_le_uint "$bin" $((elfanew + 20)) 2)
    oh=$((elfanew + 24))
    sec=$((oh + optsize))
    [ "$(read_le_uint "$bin" "$oh" 2)" -eq 523 ] || return 2
    resource_rva=$(read_le_uint "$bin" $((oh + 112 + 2 * 8)) 4)
    resource_size=$(read_le_uint "$bin" $((oh + 112 + 2 * 8 + 4)) 4)
    [ "$resource_rva" -gt 0 ] && [ "$resource_size" -gt 0 ] || return 2
    root=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$resource_rva") || return 2

    set -- $(pe_resource_leaf "$bin" "$sec" "$nsec" "$root" 3 1) || return 2
    icon_off=$1; icon_size=$2
    [ "$icon_size" -eq 4 ] || return 2
    [ "$(read_le_uint "$bin" "$icon_off" 4)" -eq 1144201745 ] || return 2

    set -- $(pe_resource_leaf "$bin" "$sec" "$nsec" "$root" 14 1) || return 2
    group_off=$1; group_size=$2
    [ "$group_size" -eq 20 ] || return 2
    [ "$(read_le_uint "$bin" $((group_off + 4)) 2)" -eq 1 ] || return 2
    [ "$(read_le_uint "$bin" $((group_off + 18)) 2)" -eq 1 ] || return 2

    set -- $(pe_resource_leaf "$bin" "$sec" "$nsec" "$root" 16 1) || return 2
    version_off=$1
    [ "$(read_le_uint "$bin" $((version_off + 40)) 4)" -eq 4277077181 ] || return 2
    [ "$(read_le_uint "$bin" $((version_off + 48)) 4)" -eq 196612 ] || return 2
    [ "$(read_le_uint "$bin" $((version_off + 52)) 4)" -eq 327680 ] || return 2

    set -- $(pe_resource_leaf "$bin" "$sec" "$nsec" "$root" 24 1) || return 2
    manifest_off=$1; manifest_size=$2
    case_dir=$(CDPATH= cd -- "$(dirname -- "$bin")/../.." && pwd)
    _pr_tmp=$(mktemp)
    dd if="$bin" of="$_pr_tmp" bs=1 skip="$manifest_off" count="$manifest_size" 2>/dev/null
    cmp -s "$_pr_tmp" "$case_dir/assets/app.manifest" || { rm -f "$_pr_tmp"; return 2; }
    rm -f "$_pr_tmp"

    [ "$(hex_count "$bin" 670061006d0065002d0061007000700000)" -eq 2 ] || return 2
    [ "$(hex_count "$bin" 700072006f0067000000)" -eq 1 ] || return 2
    [ "$(hex_count "$bin" 33002e0034002e0035000000)" -eq 2 ] || return 2
    [ "$(hex_count "$bin" 460069006c0065004400650073006300720069007000740069006f006e000000)" -eq 0 ] || return 2

    initialized=0
    i=0
    while [ "$i" -lt "$nsec" ]; do
        sh=$((sec + i * 40))
        chars=$(read_le_uint "$bin" $((sh + 36)) 4)
        if [ $((chars & 0x40)) -ne 0 ]; then
            initialized=$((initialized + $(read_le_uint "$bin" $((sh + 16)) 4)))
        fi
        i=$((i + 1))
    done
    [ "$(read_le_uint "$bin" $((oh + 8)) 4)" -eq "$initialized" ] || return 2

    echo "icon=present"
    echo "group_icon=present"
    echo "version=3.4.5"
    echo "manifest=preserved"
    echo "metadata=game-app/prog"
    echo "initialized_data=complete"
}

produce_pe_resources "$@"
