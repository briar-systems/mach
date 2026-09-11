#!/usr/bin/env sh
# the vector-rows entry point: one independent probe per declared vector cell on
# every target this host serves. see run.py's docstring for the CLI and rows.conf
# for the declarations it judges.
set -eu

here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

PYTHONDONTWRITEBYTECODE=1
export PYTHONDONTWRITEBYTECODE

for py in python3 python; do
    if command -v "$py" >/dev/null 2>&1; then
        exec "$py" "$here/run.py" "$@"
    fi
done

echo "vecrows: python3 is required to run the vector-rows driver and is not on PATH" >&2
exit 2
