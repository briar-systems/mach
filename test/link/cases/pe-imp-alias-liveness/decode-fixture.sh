#!/bin/sh
# Decode the checked-in Clang fixture without requiring LLVM on the integration
# leg. Regenerate from this directory with:
#   clang --target=x86_64-pc-windows-msvc -c -O2 -g0 -fno-ident \
#     -o qz.o qz.c
#   base64 -w 76 qz.o > qz.o.b64
# alias_probe_indirect carries a real IMAGE_REL_AMD64_REL32 dllimport reference to
# `__imp_AliasProbe`; nothing in this object or in src/main.mach names
# `AliasProbe` itself.
set -eu
out=$1
mkdir -p "$(dirname "$out")"
base64 -d qz.o.b64 >"$out"
