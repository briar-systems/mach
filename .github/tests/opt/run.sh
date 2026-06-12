#!/usr/bin/env bash
# profile opt-level integration test: confirm the selected profile's `opt` drives
# the build's optimization level and that an explicit CLI `-O*` flag overrides it.
#
# the `app` project's default profile declares `opt = 2`. building it three ways —
# no flag (profile default), forced `-O2`, forced `-O0` — must produce a binary
# that (1) runs correctly in every case, (2) is byte-identical between the profile
# default and explicit `-O2` (the profile opt-level IS the default), and
# (3) differs under `-O0` (the CLI flag overrides the manifest). a bogus opt
# value must be rejected as a manifest error.
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

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp -r "$HERE/app" "$WORK/"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"

fail=0

# build_into <out-binary> <extra build args...>
build_into() {
    local out="$1"; shift
    ( cd "$WORK/app" && "$MACH" build . -o "$out" "$@" ) >/dev/null 2>&1
}

# profile default (opt = 2), forced -O2, forced -O0.
if ! build_into out/manifest_bin; then
    echo "FAIL opt: profile opt=2 build failed" >&2; fail=1
fi
if ! build_into out/o2_bin -O2; then
    echo "FAIL opt: -O2 build failed" >&2; fail=1
fi
if ! build_into out/o0_bin -O0; then
    echo "FAIL opt: -O0 build failed" >&2; fail=1
fi

# every variant must run and exit 0 (result is opt-level-independent).
for bin in manifest_bin o2_bin o0_bin; do
    set +e
    "$WORK/app/out/$bin"
    code=$?
    set -e
    if [ "$code" -ne 0 ]; then
        echo "FAIL opt: $bin ran with exit $code, expected 0" >&2; fail=1
    fi
done

# the profile opt-level IS the default: it must match explicit -O2 byte-for-byte.
if cmp -s "$WORK/app/out/manifest_bin" "$WORK/app/out/o2_bin"; then
    echo "PASS opt: profile opt=2 produces the -O2 binary"
else
    echo "FAIL opt: profile opt=2 binary differs from -O2" >&2; fail=1
fi

# an explicit CLI flag overrides the manifest: -O0 must differ from opt=2.
if cmp -s "$WORK/app/out/manifest_bin" "$WORK/app/out/o0_bin"; then
    echo "FAIL opt: -O0 did not override profile opt=2 (binaries identical)" >&2; fail=1
else
    echo "PASS opt: -O0 overrides profile opt=2"
fi

# a bogus opt value is a manifest error.
sed 's/^opt = 2/opt = 9/' "$HERE/app/mach.toml" > "$WORK/app/mach.toml"
rm -rf "$WORK/app/out"
if ( cd "$WORK/app" && "$MACH" build . ) >/dev/null 2>&1; then
    echo "FAIL opt: invalid opt value did not fail the build" >&2; fail=1
else
    echo "PASS opt: invalid opt value is a manifest error"
fi
cp "$HERE/app/mach.toml" "$WORK/app/mach.toml"

exit "$fail"
