#!/bin/sh
# installs the mach release binary.
#
# MACH_VERSION      version to install (e.g. 1.2.3); defaults to the latest release
# MACH_INSTALL_DIR  install directory; set to skip the prompt; defaults to ~/.local/bin
# MACH_BASE_URL     release base url override (for testing)

set -eu

err() { echo "install.sh: $*" >&2; exit 1; }

main() {
    base="${MACH_BASE_URL:-https://github.com/briar-systems/mach/releases}"

    command -v curl >/dev/null || err "curl is required"

    case "$(uname -s)-$(uname -m)" in
        Linux-x86_64) target="x86_64-linux" ;;
        Linux-aarch64|Linux-arm64) target="aarch64-linux" ;;
        *) err "unsupported host $(uname -s)-$(uname -m); prebuilt binaries: https://github.com/briar-systems/mach/releases" ;;
    esac

    # mach magenta (0xff00ff) on a tty
    if [ -t 1 ]; then
        esc=$(printf '\033')
        c="${esc}[38;2;255;0;255m"; r="${esc}[0m"
    else
        c=''; r=''
    fi

    # resolve the version to install: pinned via MACH_VERSION/$1, else the latest tag
    version="${MACH_VERSION:-${1:-}}"
    if [ -n "$version" ]; then
        case "$version" in v*) tag="$version" ;; *) tag="v$version" ;; esac
    else
        tag="$(curl -fsSI "$base/latest" | tr -d '\r' | grep -i '^location:' | sed 's|.*/tag/||')"
        [ -n "$tag" ] || err "could not resolve the latest release tag"
    fi
    version="${tag#v}"

    printf '\n%s' "$c"
    cat <<'EOF'
                        _
  _ __ ___    __ _  ___| |__
 | '_ ` _ \  / _` |/ __| '_ \
 | | | | | || (_| | (__| | | |
 |_| |_| |_| \__,_|\___|_| |_|
EOF
    printf '%s' "$r"
    printf '  mach %s (%s)\n\n' "$version" "$target"

    # install directory: explicit env override wins; otherwise prompt at a terminal,
    # falling back to the default when piped or non-interactive (e.g. CI)
    if [ -n "${MACH_INSTALL_DIR:-}" ]; then
        dir="$MACH_INSTALL_DIR"
    else
        default="$HOME/.local/bin"
        if [ -t 1 ]; then
            printf 'install directory [%s]: ' "$default"
            IFS= read -r dir < /dev/tty || dir=''
        else
            dir=''
        fi
        [ -n "$dir" ] || dir="$default"
        case "$dir" in
            "~")   dir="$HOME" ;;
            "~/"*) dir="$HOME/${dir#"~/"}" ;;
        esac
    fi

    archive="mach-$version-$target.tar.gz"
    tmp="$(mktemp -d)"
    trap 'rm -rf "$tmp"' EXIT

    printf '\ndownloading %s...\n' "$archive"
    curl -fsSL -o "$tmp/$archive" "$base/download/$tag/$archive"

    printf 'extracting %s...\n' "$archive"
    tar -xzf "$tmp/$archive" -C "$tmp" mach
    mkdir -p "$dir"
    install -m 755 "$tmp/mach" "$dir/mach"
    printf '\ninstalled mach %s to %s\n' "$version" "$dir/mach"
}

main "$@"
