#!/usr/bin/env bash
# win64runner integration test: `mach test --runner` / `mach run --runner`
# (#1345). foreign-target execution must not depend on the host's binfmt_misc
# registration: with `--runner wine` a windows-target suite runs end to end on
# any linux host with wine, and without the flag a spawn failure is reported
# as exactly that (FAIL(spawn) per test) rather than auto-detected around.
#
# the no-runner control is conditional: on a host whose binfmt does launch PE
# binaries (e.g. a dev box with wine's binfmt entry), direct exec legitimately
# succeeds, so the FAIL(spawn) assertion only applies when direct exec fails.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
# absolutize a path argument: the suites cd into temp dirs, so a relative
# compiler path would break after the first cd.
case "$MACH" in */*) MACH="$(cd "$(dirname "$MACH")" && pwd)/$(basename "$MACH")";; esac
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$HERE/../../.." && pwd)"
STD="$REPO_ROOT/dep/mach-std"

if [ ! -d "$STD/src" ]; then
    echo "error: vendored std not found at $STD (run: git submodule update --init)" >&2
    exit 2
fi

# the suite runs windows binaries through wine; without it there is no launcher.
if ! command -v wine >/dev/null 2>&1; then
    echo "SKIP win64runner: 'wine' not available to act as the runner"
    exit 0
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"
APP="$WORK/app"

fail=0

# control: without --runner the PEs are exec'd directly. on a host without a
# binfmt handler that must fail per test as FAIL(spawn) — never auto-detect.
set +e
( cd "$APP" && WINEDEBUG=-all "$MACH" test . ) >"$WORK/plain.out" 2>&1
plain=$?
set -e
if [ "$plain" -eq 0 ]; then
    echo "INFO win64runner: host binfmt launches PE binaries; skipping the FAIL(spawn) control"
elif grep -q 'FAIL(spawn)' "$WORK/plain.out"; then
    echo "PASS win64runner: without --runner a foreign binary reports FAIL(spawn)"
else
    echo "FAIL win64runner: runner-less failure was not reported as FAIL(spawn)" >&2
    cat "$WORK/plain.out" >&2
    fail=1
fi

# the feature: the declared launcher makes the suite pass with no binfmt help.
if ( cd "$APP" && WINEDEBUG=-all "$MACH" test . --runner wine ) >"$WORK/test.out" 2>&1 \
    && grep -q '2 passed, 0 failed, 2 total' "$WORK/test.out"; then
    echo "PASS win64runner: mach test --runner wine runs the windows suite"
else
    echo "FAIL win64runner: mach test --runner wine did not pass the suite" >&2
    cat "$WORK/test.out" >&2
    fail=1
fi

if ( cd "$APP" && WINEDEBUG=-all "$MACH" run . --runner wine ) >"$WORK/run.out" 2>&1; then
    echo "PASS win64runner: mach run --runner wine executes the windows binary"
else
    echo "FAIL win64runner: mach run --runner wine failed" >&2
    cat "$WORK/run.out" >&2
    fail=1
fi

exit "$fail"
