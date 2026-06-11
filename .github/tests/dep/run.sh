#!/usr/bin/env bash
# dep integration test: drive `mach dep` end to end against local git repos.
#
# every "remote" is a throwaway `git init` repo under the work dir, so the suite
# is fully offline (the network-shaped commands hit file:// urls only). it proves
# the #1223 command surface: transitive clone into the flat dep tree, idempotent
# pull, loud checkout-drift repair and ref re-resolution, the only-lock-advancer
# `update`, the same-name conflict and vendored-shape hard errors, add/remove, and
# pull of a legacy v1 manifest (url/version keys).
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
case "$MACH" in */*) MACH="$(cd "$(dirname "$MACH")" && pwd)/$(basename "$MACH")";; esac

if ! command -v git >/dev/null 2>&1; then
    echo "error: git not found on PATH (required by the dep suite)" >&2
    exit 2
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
export GIT_AUTHOR_NAME=t GIT_AUTHOR_EMAIL=t@t GIT_COMMITTER_NAME=t GIT_COMMITTER_EMAIL=t@t
export GIT_CONFIG_GLOBAL=/dev/null GIT_CONFIG_SYSTEM=/dev/null

fail=0
ok()   { echo "PASS dep: $1"; }
bad()  { echo "FAIL dep: $1" >&2; fail=1; }
# has <desc> <needle> <file> — assert the file contains the needle.
has()  { if grep -qF -- "$2" "$3"; then ok "$1"; else bad "$1 (missing: $2)"; fi; }
hasnt(){ if grep -qF -- "$2" "$3"; then bad "$1 (unexpected: $2)"; else ok "$1"; fi; }

# mkrepo <name> <id> [deps-block] — a one-commit git repo acting as a remote.
mkrepo() {
    local d="$WORK/remotes/$1"; mkdir -p "$d/src"
    ( cd "$d"
      git init -q -b main
      printf '[project]\nid = "%s"\n%s' "$2" "${3:-}" > mach.toml
      printf '# %s\n' "$1" > src/lib.mach
      git add -A && git commit -qm init )
}

mkrepo leaf  leaf
mkrepo mid   mid "[deps.leaf]
git = \"file://$WORK/remotes/leaf\"
ref = \"branch/main\"
"

# --- pull: fresh, transitive into the flat tree ---------------------------------
APP="$WORK/app"; mkdir -p "$APP"
cat > "$APP/mach.toml" <<TOML
[project]
id = "app"
dep = "dep"

[deps.mid]
git = "file://$WORK/remotes/mid"
ref = "branch/main"
TOML

OUT="$WORK/out.txt"
( cd "$APP" && "$MACH" dep pull ) >"$OUT" 2>&1 || bad "pull exited nonzero"
[ -d "$APP/dep/mid" ]  && ok "pull clones the direct dep"      || bad "pull did not clone dep/mid"
[ -d "$APP/dep/leaf" ] && ok "pull clones the transitive dep into the flat tree" || bad "pull did not clone dep/leaf"
has "pull writes a lock entry for the direct dep"     "[deps.mid]"  "$APP/mach.lock"
has "pull writes a lock entry for the transitive dep" "[deps.leaf]" "$APP/mach.lock"

# --- pull: idempotent ----------------------------------------------------------
( cd "$APP" && "$MACH" dep pull ) >"$OUT" 2>&1
has "second pull is idempotent" "mach.lock up to date" "$OUT"

# --- drift: a moved checkout is repaired loudly --------------------------------
( cd "$WORK/remotes/mid" && echo x >> src/lib.mach && git commit -qam c2 )
NEW="$(git -C "$WORK/remotes/mid" rev-parse HEAD)"
git -C "$APP/dep/mid" fetch -q origin
git -C "$APP/dep/mid" checkout -q --detach "$NEW"
( cd "$APP" && "$MACH" dep pull ) >"$OUT" 2>&1
has "pull repairs checkout drift loudly" "repaired mid (checkout drift:" "$OUT"
LOCKED="$(git -C "$APP/dep/mid" rev-parse HEAD)"
LREC="$(awk '/\[deps.mid\]/{f=1} f&&/commit/{print $3; exit}' "$APP/mach.lock" | tr -d '"')"
[ "$LOCKED" = "$LREC" ] && ok "repaired checkout matches the lock commit" || bad "drift not repaired to lock ($LOCKED != $LREC)"

# --- re-resolve: a changed manifest ref is re-resolved loudly ------------------
git -C "$WORK/remotes/mid" tag v1.0.0
sed -i 's|ref = "branch/main"|ref = "tag/v1.0.0"|' "$APP/mach.toml"
( cd "$APP" && "$MACH" dep pull ) >"$OUT" 2>&1
has "pull re-resolves a changed ref loudly" "re-resolved mid (manifest ref changed: branch/main → tag/v1.0.0)" "$OUT"
has "the lock records the new ref" 'ref = "tag/v1.0.0"' "$APP/mach.lock"

# --- update: the only lock-advancer --------------------------------------------
# mid is now pinned to a tag (immutable); leaf tracks a branch.
OLD_LEAF="$(awk '/\[deps.leaf\]/{f=1} f&&/commit/{print $3; exit}' "$APP/mach.lock" | tr -d '"')"
( cd "$WORK/remotes/leaf" && echo y >> src/lib.mach && git commit -qam b2 )
( cd "$APP" && "$MACH" dep update --all ) >"$OUT" 2>&1
has "update advances a tracked branch to the new tip" "leaf: $OLD_LEAF → " "$OUT"
NEW_LEAF="$(git -C "$WORK/remotes/leaf" rev-parse HEAD)"
has "update writes the advanced commit to the lock" "$NEW_LEAF" "$APP/mach.lock"
( cd "$APP" && "$MACH" dep update mid ) >"$OUT" 2>&1
if grep -qE 'mid: ([0-9a-f]+) → \1' "$OUT"; then ok "update is a no-op on an immutable tag ref"; else bad "update changed a tag-pinned dep"; fi

# --- conflict: same name, different refs ---------------------------------------
mkrepo dep_a a "[deps.shared]
git = \"file://$WORK/remotes/leaf\"
ref = \"branch/main\"
"
CON="$WORK/con"; mkdir -p "$CON"
cat > "$CON/mach.toml" <<TOML
[project]
id = "con"

[deps.a]
git = "file://$WORK/remotes/dep_a"
ref = "branch/main"

[deps.shared]
git = "file://$WORK/remotes/leaf"
ref = "tag/v1.0.0"
TOML
git -C "$WORK/remotes/leaf" tag v1.0.0
if ( cd "$CON" && "$MACH" dep pull ) >"$OUT" 2>&1; then bad "conflicting deps did not fail"; else ok "conflicting same-name deps fail the pull"; fi
has "the conflict names both requirers" "conflicting sources or refs" "$OUT"

# --- vendored-shape: a dep dir without .git is a hard error --------------------
VEN="$WORK/ven"; mkdir -p "$VEN/dep/v/src"
cat > "$VEN/mach.toml" <<TOML
[project]
id = "ven"

[deps.v]
git = "file://$WORK/remotes/leaf"
ref = "branch/main"
TOML
printf '[project]\nid = "v"\n' > "$VEN/dep/v/mach.toml"
if ( cd "$VEN" && "$MACH" dep pull ) >"$OUT" 2>&1; then bad "vendored-shape dir did not fail"; else ok "a non-checkout dep dir is a hard error"; fi
has "the vendored-shape error names the path-dep fix" "declare it a path dependency" "$OUT"

# --- add / remove --------------------------------------------------------------
AD="$WORK/add"; mkdir -p "$AD/vendored/src"
printf '[project]\nid = "add"\n' > "$AD/mach.toml"
printf '[project]\nid = "vd"\n' > "$AD/vendored/mach.toml"
( cd "$AD" && "$MACH" dep add foo --git "file://$WORK/remotes/leaf" --ref tag/v1.0.0 ) >"$OUT" 2>&1
has "add writes a v2 git stanza" 'git = "file://' "$AD/mach.toml"
( cd "$AD" && "$MACH" dep add vd --path vendored ) >"$OUT" 2>&1
has "add writes a v2 path stanza" 'path = "vendored"' "$AD/mach.toml"
( cd "$AD" && "$MACH" dep remove foo --purge ) >"$OUT" 2>&1
hasnt "remove drops the manifest entry" "[deps.foo]" "$AD/mach.toml"
[ ! -d "$AD/dep/foo" ] && ok "remove --purge deletes the dep dir" || bad "remove --purge left dep/foo"

# --- v1 manifest pull (legacy url/version keys) --------------------------------
V1="$WORK/v1"; mkdir -p "$V1"
cat > "$V1/mach.toml" <<TOML
[project]
id = "v1"
dir_dep = "dep"

[targets.linux]
os = "linux"
isa = "x86_64"

[deps.leaf]
type = "remote"
path = "file://$WORK/remotes/leaf"
version = "branch/main"
TOML
( cd "$V1" && "$MACH" dep pull ) >"$OUT" 2>&1 || bad "v1 pull exited nonzero"
[ -d "$V1/dep/leaf" ] && ok "v1 manifest pull clones via url/version keys" || bad "v1 pull did not clone"
if ( cd "$V1" && "$MACH" dep add p --path foo ) >"$OUT" 2>&1; then bad "add --path on v1 did not fail"; else ok "add --path is rejected on a v1 manifest"; fi

exit "$fail"
