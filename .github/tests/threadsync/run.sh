#!/usr/bin/env bash
# trailing PE import-descriptor call-thunk regression test (#1388 / #1399): the
# import call-thunk for the LAST import descriptor must jump through its own
# descriptor's IAT slot, not the previous descriptor's null terminator. std links
# four windows DLLs in order [kernel32, ws2_32, advapi32, api-ms-win-core-synch];
# WaitOnAddress / WakeByAddressSingle live in the 4th (trailing) descriptor.
#
# mach's native windows lane does not cover this: the compiler does not import
# std.sync.thread, so mach.exe imports the synch DLL dead (no call site) and the
# 2 std thread tests are not in its corpus; std's own CI is linux-only. this test
# closes that gap on every PR — it cross-compiles a thread spawn+join program
# (whose join() calls into the trailing synch descriptor) and runs it under wine.
# before #1388 the synch call dispatched through a null pointer and faulted; a
# regressed thunk faults again and fails the test.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$HERE/../../.." && pwd)"
STD="$REPO_ROOT/dep/mach-std"

if [ ! -d "$STD/src" ]; then
    echo "error: vendored std not found at $STD (run: git submodule update --init)" >&2
    exit 2
fi

# the test runs a windows binary; without wine there is nothing to execute it.
if ! command -v wine >/dev/null 2>&1; then
    echo "SKIP threadsync: 'wine' not available to run the windows binary"
    exit 0
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"

if ! ( cd "$WORK/app" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "FAIL threadsync: windows cross-build failed" >&2
    exit 1
fi

EXE="$WORK/app/out/windows/bin/threadsync.exe"
test -f "$EXE" || { echo "error: windows binary not produced at $EXE" >&2; exit 1; }

# a correct run completes in well under a second. bound it with `timeout`: a
# regressed thunk faults on the synch call, and wine's crash handler (winedbg
# --auto) can hang rather than exit, so a bare `wine` would stall CI instead of
# failing — the timeout turns that hang into a fast nonzero exit (124).
set +e
WINEDEBUG=-all timeout 60 wine "$EXE" >/dev/null 2>&1
code=$?
set -e

if [ "$code" -eq 0 ]; then
    echo "PASS threadsync: trailing-descriptor synch call-thunk dispatches (spawn+join under wine)"
    exit 0
fi

echo "FAIL threadsync: trailing-descriptor synch call faulted, hung, or worker did not run (exit $code)" >&2
exit 1
