#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_link_provider <engine> <leg> <binary>
produce_link_provider() {
    b=$3
    command -v nm >/dev/null 2>&1 || {
        echo "link: link-provider: nm not found (install the 'binutils' package)" >&2; return 2
    }
    if nm "$b" 2>/dev/null | grep -qE ' T provider_marker$'; then
        echo "provider_marker=defined"
    else
        echo "provider_marker=missing"
    fi
}

produce_link_provider "$@"
