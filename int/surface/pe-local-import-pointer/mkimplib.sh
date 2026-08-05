#!/bin/sh
# Build one short-import archive selected by __imp_local_add before provider.o is
# encountered. The later strong definition makes this declaration inert.
set -eu
out=$1
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

sym=local_add
dll=demo.dll
n=$(( ${#sym} + 1 + ${#dll} + 1 ))
printf '\000\000\377\377\000\000\144\206\000\000\000\000' >"$tmp/imp"
printf "$(printf '\\%03o\\%03o\\%03o\\%03o' \
    $(( n & 255 )) $(( (n >> 8) & 255 )) $(( (n >> 16) & 255 )) $(( (n >> 24) & 255 )))" >>"$tmp/imp"
printf '\000\000\004\000' >>"$tmp/imp"
printf '%s\000%s\000' "$sym" "$dll" >>"$tmp/imp"

mkdir -p "$(dirname "$out")"
size=$(wc -c <"$tmp/imp")
printf '!<arch>\n' >"$out"
printf '%-16s%-12s%-6s%-6s%-8s%-10s\140\n' \
    'imp.o/' '0' '0' '0' '644' "$size" >>"$out"
cat "$tmp/imp" >>"$out"
if [ $(( size % 2 )) -ne 0 ]; then printf '\n' >>"$out"; fi
