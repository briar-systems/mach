#!/usr/bin/env bash
# Build a minimal foreign Mach-O member, then force llvm-ar's Darwin dialect.
# The deliberately long basename produces a second `#1/<length>` member after
# the extended `__.SYMDEF` directory.
set -eu

out=$1
dir=$(dirname "$out")
mkdir -p "$dir"
obj="$dir/foreign-object-with-long-name.o"

llvm-mc -filetype=obj -triple=x86_64-apple-darwin shim.s -o "$obj"
llvm-ar --format=darwin rcs "$out" "$obj"
