#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# produce_shared_exports <engine> <leg> <binary>
# reads the built library's export table, then links a real C consumer against it
# twice: once against the exported entry point, once against a private definition
# that carries a `#[symbol]` name. the second link must fail, which is the only
# evidence that a name is not a visibility.
produce_shared_exports() {
    b=$3
    here=$(dirname "$0")
    command -v nm >/dev/null 2>&1 || { echo "link: 3412-shared-exports: nm not found (install binutils)" >&2; return 2; }
    command -v cc >/dev/null 2>&1 || { echo "link: 3412-shared-exports: cc not found" >&2; return 2; }

    exported=$(nm -D --defined-only "$b" 2>/dev/null | awk '{ print $3 }' | LC_ALL=C sort | grep -c .)
    echo "exports=$exported"
    nm -D --defined-only "$b" 2>/dev/null | awk '{ print "export=" $3 }' | LC_ALL=C sort

    # the private definition the exported entry calls: present in the image, bound
    # locally, so no consumer can bind to it
    if nm "$b" 2>/dev/null | grep -qE ' t .*private_double$'; then
        echo "private_local=yes"
    else
        echo "private_local=no"
    fi
    # the private definition nothing calls: link GC drops it entirely, which it can
    # only do because no export roots it
    if nm "$b" 2>/dev/null | grep -q 'private_dead'; then
        echo "dead_absent=no"
    else
        echo "dead_absent=yes"
    fi

    tmp=$(mktemp -d)
    if cc -o "$tmp/consumer" "$here/consume.c" "$b" -Wl,-rpath,"$(dirname "$b")" >"$tmp/pub.log" 2>&1; then
        echo "c_pub_link=ok"
        if out=$("$tmp/consumer" 2>&1); then
            echo "$out"
        else
            echo "c_pub_run=failed"
            sed 's/^/    /' <<<"$out" >&2
        fi
    else
        echo "c_pub_link=failed"
        sed 's/^/    /' "$tmp/pub.log" >&2
    fi
    if cc -o "$tmp/consumer_private" "$here/consume_private.c" "$b" -Wl,-rpath,"$(dirname "$b")" >"$tmp/priv.log" 2>&1; then
        echo "c_private_link=linked"
    else
        echo "c_private_link=refused"
    fi
    rm -rf "$tmp"
}

produce_shared_exports "$@"
