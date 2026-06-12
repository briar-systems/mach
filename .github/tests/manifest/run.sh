#!/usr/bin/env bash
# manifest integration test: build a multi-artifact mach.toml project end to end.
#
# the project declares two binaries (hello, bye) sharing a module, two targets
# (linux, windows), and two profiles (debug, release with emit_ir). this suite
# proves the manifest reader drives a real build: per-artifact selection
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
        echo "FAIL manifest: $desc — binary not produced at $bin" >&2
        fail=1; return
    fi
    set +e
    "$bin"; local got=$?
    set -e
    if [ "$got" -ne "$want" ]; then
        echo "FAIL manifest: $desc — exit $got, expected $want" >&2
        fail=1; return
    fi
    echo "PASS manifest: $desc"
}

# expect_fail <desc> <build args...> — the build must fail.
expect_fail() {
    local desc="$1"; shift
    if ( cd "$APP" && "$MACH" build "$@" ) >/dev/null 2>&1; then
        echo "FAIL manifest: $desc — build unexpectedly succeeded" >&2
        fail=1; return
    fi
    echo "PASS manifest: $desc"
}

# default target (native -> linux) + default profile (debug), per-artifact.
( cd "$APP" && "$MACH" build . --bin hello )
( cd "$APP" && "$MACH" build . --bin bye )
expect_exit "native debug --bin hello" 7 "$APP/out/linux/debug/bin/hello"
expect_exit "native debug --bin bye"   9 "$APP/out/linux/debug/bin/bye"

# manifest defines (#1191) fold into the comptime context at each precedence
# level: tier exits 43 only when the target flag, artifact flag, and per-cell
# integer override of `tier` all reached `$if ($mach.build.<name>)`.
( cd "$APP" && "$MACH" build . --bin tier )
expect_exit "defines fold target<artifact<cell into \$mach.build.*" 43 "$APP/out/linux/debug/bin/tier"

# `mach build .` with no selector builds every declared artifact (the multi-
# artifact matrix), each to its own resolved path.
rm -rf "$APP/out/linux"
( cd "$APP" && "$MACH" build . )
expect_exit "matrix build . builds hello" 7  "$APP/out/linux/debug/bin/hello"
expect_exit "matrix build . builds bye"   9  "$APP/out/linux/debug/bin/bye"
expect_exit "matrix build . builds tier"  43 "$APP/out/linux/debug/bin/tier"

# --all-targets crosses every artifact with every declared target.
rm -rf "$APP/out"
( cd "$APP" && "$MACH" build . --all-targets )
expect_exit "all-targets builds the linux cell" 7 "$APP/out/linux/debug/bin/hello"
WEXE="$APP/out/windows/debug/bin/hello.exe"
if [ -f "$WEXE" ] && head -c2 "$WEXE" | grep -q MZ; then
    echo "PASS manifest: --all-targets builds the windows cell too"
else
    echo "FAIL manifest: --all-targets did not build the windows .exe" >&2; fail=1
fi

# release profile routes outputs to a separate directory (no debug overwrite).
( cd "$APP" && "$MACH" build . --bin hello --release )
expect_exit "native release --bin hello" 7 "$APP/out/linux/release/bin/hello"
if [ ! -x "$APP/out/linux/debug/bin/hello" ]; then
    echo "FAIL manifest: release build clobbered the debug binary" >&2; fail=1
else
    echo "PASS manifest: debug and release outputs coexist"
fi

# the release profile's emit_ir toggle writes per-module IR dumps.
if [ -f "$APP/out/linux/release/ir/demo/hello.ir" ]; then
    echo "PASS manifest: profile emit_ir writes per-module IR"
else
    echo "FAIL manifest: profile emit_ir produced no IR dump" >&2; fail=1
fi

# --no-emit-ir overrides the profile default off.
rm -rf "$APP/out/linux/release"
( cd "$APP" && "$MACH" build . --bin hello --release --no-emit-ir )
if [ -d "$APP/out/linux/release/ir" ]; then
    echo "FAIL manifest: --no-emit-ir did not override the profile default" >&2; fail=1
else
    echo "PASS manifest: --no-emit-ir overrides the profile emit_ir default"
fi

# --emit-asm writes per-module assembly even when the profile leaves it off.
( cd "$APP" && "$MACH" build . --bin bye --emit-asm )
if [ -f "$APP/out/linux/debug/asm/demo/bye.s" ]; then
    echo "PASS manifest: --emit-asm writes per-module assembly"
else
    echo "FAIL manifest: --emit-asm produced no assembly dump" >&2; fail=1
fi

# windows cross-compilation produces a PE executable at the .exe template path.
( cd "$APP" && "$MACH" build . --bin hello --target windows )
EXE="$APP/out/windows/debug/bin/hello.exe"
if [ -f "$EXE" ] && head -c2 "$EXE" | grep -q "MZ"; then
    echo "PASS manifest: windows cross-compile produces a PE .exe"
else
    echo "FAIL manifest: windows cross-compile produced no PE at $EXE" >&2; fail=1
fi

# loud failures: unknown artifact, unknown target/profile.
expect_fail "unknown bin selector" . --bin nope
expect_fail "unknown target selector" . --bin hello --target bsd
expect_fail "unknown profile selector" . --bin hello --profile fast

# mach run builds exactly one artifact to exec, so an ambiguous default must
# require --bin (naming the candidates) rather than silently picking one.
if ( cd "$APP" && "$MACH" run . ) >/dev/null 2>&1; then
    echo "FAIL manifest: ambiguous 'mach run .' did not require --bin" >&2; fail=1
else
    echo "PASS manifest: ambiguous 'mach run .' requires --bin"
fi

# a path collision (both bins to one fixed path) is rejected at build start.
COLL="$WORK/coll"
cp -r "$APP" "$COLL"
sed -i 's|^out     = .*|out     = "out/app"|' "$COLL/mach.toml"
if ( cd "$COLL" && "$MACH" build . --bin hello ) >/dev/null 2>&1; then
    echo "FAIL manifest: a path collision did not fail the build" >&2; fail=1
else
    echo "PASS manifest: colliding artifact paths fail at build start"
fi

# cascading transitive dep libs (#1218): the consumer declares no platform libs;
# its windows .exe must inherit kernel32.dll (from kdep) and user32.dll (from
# vdep) by target-tuple equality, while the native (linux) build inherits neither
# — proving tuple gating (a leak would fail the link with a DLL against a non-PE
# target).
CASC="$WORK/cascade"
cp -r "$HERE/cascade" "$CASC"
ln -s "$STD" "$CASC/dep/mach-std"

( cd "$CASC" && "$MACH" build . --target windows )
CEXE="$CASC/out/windows/debug/bin/cons.exe"
if [ -f "$CEXE" ] && strings "$CEXE" | grep -qi kernel32 && strings "$CEXE" | grep -qi user32; then
    echo "PASS manifest: windows .exe inherits kernel32 (kdep) + user32 (vdep) via cascade"
else
    echo "FAIL manifest: cascade did not import both dep libs into the windows .exe" >&2; fail=1
fi

if ( cd "$CASC" && "$MACH" build . ) >/dev/null 2>&1; then
    CBIN="$CASC/out/linux/debug/bin/cons"
    if [ -x "$CBIN" ] && ! strings "$CBIN" | grep -qiE "kernel32|user32"; then
        echo "PASS manifest: linux build inherits no windows-only dep libs (tuple gating)"
    else
        echo "FAIL manifest: a windows-only dep lib leaked into the linux build" >&2; fail=1
    fi
else
    echo "FAIL manifest: native cascade build failed (windows-only dep libs wrongly inherited?)" >&2; fail=1
fi

exit "$fail"
