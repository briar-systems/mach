#!/usr/bin/env sh
# the codegen corpus entry point. see test/README.md for the CLI and the case
# contract; lib/driver.py holds the driver.
set -eu

here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

# the driver's own bytecode is a build product, and build products live under the
# output directory, never beside the source
PYTHONDONTWRITEBYTECODE=1
export PYTHONDONTWRITEBYTECODE

for py in python3 python; do
    if command -v "$py" >/dev/null 2>&1; then
        exec "$py" "$here/lib/driver.py" "$@"
    fi
done

echo "corpus: python3 is required to run the corpus driver and is not on PATH" >&2
exit 2
