#!/bin/sh
# Decode the checked-in clang fixture without requiring LLVM on the integration
# leg. Regenerate from this directory with:
#   clang --target=x86_64-pc-windows-msvc -c -O0 -g -gcodeview -fno-ident \
#     -falign-functions=8192 -fdebug-compilation-dir=. -o codeview.o codeview.c
#   base64 -w 76 codeview.o > codeview.o.b64
# COFF records its compile timestamp, so regeneration is structurally equivalent
# rather than byte-identical. `.debug$S` carries two real SECREL+SECTION pairs
# against codeview_answer; the deliberately over-aligned text also proves the
# linked value includes PE's prefix before the linker's first content byte.
set -eu
out=$1
mkdir -p "$(dirname "$out")"
base64 -d codeview.o.b64 >"$out"
