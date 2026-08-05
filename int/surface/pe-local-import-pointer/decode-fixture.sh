#!/bin/sh
# Decode the checked-in Clang fixtures without requiring LLVM on the integration
# leg. Regenerate from this directory with:
#   for src in alias alias-two provider; do
#     clang --target=x86_64-pc-windows-msvc -c -O2 -g0 -fno-ident \
#       -fno-optimize-sibling-calls -fno-asynchronous-unwind-tables \
#       -o "$src.o" "$src.c"
#     base64 -w 76 "$src.o" >"$src.o.b64"
#   done
# The two alias objects carry duplicate IMAGE_REL_AMD64_REL32 references to
# `__imp_local_add`; provider.o defines local_add and directly calls it.
set -eu
out=$1
mkdir -p "$out"
base64 -d alias.o.b64 >"$out/alias.o"
base64 -d alias-two.o.b64 >"$out/alias-two.o"
base64 -d provider.o.b64 >"$out/provider.o"
