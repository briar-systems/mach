#!/bin/sh
set -eu
out=$1
cc -c provider.c -o "$out"
