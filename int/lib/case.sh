# case.sh — case-manifest loading, shared by run.sh and check-target-matrix.sh.
#
# the single reader for case.conf's key set and targets.conf's leg list (#2353):
# a second implementation of either schema is exactly the kind of drifting
# parallel enumeration this repo treats as a recurring defect family, and both
# callers need the identical "which legs does this case run on" answer.

_case_lib_dir=$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
_case_conf="$_case_lib_dir/../targets.conf"

# all_targets — print every target name declared in targets.conf.
all_targets() {
    while read -r name runner runmode rowcadence rest; do
        case "$name" in ''|\#*) continue ;; esac
        echo "$name"
    done < "$_case_conf"
}

# conf_runmode <target> — print the run-mode column for a target, or fail.
conf_runmode() {
    while read -r name runner runmode rowcadence rest; do
        case "$name" in ''|\#*) continue ;; esac
        if [ "$name" = "$1" ]; then echo "$runmode"; return 0; fi
    done < "$_case_conf"
    return 1
}

# in_list <item> <space-separated-list>
in_list() {
    case " $2 " in *" $1 "*) return 0 ;; esac
    return 1
}

# read_case_conf <case-dir> — load a case's defaults, then override from
# case.conf if present.
#
# Sets case_targets, case_exempt, case_profiles, case_run, case_build_target,
# case_build_flags, case_self_host, and case_allow: the `targets:` resolution
# (all targets.conf legs, or the explicit list) BEFORE the `exempt:`
# subtraction a caller applies per leg with `in_list "$leg" "$case_exempt"` -
# two separate checks, matching how a leg is included then excluded rather
# than folding both into one filtered list here.
read_case_conf() {
    dir=$1
    case_targets=all
    case_exempt=
    case_profiles="debug release"
    case_run=exec
    case_build_target=
    case_build_flags=
    case_self_host=false
    if [ -f "$dir/case.conf" ]; then
        while IFS= read -r line || [ -n "$line" ]; do
            case "$line" in ''|\#*) continue ;; esac
            key=${line%%:*}
            value=${line#*:}
            key=$(echo "$key" | tr -d '[:space:]')
            value=$(echo "$value" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')
            case "$key" in
                targets)      case_targets=$value ;;
                exempt)       case_exempt=$value ;;
                profiles)     case_profiles=$value ;;
                run)          case_run=$value ;;
                build-target) case_build_target=$value ;;
                build-flags)  case_build_flags=$value ;;
                self-host)    case_self_host=$value ;;
                *) echo "run.sh: $dir/case.conf: unknown key '$key'" >&2; exit 2 ;;
            esac
        done < "$dir/case.conf"
    fi
    if [ "$case_targets" = all ]; then
        case_allow=$(all_targets | tr '\n' ' ')
    else
        case_allow=$case_targets
    fi
}
