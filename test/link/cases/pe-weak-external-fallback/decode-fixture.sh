#!/bin/sh
# Decode a real Zig/clang-produced MinGW COFF object without requiring a C
# toolchain on the integration leg. Regenerate with a public Zig installation:
#   zig cc -target x86_64-windows-gnu -c -O2 -g0 -fno-ident -ffunction-sections \
#     -o runtime.o runtime.c
#   base64 -w 76 runtime.o >runtime.o.b64
set -eu
out=$1
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

base64 -d runtime.o.b64 >"$tmp/runtime.o"
size=$(wc -c <"$tmp/runtime.o")

mkdir -p "$(dirname "$out")"
printf '!<arch>\n' >"$out"
printf '%-16s%-12s%-6s%-6s%-8s%-10s\140\n' 'runtime.o/' '0' '0' '0' '644' "$size" >>"$out"
cat "$tmp/runtime.o" >>"$out"
if [ $(( size % 2 )) -ne 0 ]; then printf '\n' >>"$out"; fi
