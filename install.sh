#!/bin/sh
# installs the mach release binary.
# usage: curl -fsSL https://github.com/octalide/mach/releases/latest/download/install.sh | sh
#
# MACH_VERSION      version to install (e.g. 1.4.2); defaults to the latest release
# MACH_INSTALL_DIR  install directory; defaults to ~/.local/bin
# MACH_BASE_URL     release base url override (for testing)
set -eu

base="${MACH_BASE_URL:-https://github.com/octalide/mach/releases}"
dir="${MACH_INSTALL_DIR:-$HOME/.local/bin}"
version="${MACH_VERSION:-${1:-}}"

err() { echo "install.sh: $*" >&2; exit 1; }

command -v curl >/dev/null || err "curl is required"
command -v sha256sum >/dev/null || err "sha256sum is required"

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64) target="x86_64-linux" ;;
    *) err "unsupported host $(uname -s)-$(uname -m); prebuilt binaries: https://github.com/octalide/mach/releases" ;;
esac

if [ -n "$version" ]; then
    case "$version" in
        v*) tag="$version" ;;
        *)  tag="v$version" ;;
    esac
else
    tag="$(curl -fsSI "$base/latest" | tr -d '\r' | grep -i '^location:' | sed 's|.*/tag/||')"
    [ -n "$tag" ] || err "could not resolve the latest release tag"
fi
version="${tag#v}"

archive="mach-$version-$target.tar.gz"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

echo "downloading $archive ($tag)"
curl -fsSL -o "$tmp/$archive" "$base/download/$tag/$archive"
curl -fsSL -o "$tmp/SHA256SUMS" "$base/download/$tag/SHA256SUMS"

grep " $archive\$" "$tmp/SHA256SUMS" > "$tmp/SHA256SUMS.want" || err "no checksum for $archive in SHA256SUMS"
( cd "$tmp" && sha256sum -c SHA256SUMS.want >/dev/null 2>&1 ) || err "checksum verification FAILED for $archive; aborting"

tar -xzf "$tmp/$archive" -C "$tmp" mach
mkdir -p "$dir"
install -m 755 "$tmp/mach" "$dir/mach"
echo "installed mach $version to $dir/mach"

case ":$PATH:" in
    *":$dir:"*) ;;
    *) echo "note: $dir is not on PATH; add it to your shell profile" ;;
esac
