#!/usr/bin/env bash
# Reconcile this repo's issue labels with the manifest in labels.yml (idempotent).
#
# usage:
#   .github/sync-labels.sh             create/update labels from the manifest
#   .github/sync-labels.sh --prune     also delete repo labels absent from the manifest
#   .github/sync-labels.sh --dry-run   print the actions without applying them
#
# requires: gh (authenticated), python3 with pyyaml.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
manifest="$here/labels.yml"

prune=0 dry=0
for arg in "$@"; do
  case "$arg" in
    --prune)   prune=1 ;;
    --dry-run) dry=1 ;;
    -h|--help) sed -n '2,9p' "$0"; exit 0 ;;
    *) echo "unknown argument: $arg" >&2; exit 2 ;;
  esac
done

command -v gh >/dev/null 2>&1 || { echo "error: gh not found on PATH" >&2; exit 1; }
python3 -c 'import yaml' >/dev/null 2>&1 || { echo "error: python3 with pyyaml required (pip install pyyaml)" >&2; exit 1; }

run() { if [ "$dry" -eq 1 ]; then echo "would: $*"; else "$@"; fi; }

# desired labels as name<TAB>color<TAB>description
desired="$(python3 - "$manifest" <<'PY'
import sys, yaml
for l in yaml.safe_load(open(sys.argv[1]))["labels"]:
    print("{}\t{}\t{}".format(l["name"], str(l["color"]).lstrip("#"), l.get("description", "")))
PY
)"

while IFS=$'\t' read -r name color desc; do
  [ -z "$name" ] && continue
  run gh label create "$name" --color "$color" --description "$desc" --force
done <<< "$desired"

if [ "$prune" -eq 1 ]; then
  want="$(cut -f1 <<< "$desired" | sort -u)"
  gh label list --limit 200 --json name -q '.[].name' | sort -u | while read -r name; do
    grep -qxF "$name" <<< "$want" || run gh label delete "$name" --yes
  done
fi

[ "$dry" -eq 1 ] && echo "dry-run complete; no changes made." || echo "labels synced."
