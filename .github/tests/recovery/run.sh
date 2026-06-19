#!/usr/bin/env bash
# Parser error-recovery at declaration scope. A removed-syntax error — a comptime
# attribute setter (`$sym.attr = value;`) or a C-style `...` variadic parameter,
# both removed in v2.0.0 — must emit its ONE migration diagnostic and resync on
# the NEXT declaration. It must NOT skip that declaration and reparse its body at
# module scope, which used to spray a spurious "expected a declaration" for every
# non-`val` body statement (sync_to_decl's unconditional leading advance, fixed
# for v2.0.1). Each app's function body is all-valid, so a correct compiler emits
# exactly the migration diagnostic and zero "expected a declaration" cascade.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
case "$MACH" in */*) MACH="$(cd "$(dirname "$MACH")" && pwd)/$(basename "$MACH")";; esac
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
fail=0

# check <app-dir> <expected-diagnostic-substring>
check() {
    local app="$1"; local want="$2"
    cp -r "$HERE/$app" "$WORK/$app"
    local out
    if out="$( cd "$WORK/$app" && "$MACH" build . 2>&1 )"; then
        echo "FAIL recovery/$app: build unexpectedly succeeded (removed syntax must error)" >&2
        fail=1; return
    fi
    if ! printf '%s' "$out" | grep -qF -- "$want"; then
        echo "FAIL recovery/$app: missing migration diagnostic '$want'; got:" >&2
        printf '%s\n' "$out" >&2; fail=1; return
    fi
    if printf '%s' "$out" | grep -qF -- "expected a declaration"; then
        echo "FAIL recovery/$app: parse cascade — the error spilled into the function body:" >&2
        printf '%s\n' "$out" >&2; fail=1; return
    fi
    echo "PASS recovery/$app: one migration diagnostic, recovered at the next decl (no cascade)"
}

check setter  "comptime attribute setters were removed in v2.0.0"
check varargs 'C-style variadic `...` was removed in v2.0.0'

exit "$fail"
