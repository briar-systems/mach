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
compiler_ver=$(sed -n 's/^pub val MACH_VERSION: str = "\(.*\)";$/\1/p' "$root/src/lang/version.mach")
if [ -z "$compiler_ver" ]; then
    echo "version-vendor.sh: compiler version constant not found" >&2
    exit 2
fi
compiler_major=${compiler_ver%%.*}
if [ "$("$cc" info --version)" != "$compiler_ver" ]; then
    echo "version-vendor.sh: vendored CLI reports the embedding project version" >&2
    exit 1
fi
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
debug = true
simd = "scalarize"

[artifact.host]
kind = "bin"
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

cat > "$work/src/main.mach" <<EOF
use std.runtime;
use std.types.string.str_equals;
use std.types.size.usize;
use std.types.string.str;
use mach.cli.cmd.dispatch;
use mach.lang.driver.load;
use mach.lang.version;

test "mach.lang.version:vendored_context" {
    if (!str_equals(load.MACH_VERSION, "$compiler_ver")) { ret 1; }
    \$if (\$mach.version != "$compiler_ver") { ret 1; }
    \$if (\$mach.version.major != $compiler_major) { ret 1; }
    ret 0;
}

#[symbol("main")]
fun main(argc: usize, argv: *str) i64 { ret dispatch(argc, argv); }
EOF

cd "$work"
"$cc" dep pull
vendored_cli="$work/vendored-mach"
"$cc" build . -o "$vendored_cli"
if [ "$("$vendored_cli" info --version)" != "$compiler_ver" ]; then
    echo "version-vendor.sh: built vendored CLI reports the embedding project version" >&2
    exit 1
fi
if ! readelf --debug-dump=info "$vendored_cli" 2>/dev/null | grep -Fq "DW_AT_producer"; then
    echo "version-vendor.sh: vendored CLI has no debug producer metadata" >&2
    exit 1
fi
if ! readelf --debug-dump=info "$vendored_cli" 2>/dev/null | grep -F "DW_AT_producer" | grep -Fq "mach $compiler_ver"; then
    echo "version-vendor.sh: vendored debug producer reports the embedding project version" >&2
    exit 1
fi
"$cc" test . --include-deps --filter mach.lang.version
