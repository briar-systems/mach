#!/usr/bin/env sh
# the object-format fuzz runner. see test/fuzz/README.md for the corpus contract
# and the runner's CLI; lib/fuzz.py holds the driver.
set -eu

here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

PYTHONDONTWRITEBYTECODE=1
export PYTHONDONTWRITEBYTECODE

for py in python3 python; do
    if command -v "$py" >/dev/null 2>&1; then
        exec "$py" "$here/lib/fuzz.py" "$@"
    fi
done

echo "fuzz: python3 is required to run the fuzz driver and is not on PATH" >&2
exit 2
