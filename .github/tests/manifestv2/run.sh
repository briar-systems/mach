#!/usr/bin/env bash
# manifestv2 integration test: build a v2-format mach.toml project end to end.
#
# the project declares two binaries (hello, bye) sharing a module, two targets
# (linux, windows), and two profiles (debug, release with emit_ir). this suite
# proves the v2 manifest reader drives a real build: per-artifact selection
# (`--bin`), profile selection (`--release`) routing outputs to a separate dir,
# the profile's `emit_ir` toggle (and a `--no-emit-ir` override of it), windows
# cross-compilation, and the loud failures for an ambiguous default, an unknown
# artifact, and a path collision.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
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

cp -r "$HERE/app" "$WORK/app"
mkdir -p "$WORK/app/dep"
ln -s "$STD" "$WORK/app/dep/mach-std"
APP="$WORK/app"

fail=0

# expect_exit <desc> <expected-code> <binary> — run a built binary and check it.
expect_exit() {
    local desc="$1"; local want="$2"; local bin="$3"
    if [ ! -x "$bin" ]; then
        echo "FAIL manifestv2: $desc — binary not produced at $bin" >&2
        fail=1; return
    fi
    set +e
    "$bin"; local got=$?
    set -e
    if [ "$got" -ne "$want" ]; then
        echo "FAIL manifestv2: $desc — exit $got, expected $want" >&2
        fail=1; return
    fi
    echo "PASS manifestv2: $desc"
}

# expect_fail <desc> <build args...> — the build must fail.
expect_fail() {
    local desc="$1"; shift
    if ( cd "$APP" && "$MACH" build "$@" ) >/dev/null 2>&1; then
        echo "FAIL manifestv2: $desc — build unexpectedly succeeded" >&2
        fail=1; return
    fi
    echo "PASS manifestv2: $desc"
}

# default target (native -> linux) + default profile (debug), per-artifact.
( cd "$APP" && "$MACH" build . --bin hello )
( cd "$APP" && "$MACH" build . --bin bye )
expect_exit "native debug --bin hello" 7 "$APP/out/linux/debug/bin/hello"
expect_exit "native debug --bin bye"   9 "$APP/out/linux/debug/bin/bye"

# release profile routes outputs to a separate directory (no debug overwrite).
( cd "$APP" && "$MACH" build . --bin hello --release )
expect_exit "native release --bin hello" 7 "$APP/out/linux/release/bin/hello"
if [ ! -x "$APP/out/linux/debug/bin/hello" ]; then
    echo "FAIL manifestv2: release build clobbered the debug binary" >&2; fail=1
else
    echo "PASS manifestv2: debug and release outputs coexist"
fi

# the release profile's emit_ir toggle writes per-module IR dumps.
if [ -f "$APP/out/linux/release/ir/demo/hello.ir" ]; then
    echo "PASS manifestv2: profile emit_ir writes per-module IR"
else
    echo "FAIL manifestv2: profile emit_ir produced no IR dump" >&2; fail=1
fi

# --no-emit-ir overrides the profile default off.
rm -rf "$APP/out/linux/release"
( cd "$APP" && "$MACH" build . --bin hello --release --no-emit-ir )
if [ -d "$APP/out/linux/release/ir" ]; then
    echo "FAIL manifestv2: --no-emit-ir did not override the profile default" >&2; fail=1
else
    echo "PASS manifestv2: --no-emit-ir overrides the profile emit_ir default"
fi

# --emit-asm writes per-module assembly even when the profile leaves it off.
( cd "$APP" && "$MACH" build . --bin bye --emit-asm )
if [ -f "$APP/out/linux/debug/asm/demo/bye.s" ]; then
    echo "PASS manifestv2: --emit-asm writes per-module assembly"
else
    echo "FAIL manifestv2: --emit-asm produced no assembly dump" >&2; fail=1
fi

# windows cross-compilation produces a PE executable at the .exe template path.
( cd "$APP" && "$MACH" build . --bin hello --target windows )
EXE="$APP/out/windows/debug/bin/hello.exe"
if [ -f "$EXE" ] && head -c2 "$EXE" | grep -q "MZ"; then
    echo "PASS manifestv2: windows cross-compile produces a PE .exe"
else
    echo "FAIL manifestv2: windows cross-compile produced no PE at $EXE" >&2; fail=1
fi

# loud failures: ambiguous default, unknown artifact, unknown target/profile.
expect_fail "ambiguous default (two bins, no selector)" .
expect_fail "unknown bin selector" . --bin nope
expect_fail "unknown target selector" . --bin hello --target bsd
expect_fail "unknown profile selector" . --bin hello --profile fast

# a path collision (both bins to one fixed path) is rejected at build start.
COLL="$WORK/coll"
cp -r "$APP" "$COLL"
sed -i 's|^out     = .*|out     = "out/app"|' "$COLL/mach.toml"
if ( cd "$COLL" && "$MACH" build . --bin hello ) >/dev/null 2>&1; then
    echo "FAIL manifestv2: a path collision did not fail the build" >&2; fail=1
else
    echo "PASS manifestv2: colliding artifact paths fail at build start"
fi

exit "$fail"
