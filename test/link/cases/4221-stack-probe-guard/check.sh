#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

engine="$1"; leg="$2"; bin="$3"
out="$(mktemp)"
trap 'rm -f "$out"' EXIT
run_captured "$engine" "$leg" "$bin" "$out"
case "$run_status" in
    138|139) echo "faulted at the guard" ;;
    1)       echo "ran past the guard" ;;
    *)       report_run_failure "$leg" "$run_status" "$run_out"; exit 1 ;;
esac
