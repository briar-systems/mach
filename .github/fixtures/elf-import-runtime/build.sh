#!/usr/bin/env bash
set -eu
out=$1
provider=$(realpath "$out/libprovider.so")
cc -fPIC -shared provider.c -Wl,-soname,"$provider" -o "$out/libprovider.so"
cc -fPIC -O1 -c probe.c -o "$out/probe.o"
readelf -rW "$out/probe.o" > "$out/probe.relocs"
grep -E 'GOTPCREL.*got_value' "$out/probe.relocs"
grep -E 'PLT32.*got_value' "$out/probe.relocs"
grep -E 'GOTPCREL.*got_object' "$out/probe.relocs"
