#!/usr/bin/env bash
# the release installer needs its download/archive tools, with no Git executable
set -euo pipefail
repo=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
case "$(uname -s)-$(uname -m)" in Linux-x86_64) ;; *) exit 77 ;; esac
fixture=$(mktemp -d)
trap 'rm -rf "$fixture"' EXIT
release=$fixture/releases/download/v0.0.0
mkdir -p "$release" "$fixture/tools" "$fixture/payload"
printf '#!/bin/sh\nexit 0\n' > "$fixture/payload/mach"
archive=mach-0.0.0-x86_64-linux.tar.gz
tar -czf "$release/$archive" -C "$fixture/payload" mach
(cd "$release" && sha256sum "$archive" > SHA256SUMS)
for tool in curl uname mktemp cat awk sha256sum tar gzip mkdir install rm; do
    ln -s "$(command -v "$tool")" "$fixture/tools/$tool"
done
if ! env PATH="$fixture/tools" MACH_VERSION=0.0.0 MACH_BASE_URL="file://$fixture/releases" MACH_INSTALL_DIR="$fixture/installed" /bin/sh "$repo/dist/install.sh" > "$fixture/log" 2>&1; then
    cat "$fixture/log" >&2
    exit 1
fi
test -x "$fixture/installed/mach"
cmp "$fixture/payload/mach" "$fixture/installed/mach"
printf 'installer without Git passed\n'
