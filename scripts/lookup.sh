#!/usr/bin/env bash

SCRIPT_DIR="$(dirname "$0")"
MAX_VERSION=5

cp "$SCRIPT_DIR"/lookup.template lookup.S

for ((i = 1 ; i <= MAX_VERSION ; i++)); do
  echo -e "$(python3 "$SCRIPT_DIR"/indexes.py "$i")" >> lookup.S
done
