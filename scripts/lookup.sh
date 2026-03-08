#!/usr/bin/env bash

OUT_FILE="lookup.S"
CSV_FILE="qrinfo.csv"
SCRIPT_DIR="$(dirname "$0")"
MAX_VERSION=40

generate_indexes() {
  echo "[2/3] Generating align pattern indexes table"
  for ((i = 2 ; i <= MAX_VERSION ; i++)); do
    printf "\nglobale_ sym_(%s_%i)\n" "qralign" "$i" >> "$OUT_FILE"
    python "$SCRIPT_DIR"/indexes.py "$i" align >> "$OUT_FILE"
  done
  printf "\nglobale_ sym_(%s)\n" "qralign" >> "$OUT_FILE"
  for ((i = 2 ; i <= MAX_VERSION ; i++)); do
    printf "  pointer_ sym_(%s_%i)\n" "qralign" "$i" >> "$OUT_FILE"
  done

  echo "[3/3] Generating module indexes table"
  for ((i = 1 ; i <= MAX_VERSION ; i++)); do
    printf "\nglobale_ sym_(%s_%i)\n" "qrindex" "$i" >> "$OUT_FILE"
    python "$SCRIPT_DIR"/indexes.py "$i" >> "$OUT_FILE"
  done
  printf "\nglobale_ sym_(%s)\n" "qrindex" >> "$OUT_FILE"
  for ((i = 1 ; i <= MAX_VERSION ; i++)); do
    printf "  pointer_ sym_(%s_%i)\n" "qrindex" "$i" >> "$OUT_FILE"
  done
}

generate_info() {
  echo "[1/3] Generating version info table"
  printf "\nglobale_ sym_(%s)\n" "qrinfo" >> "$OUT_FILE"
  python "$SCRIPT_DIR"/info.py "$SCRIPT_DIR"/"$CSV_FILE" >> "$OUT_FILE"
}

create_lookup() {
  cp "$SCRIPT_DIR"/lookup.S "$OUT_FILE"
  generate_info
  generate_indexes
}

create_lookup
