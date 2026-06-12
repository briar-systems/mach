#!/usr/bin/env bash
# `mach init` id-validation integration test (#1355). the project id (the
# positional name, or `--name`) must be a valid id BEFORE any scaffolding: an id
# the manifest grammar would reject (`.`, path separators, spaces) is refused and
# nothing is written. a valid id scaffolds a grammatical `[bin.<id>]` manifest.
#
# the refusal cases run fully offline — validation fails before any filesystem
# write or dependency clone. the valid case asserts on the written manifest
# (which precedes the network `dep` sync), so it does not depend on connectivity.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
# absolutize a path argument: the suite cds into a temp dir, so a relative
# compiler path would break after the cd.
case "$MACH" in */*) MACH="$(cd "$(dirname "$MACH")" && pwd)/$(basename "$MACH")";; esac

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

fail=0
ok()  { echo "PASS init: $1"; }
bad() { echo "FAIL init: $1" >&2; fail=1; }

# refuse <desc> <name...> — `mach init <name>` in a fresh empty dir must exit
# nonzero, name the bad id, and leave the directory untouched (no mach.toml, no
# src/, and no derived target directory).
refuse() {
    local desc="$1"; shift
    local d; d="$(mktemp -d "$WORK/case.XXXXXX")"
    local out
    set +e
    out="$( cd "$d" && "$MACH" init "$@" 2>&1 )"; local code=$?
    set -e
    if [ "$code" -eq 0 ]; then
        bad "$desc: init exited 0 (expected refusal)"; printf '%s\n' "$out" >&2; return
    fi
    if ! printf '%s\n' "$out" | grep -qF -- "is not a valid project id"; then
        bad "$desc: error did not name an invalid id"; printf '%s\n' "$out" >&2; return
    fi
    # nothing written: the case dir must be empty (no mach.toml, src, or subdir).
    if [ -n "$(ls -A "$d")" ]; then
        bad "$desc: refused init left files behind: $(ls -A "$d" | tr '\n' ' ')"; return
    fi
    ok "$desc"
}

refuse "'.' is refused, nothing written"          .
refuse "'..' is refused, nothing written"         ..
refuse "a path-separator name is refused"         foo/bar
refuse "a dotted name is refused"                 foo.bar
refuse "a name with spaces is refused"            "a b"
refuse "an invalid --name is refused"             proj --name "bad id"

# valid case: a bare-key id scaffolds a grammatical manifest. assert on the
# written files (which precede the network dep sync) so the case is offline-safe.
VALID="$WORK/valid"; mkdir -p "$VALID"
set +e
( cd "$VALID" && "$MACH" init valid_name >/dev/null 2>&1 )
set -e
TOML="$VALID/valid_name/mach.toml"
if [ ! -f "$TOML" ]; then
    bad "a valid id writes mach.toml"
else
    grep -qF 'id = "valid_name"' "$TOML" && ok "the manifest carries the id verbatim" \
        || bad "the manifest id is wrong"
    grep -qF '[bin.valid_name]' "$TOML" && ok "the artifact table names the id" \
        || bad "the artifact table is missing or misnamed"
    grep -qF '[bin..]' "$TOML" && bad "the manifest contains an empty bin key" \
        || ok "the manifest has no empty bin key"
fi
[ -f "$VALID/valid_name/src/main.mach" ] && ok "the starter source is written" \
    || bad "src/main.mach is missing"

exit "$fail"
