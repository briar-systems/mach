#!/usr/bin/env bash
# inline decorator integration test (#1476): prove an `inline` backtick decorator
# forces inlining of a function the cost model would otherwise leave alone.
#
# `big` exceeds the inline pass's instruction threshold and is called from two
# sites, so neither the size nor the single-use heuristic inlines it — only
# FN_FLAG_INLINE does. building at -O2 with `--emit-asm`, the decorated build
# must contain NO call to `big` in the caller's assembly (fully inlined), while a
# control build with the `inline` decorator stripped must still call it.
#
# `big` is fed a runtime value and its result is returned, so the calls can't be
# constant-folded or dead-code-eliminated — without that the control's surviving-
# call signal would be a no-op. its link name is pinned with `symbol`, so the
# `call big` grep matches a stable mnemonic. run with no arguments the program is
# deterministic: exit 204 (the low byte of big(big(1,2),3)).
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
    echo "error: vendored std not found at $STD (run: mach dep pull)" >&2
    exit 2
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"

fail=0

# count_big_calls — number of `call` instructions targeting `big` in the app's
# emitted assembly (across whatever path the asm artifact lands under).
count_big_calls() {
    local n=0 f
    while IFS= read -r f; do
        n=$(( n + $(grep -cE 'call[[:space:]].*big' "$f" || true) ))
    done < <(find "$WORK/app/out" -name 'main.s')
    echo "$n"
}

# decorated build: `big` must be fully inlined (no surviving call).
rm -rf "$WORK/app/out"
if ! ( cd "$WORK/app" && "$MACH" build . --emit-asm -O2 ) >/dev/null 2>&1; then
    echo "FAIL inline: decorated build failed" >&2
    fail=1
else
    calls=$(count_big_calls)
    if [ "$calls" -ne 0 ]; then
        echo "FAIL inline: 'inline' decorator did not inline 'big' ($calls call(s) remain)" >&2
        fail=1
    else
        echo "PASS inline: 'inline' decorator forces inlining at -O2"
    fi
    set +e
    "$WORK/app/out/linux/bin/inldec"
    code=$?
    set -e
    if [ "$code" -ne 204 ]; then
        echo "FAIL inline: inlined program ran with exit $code, expected 204" >&2
        fail=1
    else
        echo "PASS inline: the inlined program runs correctly"
    fi
fi

# control: strip the `inline` decorator; the cost model must now leave the calls
# in place, proving the decorator — not the heuristic — is what inlined `big`.
# the `symbol("big")` pin is on its own line and survives, so `big` keeps its name.
sed '/`inline`/d' "$HERE/app/src/main.mach" > "$WORK/app/src/main.mach"
rm -rf "$WORK/app/out"
if ! ( cd "$WORK/app" && "$MACH" build . --emit-asm -O2 ) >/dev/null 2>&1; then
    echo "FAIL inline: control build failed" >&2
    fail=1
else
    calls=$(count_big_calls)
    if [ "$calls" -lt 1 ]; then
        echo "FAIL inline: control inlined 'big' without the decorator — test no longer isolates the hint" >&2
        fail=1
    else
        echo "PASS inline: without the decorator 'big' is left out of line"
    fi
fi

exit "$fail"
