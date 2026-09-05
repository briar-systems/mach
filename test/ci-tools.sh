#!/usr/bin/env bash
# ci-tools.sh: install the pinned external oracles a leg depends on.
#
# usage: ci-tools.sh <runner-label>
#
# the runner label is the assignment: engines.conf's runner column says which rows
# this machine owns, so a workflow names the runner it is already executing on and
# never a target list it could get wrong.
#
# the versions come from test/tools.lock and from nowhere else. the driver decides
# which rows a leg reaches (`run.sh --tools`) and tools.lock says what each row wants
# and where it is obtained, so a workflow carries no copy of a version and cannot
# install one thing while the driver demands another. #2948 was the shape without
# this: three legs refused to start because the runner images carry llvm 18 and
# spirv-tools 2025.1 and the workflow installed whatever apt had.
#
# a tool with no `source` row is one the runner already carries, and this script does
# not touch it: the driver's own check is what reports its absence, loudly, which is
# the behaviour a runner that cannot be brought to the pin must keep.
#
# installed binaries land in one directory that is prepended to GITHUB_PATH, so the
# pinned versions answer to their unsuffixed names - `llvm-objdump`, not
# `llvm-objdump-22` - which is what the driver probes and what a golden was blessed
# against.
set -euo pipefail

here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

[ $# -eq 1 ] || { echo "usage: ci-tools.sh <runner-label>" >&2; exit 2; }
runner=$1

bindir=${MACH_CI_TOOLS_BIN:-$here/../out/ci-tools/bin}
mkdir -p "$bindir"

case "$(uname -s)" in
    Linux)  host_os=linux ;;
    Darwin) host_os=darwin ;;
    *)      host_os=windows ;;
esac
host_arch=$(uname -m)
case "$host_arch" in
    arm64) host_arch=aarch64 ;;
    amd64) host_arch=x86_64 ;;
esac

# apt.llvm.org and the llvm-project release both serve the pinned major, so one
# install covers every llvm-* row a leg reaches rather than one per tool.
llvm_done=""
install_llvm() {
    local ver=$1
    local major=${ver%%.*}
    [ -z "$llvm_done" ] || return 0
    llvm_done=$ver

    if [ "$host_os" = linux ]; then
        # llvm.sh adds the apt.llvm.org repository for this distribution and installs
        # the versioned package set. both amd64 and arm64 are served.
        curl -fsSL -o "$bindir/llvm.sh" https://apt.llvm.org/llvm.sh
        chmod +x "$bindir/llvm.sh"
        sudo "$bindir/llvm.sh" "$major"
        # the unsuffixed names live in the version's own bin directory, so putting it
        # on PATH needs no symlink farm and shadows the image's older llvm cleanly.
        echo "/usr/lib/llvm-$major/bin" >> "${GITHUB_PATH:-/dev/null}"
        export PATH="/usr/lib/llvm-$major/bin:$PATH"
        return 0
    fi

    if [ "$host_os" = darwin ]; then
        # select the pinned major independently of homebrew's moving stable formula
        local formula="llvm@$major"
        brew install "$formula"
        local prefix
        prefix=$(brew --prefix "$formula")
        local got
        got=$("$prefix/bin/llvm-objdump" --version | grep -o 'LLVM version [0-9]*' | grep -o '[0-9]*$')
        if [ "$got" != "$major" ]; then
            echo "ci-tools.sh: $formula installed major $got but tools.lock pins $major" >&2
            exit 1
        fi
        echo "$prefix/bin" >> "${GITHUB_PATH:-/dev/null}"
        export PATH="$prefix/bin:$PATH"
        return 0
    fi

    if [ "$host_os" = windows ]; then
        # the release tools.lock names, fetched by name. nothing is resolved at run
        # time here: the version is in the lock, which is the only place it belongs.
        local dir="clang+llvm-$ver-x86_64-pc-windows-msvc"
        local url="https://github.com/llvm/llvm-project/releases/download/llvmorg-$ver/$dir.tar.xz"
        echo "ci-tools.sh: llvm $ver from $url"
        # to a file and then `tar -xf`: Git Bash's tar is not guaranteed to carry an
        # xz decompressor for `-J` on a stream, and bsdtar detects the compression
        # from the archive itself.
        curl -fsSL -o "$bindir/llvm.tar.xz" "$url"
        tar -xf "$bindir/llvm.tar.xz" -C "$bindir" --strip-components=2 \
            "$dir/bin/llvm-objdump.exe" "$dir/bin/llvm-readobj.exe" "$dir/bin/llvm-dwarfdump.exe"
        rm -f "$bindir/llvm.tar.xz"
        # GITHUB_PATH is read by the runner rather than by bash, so it takes a windows
        # path: an MSYS `/d/a/...` entry resolves for no step, including the bash ones.
        echo "$(cygpath -w "$bindir")" >> "${GITHUB_PATH:-/dev/null}"
        return 0
    fi

    echo "ci-tools.sh: no llvm $major delivery for $host_os" >&2
    exit 1
}

# the SDK is the only thing that serves this version at all: SPIRV-Tools tags no
# 2026.3 release and attaches binaries to none of its releases. tools.lock records
# why, and the linux SDK is x86_64 only, which is the bound engines.conf already
# reflects by giving the spirv row an x86_64 linux runner.
spirv_done=""
install_vulkan_sdk() {
    local ver=$1
    [ -z "$spirv_done" ] || return 0
    spirv_done=$ver

    if [ "$host_os" != linux ] || [ "$host_arch" != x86_64 ]; then
        echo "ci-tools.sh: vulkan sdk $ver serves linux x86_64 only, and this host is $host_os/$host_arch; the spirv column belongs to the runner engines.conf names for it" >&2
        exit 1
    fi

    local url="https://sdk.lunarg.com/sdk/download/$ver/linux/vulkansdk-linux-x86_64-$ver.tar.xz"
    echo "ci-tools.sh: spirv-tools from $url"
    curl -fsSL "$url" | tar -xJf - -C "$bindir" --strip-components=3 \
        "$ver/x86_64/bin/spirv-dis" "$ver/x86_64/bin/spirv-val"
    chmod +x "$bindir/spirv-dis" "$bindir/spirv-val"
    echo "$bindir" >> "${GITHUB_PATH:-/dev/null}"
}

while read -r name exe rule want provider handle; do
    [ -n "$name" ] || continue

    # the link suite reaches its C compiler through `${CC:-cc}`, so the alias
    # tools.lock records has to reach it too. exporting it here keeps the lock the one
    # place a host's spelling is written down, rather than a CC pinned in a workflow.
    if [ "$name" = cc ] && [ "$exe" != cc ]; then
        echo "CC=$exe" >> "${GITHUB_ENV:-/dev/null}"
        echo "ci-tools.sh: cc is spelled $exe on this host, per tools.lock"
    fi

    case "$provider" in
        -)          echo "ci-tools.sh: $name ($rule $want) comes with the runner as $exe" ;;
        llvm)       install_llvm "$handle" ;;
        vulkan-sdk) install_vulkan_sdk "$handle" ;;
        *)          echo "ci-tools.sh: tools.lock names provider '$provider' for $name, which this script does not serve" >&2; exit 1 ;;
    esac
done < <(bash "$here/run.sh" --tools --runner "$runner")

echo "ci-tools.sh: installed for the rows engines.conf assigns to $runner"
