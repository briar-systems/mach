#!/bin/sh
# require the driver to root project.work before entering the symlinked dependency
set -eu

out=$1
case "$out" in
    /*) ;;
    *) echo "dependency project.work is not absolute: $out" >&2; exit 1 ;;
esac

test ! -e out
cc -c provider.c -o "$out"
test ! -e out
