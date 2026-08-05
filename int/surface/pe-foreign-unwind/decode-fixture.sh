#!/bin/sh
# Decode the checked-in clang fixture without requiring LLVM on the integration
# leg. Regenerate from qz.c with:
#   clang --target=x86_64-pc-windows-msvc -c -O2 -g0 -fno-ident -o qz.o qz.c
#   base64 -w 76 qz.o > qz.o.b64
# COFF records its compile timestamp, so regeneration is structurally equivalent
# rather than byte-identical.
set -eu
out=$1
mkdir -p "$(dirname "$out")"
base64 -d qz.o.b64 >"$out"
