#!/bin/sh
# build an import library the way llvm-dlltool would, using coreutils alone so the
# case needs no LLVM on the leg. each member is a COFF short import record
# (IMPORT_OBJECT_HEADER): Sig1=0, Sig2=0xFFFF, Version=0, Machine=0x8664, then
# SizeOfData bytes holding the NUL-terminated public symbol name followed by the
# NUL-terminated DLL name and, for EXPORTAS, the distinct loader export name. a
# /bigobj header shares the signature but carries its fixed ClassID. the records
# cover direct name, explicit export-name, and ordinal loader bindings.
set -eu
out=$1
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

# record <file> <symbol> <dll> <hint-or-ordinal> <flags> [export-name]
record() {
    f=$1; sym=$2; dll=$3; hint=$4; flags=$5
    export_name=${6-}
    n=$(( ${#sym} + 1 + ${#dll} + 1 ))
    if [ -n "$export_name" ]; then n=$(( n + ${#export_name} + 1 )); fi
    # header: sig1, sig2, version, machine(0x8664 LE), timestamp, size(LE u32),
    # ordinal/hint, then type/name-type
    printf '\000\000\377\377\000\000\144\206\000\000\000\000' >"$f"
    printf "$(printf '\\%03o\\%03o\\%03o\\%03o' \
        $(( n & 255 )) $(( (n >> 8) & 255 )) $(( (n >> 16) & 255 )) $(( (n >> 24) & 255 )))" >>"$f"
    printf "$(printf '\\%03o\\%03o\\%03o\\%03o' \
        $(( hint & 255 )) $(( (hint >> 8) & 255 )) \
        $(( flags & 255 )) $(( (flags >> 8) & 255 )))" >>"$f"
    printf '%s\000%s\000' "$sym" "$dll" >>"$f"
    if [ -n "$export_name" ]; then printf '%s\000' "$export_name" >>"$f"; fi
}

record "$tmp/a" Sleep       kernel32.dll 0   4  # code, by name
record "$tmp/b" ExitProcess kernel32.dll 0   4  # code, by name
record "$tmp/c" MessageBeep user32.dll   123 0  # code, by ordinal
record "$tmp/d" PublicName  demo.dll     7   16 ExportName # code, EXPORTAS

mkdir -p "$(dirname "$out")"
printf '!<arch>\n' >"$out"
for m in a b c d; do
    sz=$(wc -c <"$tmp/$m")
    # ar member header: 16 name, 12 mtime, 6 uid, 6 gid, 8 mode, 10 size, 2 magic.
    # a fixed mtime/uid/gid keeps the generated archive byte-stable across runs.
    printf '%-16s%-12s%-6s%-6s%-8s%-10s\140\n' 'imp.o/' '0' '0' '0' '644' "$sz" >>"$out"
    cat "$tmp/$m" >>"$out"
    # members are padded to an even offset
    if [ $(( sz % 2 )) -ne 0 ]; then printf '\n' >>"$out"; fi
done
