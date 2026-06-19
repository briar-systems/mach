#!/usr/bin/env bash
# attr integration test (#1532, #1526): prove the `#[attr]` decorator surface — the
# sole decorator form since v2.4.0 (backtick decorators removed) — parses and drives
# codegen across all five directives: symbol/inline/align/section on a linux object
# and library on a windows PE, with every directive composing on a single decl. also
# proves the lexer exception: a spaced `# [...]` line stays a comment, not an
# attribute (were `#` not de-special-cased before a space, that line would open an
# attribute and the build would fail).
#
# the linux app is built and run (must exit 42), then its `main.o` is inspected to
# confirm the attributes took effect — pinned symbols (`attr_g`/`attr_add`, unmangled)
# and named sections (`.attrtext`/`.attrdata`). the windows app cross-builds the
# `#[library(...)]`-routed imports into a PE (no run: the routing is structural).
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

# build APP_DIR — build a staged app, capturing output for diagnostics on failure.
build() {
    ( cd "$1" && "$MACH" build . ) >"$1/build.out" 2>&1
}

# --- linux app: symbol / inline / align / section ---
stage "$HERE/app" "$WORK/app"

if ! build "$WORK/app"; then
    echo "FAIL attr: attribute (#[...]) linux build failed" >&2
    cat "$WORK/app/build.out" >&2
    fail=1
else
    # the app carries a spaced `# [...]` comment; a clean build proves the lexer
    # kept it a comment rather than opening an attribute.
    echo "PASS attr: a #[...] app (with a spaced '# [...]' comment) builds clean"

    set +e
    "$WORK/app/out/linux/bin/attrlin"; ca=$?
    set -e
    if [ "$ca" -ne 42 ]; then
        echo "FAIL attr: linux runtime mismatch — exit $ca, expected 42" >&2
        fail=1
    else
        echo "PASS attr: the #[...] app runs (exit 42)"
    fi

    obj_a="$(find "$WORK/app/out" -name main.o | head -1)"
    # confirm the attributes actually took effect: `symbol` pins the link names
    # (unmangled `attr_g`/`attr_add`), `section` creates `.attrtext`/`.attrdata`.
    # skip the structural half if no ELF inspector is present rather than failing.
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

# --- windows app: library ---
# `library` is the windows-only PE import-routing directive. cross-build the app
# (no run: the routing is structural, no emulator needed) and require a clean build.
stage "$HERE/win" "$WORK/win"

if ! build "$WORK/win"; then
    echo "FAIL attr: attribute (#[...]) windows cross-build failed" >&2
    cat "$WORK/win/build.out" >&2
    fail=1
else
    exe_a="$WORK/win/out/windows/bin/attrwin.exe"
    if [ -f "$exe_a" ]; then
        echo "PASS attr: #[library(...)] imports cross-build into a PE"
    else
        echo "FAIL attr: windows build produced no PE at $exe_a" >&2
        fail=1
    fi
fi

exit "$fail"
