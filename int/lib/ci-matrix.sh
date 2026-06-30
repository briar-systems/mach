#!/usr/bin/env bash
# ci-matrix.sh — emit the GitHub Actions matrix for one cadence from targets.conf.
#
# usage: ci-matrix.sh <cadence>
#
# reads int/targets.conf, keeps the rows whose cadence column matches <cadence>,
# and prints a compact JSON array of leg objects on stdout:
#
#   [{"target":"linux","runner":"ubuntu-latest","runmode":"native"}, ...]
#
# the workflow feeds this straight into `strategy.matrix.include` via fromJSON, so
# the leg set is wholly data-driven: a new targets.conf row appears in CI with no
# workflow edit. keys avoid hyphens so `matrix.runmode` resolves in the GH context.
set -eu

cadence=${1:-}
if [ -z "$cadence" ]; then
    echo "usage: ci-matrix.sh <cadence>" >&2
    exit 2
fi

here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
conf="$here/../targets.conf"

printf '['
first=1
while read -r name runner runmode rowcadence rest; do
    case "$name" in
        ''|\#*) continue ;;
    esac
    [ "$rowcadence" = "$cadence" ] || continue
    if [ "$first" -eq 1 ]; then first=0; else printf ','; fi
    printf '{"target":"%s","runner":"%s","runmode":"%s"}' "$name" "$runner" "$runmode"
done < "$conf"
printf ']\n'
