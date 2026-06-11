#!/usr/bin/env bash
# `mach info` integration test (#1312). asserts the at-a-glance compiler info
# surface, all of which must work with NO project (run from an empty temp dir):
#
#   info       — exit 0 and the expected line-oriented surface (version banner,
#                build host, and the four capability lines from the registries).
#   --version  — `mach info --version` prints the version string alone, one line.
#   banner     — the bare `mach` usage banner header surfaces the version.
#   no flag    — bare `mach --version` is NOT a recognized version flag: it falls
#                through to the unknown-command path (non-zero, no one-line
#                version), distinguishing it from the `mach info --version` alias.
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

# run from the empty WORK dir so every assertion proves the no-project path.
cd "$WORK"

# info — exit 0 and the full surface from an empty dir.
set +e
out="$("$MACH" info)"; code=$?
set -e
if [ "$code" -ne 0 ]; then
    echo "FAIL info/run: 'mach info' exited $code from a project-less dir" >&2
    printf '%s\n' "$out" >&2; fail=1
fi
for want in "^mach [0-9]" "^host: " "^isa: " "^os: " "^abi: " "^object: "; do
    if ! printf '%s\n' "$out" | grep -qE -- "$want"; then
        echo "FAIL info/run: 'mach info' output missing line matching /$want/" >&2
        printf '%s\n' "$out" >&2; fail=1
    fi
done
# the build host folds to a concrete os/isa, never the comptime-dead fallback.
if printf '%s\n' "$out" | grep -qE -- "^host: unknown"; then
    echo "FAIL info/run: 'mach info' host did not fold to a concrete os/isa" >&2
    printf '%s\n' "$out" >&2; fail=1
fi
if [ "$fail" -eq 0 ]; then echo "PASS info/run: 'mach info' prints the no-project surface"; fi

# --version — exactly one line, the version string alone (no "mach " prefix).
set +e
vout="$("$MACH" info --version)"; vcode=$?
set -e
vlines="$(printf '%s\n' "$vout" | grep -c '')"
if [ "$vcode" -ne 0 ]; then
    echo "FAIL info/version: 'mach info --version' exited $vcode" >&2; fail=1
elif [ "$vlines" -ne 1 ]; then
    echo "FAIL info/version: 'mach info --version' printed $vlines lines, want 1" >&2
    printf '%s\n' "$vout" >&2; fail=1
elif ! printf '%s' "$vout" | grep -qE '^[0-9]+\.[0-9]+\.[0-9]+'; then
    echo "FAIL info/version: 'mach info --version' is not a bare version string" >&2
    printf '%s\n' "$vout" >&2; fail=1
else
    echo "PASS info/version: 'mach info --version' is one bare version line"
fi

# banner — the bare 'mach' usage header surfaces the version.
set +e
banner="$("$MACH" 2>&1)"; bcode=$?
set -e
if ! printf '%s\n' "$banner" | grep -qE -- "^mach [0-9]+\.[0-9]+\.[0-9]+ — "; then
    echo "FAIL info/banner: bare 'mach' usage header does not surface the version" >&2
    printf '%s\n' "$banner" >&2; fail=1
else
    echo "PASS info/banner: bare 'mach' usage header surfaces the version"
fi

# no flag — bare 'mach --version' is unrecognized: non-zero, unknown-command path,
# and no standalone one-line version (its first line is the error, not a version).
set +e
gvout="$("$MACH" --version 2>&1)"; gvcode=$?
set -e
gvfirst="$(printf '%s\n' "$gvout" | head -n1)"
if [ "$gvcode" -eq 0 ]; then
    echo "FAIL info/global: bare 'mach --version' unexpectedly succeeded" >&2
    printf '%s\n' "$gvout" >&2; fail=1
elif ! printf '%s' "$gvout" | grep -qF -- "unknown command '--version'"; then
    echo "FAIL info/global: bare 'mach --version' is not handled as an unknown command" >&2
    printf '%s\n' "$gvout" >&2; fail=1
elif printf '%s' "$gvfirst" | grep -qE '^[0-9]+\.[0-9]+\.[0-9]+$'; then
    echo "FAIL info/global: bare 'mach --version' printed a standalone version line" >&2
    printf '%s\n' "$gvout" >&2; fail=1
else
    echo "PASS info/global: bare 'mach --version' is not a recognized version flag"
fi

exit "$fail"
