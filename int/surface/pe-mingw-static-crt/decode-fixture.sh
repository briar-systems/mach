#!/bin/sh
# Decode real Zig/clang-produced MinGW COFF objects without requiring a C
# toolchain on the integration leg. Regenerate with a public Zig installation:
#   for src in options startup vsnprintf caller; do
#     zig cc -target x86_64-windows-gnu -c -O2 -g0 -fno-ident \
#       -o "$src.o" "$src.c"
#     base64 -w 76 "$src.o" >"$src.o.b64"
#   done
# No unwind-suppression flag is used: the foreign functions retain their normal
# `.pdata`/`.xdata`, which the PE linker must frame together with archive selection.
set -eu
out=$1
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

for src in options startup vsnprintf caller; do
    base64 -d "$src.o.b64" >"$tmp/$src.o"
done

# A COFF short import record for the UCRT helper used by the static vsnprintf.
# header: Sig1/Sig2, version, AMD64 machine, timestamp, data size, hint,
# code/by-name flags; data is symbol then DLL, both NUL-terminated.
sym=__stdio_common_vsprintf
dll=api-ms-win-crt-stdio-l1-1-0.dll
n=$(( ${#sym} + 1 + ${#dll} + 1 ))
printf '\000\000\377\377\000\000\144\206\000\000\000\000' >"$tmp/import.o"
printf "$(printf '\\%03o\\%03o\\%03o\\%03o' \
    $(( n & 255 )) $(( (n >> 8) & 255 )) $(( (n >> 16) & 255 )) $(( (n >> 24) & 255 )))" >>"$tmp/import.o"
printf '\000\000\004\000' >>"$tmp/import.o"
printf '%s\000%s\000' "$sym" "$dll" >>"$tmp/import.o"

append_member() {
    member=$1
    name=$2
    size=$(wc -c <"$member")
    printf '%-16s%-12s%-6s%-6s%-8s%-10s\140\n' "$name/" '0' '0' '0' '644' "$size" >>"$out"
    cat "$member" >>"$out"
    if [ $(( size % 2 )) -ne 0 ]; then printf '\n' >>"$out"; fi
}

mkdir -p "$(dirname "$out")"
printf '!<arch>\n' >"$out"
# The options dependency precedes the vsnprintf member that first requires it;
# caller comes later and is the initial root. Startup is deliberately unrelated.
append_member "$tmp/options.o" options.o
append_member "$tmp/startup.o" startup.o
append_member "$tmp/vsnprintf.o" vsnprintf.o
append_member "$tmp/caller.o" caller.o
append_member "$tmp/import.o" import.o
