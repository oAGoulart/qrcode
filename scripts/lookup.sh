#!/usr/bin/env bash

OUT_FILE="lookup.S"
CSV_FILE="qrinfo.csv"
SCRIPT_DIR="$(dirname "$0")"
MAX_VERSION=5

declare_global() {
  echo ""
  echo "#if defined(__APPLE__)"
  printf ".globl _%s\n" "$1"
  printf "_%s:\n" "$1"
  echo "#else"
  printf ".global %s\n" "$1"
  printf "%s:\n" "$1"
  echo "#endif"
}

generate_indexes() {
  declare_global "qrindex"
  for ((i = 1 ; i <= MAX_VERSION ; i++)); do
    echo -e "$(python3 "$SCRIPT_DIR"/indexes.py "$i")"
  done
}

generate_info() {
  declare_global "qrinfo"
  echo -e "$(python3 "$SCRIPT_DIR"/info.py "$SCRIPT_DIR"/"$CSV_FILE")"
}

create_lookup() {
  cp "$SCRIPT_DIR"/lookup.template "$OUT_FILE"
  #generate_info
  generate_indexes
}

create_lookup >> "$OUT_FILE"
