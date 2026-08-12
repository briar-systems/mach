#!/usr/bin/env sh
# prove a source-vendored frontend retains Mach's version, not its host's
set -eu

cc=${1:-}
root=${2:-.}
if [ -z "$cc" ]; then
    echo "usage: version-vendor.sh <compiler> [mach-root]" >&2
    exit 2
fi

cc=$(realpath "$cc")
root=$(realpath "$root")
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
mkdir -p "$work/src"

case "$(uname -m)" in
    x86_64)  isa=x86_64; abi=sysv64 ;;
    aarch64) isa=aarch64; abi=aapcs64 ;;
    *) echo "version-vendor.sh: unsupported host architecture" >&2; exit 2 ;;
esac

cat > "$work/mach.toml" <<EOF
[project]
id = "mach-version-vendor-test"
version = "mach-version-vendor-test"
src = "src"
out = "out/{target.name}/{profile.name}"

[target.host]
isa = "$isa"
os = "linux"
abi = "$abi"

[profile.debug]
opt = 0
debug = false
simd = "scalarize"

[artifact.host]
kind = "static"
entry = "main.mach"
out = "lib/host"
targets = ["*"]
link = []
need = []

[dep.mach]
path = "$root"

[dep.mach-std]
git = "https://github.com/briar-systems/mach-std"
ref = "branch/main"
EOF

cat > "$work/src/main.mach" <<'EOF'
use std.runtime;
use mach.lang.version;

#[symbol("main")]
fun main() i32 { ret 0; }
EOF

cd "$work"
"$cc" dep pull
"$cc" test . --include-deps --filter mach.lang.version
