#!/usr/bin/env bash
# path-dependency integration test: `mach dep pull` materializes a `[deps.*]` path
# entry as a relative symlink under `<dep>/<alias>`, the build resolves the dep's
# modules through it, and the documented failure modes are loud (#1370).
#
# the build needs the vendored std, staged as an in-tree symlink (offline); every
# other "dep" is a plain directory in the work tree, so the suite touches no
# network. it proves: materialization + build-and-call, no lock entry for a path
# dep, idempotent re-pull, and the missing-path / missing-manifest / real-directory
# hard errors.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
case "$MACH" in */*) MACH="$(cd "$(dirname "$MACH")" && pwd)/$(basename "$MACH")";; esac
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$HERE/../../.." && pwd)"
STD="$REPO_ROOT/dep/mach-std"

if [ ! -d "$STD/src" ]; then
    echo "error: vendored std not found at $STD (run: mach dep pull)" >&2
    exit 2
fi
if ! command -v ln >/dev/null 2>&1; then
    echo "error: ln not found on PATH (required by the pathdep suite)" >&2
    exit 2
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

fail=0
ok()   { echo "PASS pathdep: $1"; }
bad()  { echo "FAIL pathdep: $1" >&2; fail=1; }
has()  { if grep -qF -- "$2" "$3"; then ok "$1"; else bad "$1 (missing: $2)"; fi; }
hasnt(){ if grep -qF -- "$2" "$3"; then bad "$1 (unexpected: $2)"; else ok "$1"; fi; }

# the library every consumer depends on by path.
mkdir -p "$WORK/lib-x/src"
cat > "$WORK/lib-x/mach.toml" <<'TOML'
[project]
id      = "libx"
version = "0.1.0"
src     = "src"

[lib.libx]
entry = "lib.mach"
TOML
printf 'pub fun answer() i64 { ret 42; }\n' > "$WORK/lib-x/src/lib.mach"

# scaffold a fresh app that depends on lib-x by path; std is staged in place so the
# offline pull's in-tree std path dep is a no-op.
scaffold_app() {
    local dir="$1"
    mkdir -p "$dir/src" "$dir/dep"
    cat > "$dir/mach.toml" <<'TOML'
[project]
id      = "app"
version = "0.1.0"
src     = "src"
dep     = "dep"
target  = "native"

[target.linux]
isa = "x86_64"
os  = "linux"
abi = "sysv64"

[bin.app]
entry = "main.mach"

[profile.debug]
opt = 0

[deps.mach-std]
path = "dep/mach-std"

[deps.libx]
path = "../lib-x"
TOML
    cat > "$dir/src/main.mach" <<'TOML'
use std.runtime;
use libx.lib.answer;

`symbol("main")`
fun main(argc: i64, argv: **u8) i64 {
    ret answer();
}
TOML
    ln -s "$STD" "$dir/dep/mach-std"
}

OUT="$WORK/out.txt"

# --- materialize: pull links the path dep, no lock entry -----------------------
APP="$WORK/app"; scaffold_app "$APP"
( cd "$APP" && "$MACH" dep pull ) >"$OUT" 2>&1 || bad "pull exited nonzero"
[ -L "$APP/dep/libx" ] && ok "pull materializes the path dep as a symlink" || bad "pull did not create the dep/libx symlink"
[ -f "$APP/dep/libx/mach.toml" ] && ok "the symlink resolves to the lib's tree" || bad "dep/libx/mach.toml not reachable through the link"
if [ -f "$APP/mach.lock" ]; then
    hasnt "a path dep carries no lock entry" "[deps.libx]" "$APP/mach.lock"
else
    ok "a path dep carries no lock entry"
fi

# --- build: the app resolves the dep's modules through the symlink -------------
( cd "$APP" && "$MACH" build . ) >"$OUT" 2>&1 || bad "build through the symlink failed"
BIN="$APP/out/linux/debug/bin/app"
if [ -x "$BIN" ]; then
    set +e; "$BIN"; got=$?; set -e
    [ "$got" -eq 42 ] && ok "the built app calls into the path dep (exit 42)" || bad "app exit $got, expected 42"
else
    bad "app binary not produced at $BIN"
fi

# --- idempotent: a second pull keeps the link and builds -----------------------
( cd "$APP" && "$MACH" dep pull ) >"$OUT" 2>&1 || bad "re-pull exited nonzero"
[ -L "$APP/dep/libx" ] && ok "re-pull leaves the symlink in place" || bad "re-pull lost the symlink"
( cd "$APP" && "$MACH" build . ) >"$OUT" 2>&1 && [ -x "$BIN" ] && ok "the app still builds after re-pull" || bad "build after re-pull failed"

# --- error: a path pointing at a missing directory -----------------------------
MISS="$WORK/miss"; mkdir -p "$MISS"
printf '[project]\nid = "miss"\n\n[deps.libx]\npath = "../nope"\n' > "$MISS/mach.toml"
if ( cd "$MISS" && "$MACH" dep pull ) >"$OUT" 2>&1; then bad "a missing path dep did not fail"; else ok "a missing path dep is a hard error"; fi
has "the missing-path error names the directory" "points at a missing directory" "$OUT"

# --- error: a path with no mach.toml -------------------------------------------
NOM="$WORK/nomanifest"; mkdir -p "$NOM" "$WORK/bare/src"
printf '[project]\nid = "nm"\n\n[deps.libx]\npath = "../bare"\n' > "$NOM/mach.toml"
if ( cd "$NOM" && "$MACH" dep pull ) >"$OUT" 2>&1; then bad "a path dep without a manifest did not fail"; else ok "a path dep without mach.toml is a hard error"; fi
has "the no-manifest error names the cause" "has no mach.toml" "$OUT"

# --- error: a real directory squatting the vendor location ---------------------
SQ="$WORK/squat"; mkdir -p "$SQ/dep/libx/src"
printf '[project]\nid = "sq"\ndep = "dep"\n\n[deps.libx]\npath = "../lib-x"\n' > "$SQ/mach.toml"
printf '[project]\nid = "stale"\n' > "$SQ/dep/libx/mach.toml"
if ( cd "$SQ" && "$MACH" dep pull ) >"$OUT" 2>&1; then bad "a real dir at the vendor location did not fail"; else ok "a real dir at the vendor location is a hard error"; fi
has "the real-dir error names the path-dep link" "is a real directory" "$OUT"

exit "$fail"
