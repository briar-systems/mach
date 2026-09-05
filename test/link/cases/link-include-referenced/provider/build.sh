#!/bin/sh
set -eu
out=$1
cc -shared -fPIC -nostdlib -fno-stack-protector -Wl,-soname,libdemo.so.1 -o "$out" demo.c
