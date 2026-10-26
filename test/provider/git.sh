#!/usr/bin/env bash
# local fixtures use Git; every compiler operation is then run without Git on PATH
set -euo pipefail
mach_bin=$(realpath "${MACH:-./m}")
case "$(uname -s)-$(uname -m)" in Linux-x86_64) ;; *) exit 77 ;; esac
fixture=$(mktemp -d)
trap 'result=$?; if [ "$result" = 0 ]; then rm -rf "$fixture"; else printf "fixture retained: %s\n" "$fixture" >&2; fi' EXIT
mkdir -p "$fixture/empty" "$fixture/lib/src" "$fixture/app/src"
export GIT_CONFIG_NOSYSTEM=1 GIT_CONFIG_GLOBAL=/dev/null
lib=$fixture/lib
app=$fixture/app
cat > "$lib/mach.toml" <<'MANIFEST'
[project]
id = "lib"
version = "1.0.0"
mach = "^6"
src = "src"
out = "out"
MANIFEST
printf 'pub fun answer() i64 { ret 42; }\n' > "$lib/src/value.mach"
printf 'out\n' > "$lib/.gitignore"
git -C "$lib" init -q -b main
git -C "$lib" -c user.name=fixture -c user.email=fixture@example.test add -A
git -C "$lib" -c user.name=fixture -c user.email=fixture@example.test commit -qm one
git -C "$lib" -c user.name=fixture -c user.email=fixture@example.test tag -am one v1.0.0
first=$(git -C "$lib" rev-parse HEAD)
cat > "$app/mach.toml" <<'MANIFEST'
[project]
id = "app"
version = "0.0.0"
mach = "^6"
src = "src"
out = "out/{target.name}/{profile.name}"
[target.host]
isa = "x86_64"
os = "linux"
abi = "sysv64"
[profile.debug]
default = true
opt = 0
debug = false
simd = "scalarize"
vectorize = true
float_reassoc = false
[artifact.app]
kind = "bin"
entry = "main.mach"
out = "app"
targets = ["*"]
link = []
need = []
MANIFEST
cat > "$app/src/main.mach" <<'MACH'
use lib.value;
#[symbol("_start")]
fun start() {
    val status: i64 = value.answer() - 42;
    asm x86_64 {
        mov rax, 60
        mov rdi, {status}
        syscall
    }
}
MACH
printf 'out\n' > "$app/.gitignore"
git -C "$app" init -q -b main
printf 'keep staged\n' > "$app/unrelated"
git -C "$app" add unrelated
saved=$(git -C "$app" ls-files --stage unrelated)
no_git() { env PATH="$fixture/empty" "$mach_bin" "$@"; }
no_git dep add "$app" lib --git "$lib" --version '^1.0'
test "$(git -C "$app/dep/lib" rev-parse HEAD)" = "$first"
test "$(git -C "$app" ls-files --stage unrelated)" = "$saved"
git -C "$app/dep/lib" fsck --full
no_git dep pull "$app"
no_git dep verify "$app"
no_git build "$app"
"$app/out/host/debug/app"
# Worktree edits must be rejected and retained.
printf '# dirty\n' >> "$app/dep/lib/src/value.mach"
if no_git dep verify "$app"; then echo 'dirty checkout accepted' >&2; exit 1; fi
printf 'pub fun answer() i64 { ret 42; }\n' > "$app/dep/lib/src/value.mach"
# Update a compatible release, then restore a deliberately drifted checkout offline.
aux=$fixture/aux
mkdir -p "$aux/src"
printf '[project]\nid="aux"\nversion="1.0.0"\nmach="^6"\nsrc="src"\nout="out"\n' > "$aux/mach.toml"
printf 'pub fun zero() i64 { ret 0; }\n' > "$aux/src/value.mach"
git -C "$aux" init -q -b main
git -C "$aux" add -A
git -C "$aux" -c user.name=fixture -c user.email=fixture@example.test commit -qm aux
git -C "$aux" tag v1.0.0
git -C "$lib" -c protocol.file.allow=always submodule add -q "$aux" dep/aux
printf '\n[dep.aux]\ngit = "%s"\nref = "tag/v1.0.0"\n' "$aux" >> "$lib/mach.toml"
sed -i 's/1.0.0/1.1.0/' "$lib/mach.toml"
# the dependency selects its unchanged exact tag, independently of lib's version
sed -i 's/tag\/v1.1.0/tag\/v1.0.0/' "$lib/mach.toml"
printf 'use aux.value;\npub fun answer() i64 { ret 43 + value.zero(); }\n' > "$lib/src/value.mach"
git -C "$lib" add -A
git -C "$lib" -c user.name=fixture -c user.email=fixture@example.test commit -qm two
git -C "$lib" -c user.name=fixture -c user.email=fixture@example.test tag -am two v1.1.0
second=$(git -C "$lib" rev-parse HEAD)
no_git dep update "$app" lib
test "$(git -C "$app/dep/lib" rev-parse HEAD)" = "$second"
test "$(git -C "$app" ls-files --stage unrelated)" = "$saved"
no_git dep verify "$app"
no_git check "$app"
test -f "$app/dep/aux/mach.toml"
test ! -f "$app/dep/lib/dep/aux/mach.toml"
git -C "$app/dep/lib" checkout -q --detach "$first"
no_git dep pull "$app"
test "$(git -C "$app/dep/lib" rev-parse HEAD)" = "$second"
# A lost work tree is restored from its module store with the source unavailable.
mv "$lib" "$lib.offline"
rm -rf "$app/dep/lib"
no_git dep pull "$app"
test "$(git -C "$app/dep/lib" rev-parse HEAD)" = "$second"
no_git dep verify "$app"
mv "$lib.offline" "$lib"
# Index locks are retained on refusal, without moving the work tree.
git -C "$app/dep/lib" checkout -q --detach "$first"
module_store=$(git -C "$app/dep/lib" rev-parse --absolute-git-dir)
printf 'locked\n' > "$module_store/index.lock"
if no_git dep pull "$app"; then echo 'index lock bypassed' >&2; exit 1; fi
test "$(cat "$module_store/index.lock")" = locked
test "$(git -C "$app/dep/lib" rev-parse HEAD)" = "$first"
rm "$module_store/index.lock"
no_git dep pull "$app"
# A running Git command failure must not silently select the native provider.
mkdir -p "$fixture/failing-git"
printf '#!/bin/sh\nexit 42\n' > "$fixture/failing-git/git"
chmod +x "$fixture/failing-git/git"
if env PATH="$fixture/failing-git" "$mach_bin" dep verify "$app"; then echo 'failed Git probe bypassed' >&2; exit 1; fi
# A working Git probe followed by a command failure retains that failure.
printf '#!/bin/sh\nif [ "$1" = --version ]; then exit 0; fi\nexit 42\n' > "$fixture/failing-git/git"
if env PATH="$fixture/failing-git" "$mach_bin" dep verify "$app" >"$fixture/git-error" 2>&1; then echo 'Git command failure bypassed' >&2; exit 1; fi
case "$(cat "$fixture/git-error")" in *'exited with status 42'*) ;; *) cat "$fixture/git-error" >&2; exit 1 ;; esac
# An executable that cannot launch selects the native provider.
printf 'invalid executable\n' > "$fixture/failing-git/git"
env PATH="$fixture/failing-git" "$mach_bin" dep verify "$app"
# Enclosing-repository gitlinks remain authoritative for a subproject.
outer=$fixture/outer
mkdir -p "$outer/nested/src"
sed '/^\[dep\./,$d' "$app/mach.toml" > "$outer/nested/mach.toml"
cp "$app/src/main.mach" "$outer/nested/src/main.mach"
printf 'out\n' > "$outer/nested/.gitignore"
git -C "$outer" init -q -b main
no_git dep add "$outer/nested" lib --git "$lib" --ref tag/v1.0.0
git -C "$outer" -c protocol.file.allow=always submodule add -q "$lib" nested/dep/lib
git -C "$outer/nested/dep/lib" fetch -q origin
git -C "$outer/nested/dep/lib" checkout -q --detach "$second"
no_git dep pull "$outer/nested"
test "$(git -C "$outer/nested/dep/lib" rev-parse HEAD)" = "$first"
no_git dep verify "$outer/nested"
no_git dep remove "$app" lib --purge
test ! -e "$app/dep/lib"
test -z "$(git -C "$app" ls-files --stage dep/lib)"
test "$(git -C "$app" ls-files --stage unrelated)" = "$saved"
printf 'native Git dependency workflows passed\n'
