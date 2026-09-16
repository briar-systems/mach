#!/usr/bin/env sh
# a step the default library artifact requires, so it runs for the consumer
set -eu
mkdir -p "$(dirname "$1")"
printf 'boom\n' > "$1"
