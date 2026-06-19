#!/usr/bin/env bash
# attr integration test (#1532): prove the `#[attr]` decorator surface parses and
# drives codegen identically to the backtick form across all five directives —
# symbol/inline/align/section on a linux object, and library on a windows PE —
# and that a spaced `# [...]` comment is still a comment, not an attribute.
#
# the backtick control for each app is *derived from the attr source by sed*
# (`#[name(args)]` -> `` `name(args)` ``), so the two trees differ only in the
# decorator delimiters; any divergence in an emitted artifact is therefore
# attributable to the surface alone. the linux pair is built and run (both must
# exit 42) and their `main.o` — code, custom sections, pinned symbols and
# relocations — compared byte-for-byte; the windows pair is cross-built and the
# emitted PEs compared. the attr app's `# [...]` line proves the lexer exception:
# were `#` not de-special-cased before a space, that comment would open an
# attribute and the build would fail.
#
# usage: run.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

MACH="${1:-mach}"
# absolutize a path argument: the suites cd into temp dirs, so a relative
# compiler path would break after the first cd.
case "$MACH" in */*) MACH="$(cd "$(dirname "$MACH")" && pwd)/$(basename "$MACH")";; esac
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$HERE/../../.." && pwd)"
STD="$REPO_ROOT/dep/mach-std"

if [ ! -d "$STD/src" ]; then
    echo "error: vendored std not found at $STD (run: mach dep pull)" >&2
    exit 2
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

fail=0

# stage SRC DST — copy an app tree to DST under WORK and wire its vendored std.
stage() {
    cp -r "$1" "$2"
    mkdir -p "$2/dep"
    ln -s "$STD" "$2/dep/mach-std"
}

# derive_control SRC DST — stage the backtick twin: identical to SRC except every
# `#[name(args)]` is rewritten to `` `name(args)` ``. spaced `# [...]` comments do
# not match `#\[` and are left untouched, so only the decorator surface differs.
derive_control() {
    stage "$1" "$2"
    sed -E 's/#\[([^][]*)\]/`\1`/g' "$1/src/main.mach" > "$2/src/main.mach"
}

# build APP_DIR — build a staged app, capturing output for diagnostics on failure.
build() {
    ( cd "$1" && "$MACH" build . ) >"$1/build.out" 2>&1
}

# --- linux pair: symbol / inline / align / section ---
stage "$HERE/app" "$WORK/app"
derive_control "$HERE/app" "$WORK/ctl"
sed -i 's/attrlin/attrctl/' "$WORK/ctl/mach.toml"

if ! build "$WORK/app"; then
    echo "FAIL attr: attribute (#[...]) linux build failed" >&2
    cat "$WORK/app/build.out" >&2
    fail=1
elif ! build "$WORK/ctl"; then
    echo "FAIL attr: backtick control linux build failed" >&2
    cat "$WORK/ctl/build.out" >&2
    fail=1
else
    # the attr app carries a spaced `# [...]` comment; a clean build proves the
    # lexer kept it a comment rather than opening an attribute.
    echo "PASS attr: a #[...] app (with a spaced '# [...]' comment) builds clean"

    set +e
    "$WORK/app/out/linux/bin/attrlin"; ca=$?
    "$WORK/ctl/out/linux/bin/attrctl"; cc=$?
    set -e
    if [ "$ca" -ne 42 ] || [ "$cc" -ne 42 ]; then
        echo "FAIL attr: linux runtime mismatch — attr exit $ca, backtick exit $cc, expected 42" >&2
        fail=1
    else
        echo "PASS attr: #[...] and backtick decls run identically (exit 42)"
    fi

    obj_a="$(find "$WORK/app/out" -name main.o | head -1)"
    obj_c="$(find "$WORK/ctl/out" -name main.o | head -1)"
    if [ -n "$obj_a" ] && [ -n "$obj_c" ] && cmp -s "$obj_a" "$obj_c"; then
        echo "PASS attr: #[...] and backtick emit a byte-identical object"
    else
        echo "FAIL attr: #[...] and backtick objects diverged" >&2
        fail=1
    fi

    # positive: confirm the attributes actually took effect, so the equivalence
    # above is not two no-ops matching. `symbol` pins the link names (unmangled
    # `attr_g`/`attr_add`), `section` creates `.attrtext`/`.attrdata`. skip the
    # structural half if no ELF inspector is present rather than failing.
    if command -v readelf >/dev/null 2>&1 && command -v nm >/dev/null 2>&1; then
        secs="$(readelf -S "$obj_a" 2>/dev/null | grep -oE '\.attr[a-z]+' | sort -u | tr '\n' ' ')"
        syms="$(nm "$obj_a" 2>/dev/null | grep -cE ' (attr_g|attr_add)$' || true)"
        if printf '%s' "$secs" | grep -q '.attrtext' && printf '%s' "$secs" | grep -q '.attrdata' && [ "$syms" -eq 2 ]; then
            echo "PASS attr: the symbol/section attributes took effect (sections: $secs)"
        else
            echo "FAIL attr: expected pinned symbols / named sections missing (sections: '$secs', symbols: $syms)" >&2
            fail=1
        fi
    else
        echo "INFO attr: no ELF inspector (readelf/nm); skipping the structural check"
    fi
fi

# --- windows pair: library ---
# `library` is the windows-only PE import-routing directive. cross-build the attr
# app and its backtick twin (no run: the PE comparison needs no emulator) and
# require the emitted binaries to be byte-identical.
stage "$HERE/win" "$WORK/win"
derive_control "$HERE/win" "$WORK/winctl"
sed -i 's/attrwin/attrwinctl/' "$WORK/winctl/mach.toml"

if ! build "$WORK/win"; then
    echo "FAIL attr: attribute (#[...]) windows cross-build failed" >&2
    cat "$WORK/win/build.out" >&2
    fail=1
elif ! build "$WORK/winctl"; then
    echo "FAIL attr: backtick control windows cross-build failed" >&2
    cat "$WORK/winctl/build.out" >&2
    fail=1
else
    exe_a="$WORK/win/out/windows/bin/attrwin.exe"
    exe_c="$WORK/winctl/out/windows/bin/attrwinctl.exe"
    if cmp -s "$exe_a" "$exe_c"; then
        echo "PASS attr: #[library(...)] and backtick emit a byte-identical PE"
    else
        echo "FAIL attr: #[library(...)] and backtick PEs diverged" >&2
        fail=1
    fi
fi

exit "$fail"
