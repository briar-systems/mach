#!/bin/sh
set -eu
mkdir -p "$(dirname "$1")"
base64 -d assets/app.ico.b64 >"$1"
