#!/usr/bin/env bash

OUT_FILE="lookup.S"
CSV_FILE="qrinfo.csv"
SCRIPT_DIR="$(dirname "$0")"
MAX_VERSION=40

generate_info() {
  echo "[1/4] Generating version info table"
  printf "\nglobale_ sym_(%s)\n" "qrinfo" >> "$OUT_FILE"
  python3 "$SCRIPT_DIR"/info.py "$SCRIPT_DIR"/"$CSV_FILE" >> "$OUT_FILE"
}

generate_indexes_data() {
  echo "[2/4] Generating align pattern indexes data"
  for ((i = 2 ; i <= MAX_VERSION ; i++)); do
    printf "\nglobale_ sym_(%s_%i)\n" "qralign" "$i" >> "$OUT_FILE"
    python3 "$SCRIPT_DIR"/indexes.py "$i" align >> "$OUT_FILE"
  done

  echo "[3/4] Generating module indexes data"
  for ((i = 1 ; i <= MAX_VERSION ; i++)); do
    printf "\nglobale_ sym_(%s_%i)\n" "qrindex" "$i" >> "$OUT_FILE"
    python3 "$SCRIPT_DIR"/indexes.py "$i" >> "$OUT_FILE"
  done
}

generate_pointers() {
  echo "[4/4] Generating pointer tables"
  cat << 'EOF' >> "$OUT_FILE"

#if defined(_WIN32) || defined(__MSYS__) || defined(__CYGWIN__)
  .section .data,"dw"
#elif defined(__APPLE__)
  .section __DATA,__const
#else
  .section .data.rel.ro,"aw",@progbits
#endif
EOF

  printf "\nglobale_ sym_(%s)\n" "qralign" >> "$OUT_FILE"
  for ((i = 2 ; i <= MAX_VERSION ; i++)); do
    printf "  pointer_ sym_(%s_%i)\n" "qralign" "$i" >> "$OUT_FILE"
  done

  printf "\nglobale_ sym_(%s)\n" "qrindex" >> "$OUT_FILE"
  for ((i = 1 ; i <= MAX_VERSION ; i++)); do
    printf "  pointer_ sym_(%s_%i)\n" "qrindex" "$i" >> "$OUT_FILE"
  done
}

create_lookup() {
  cp "$SCRIPT_DIR"/lookup.S "$OUT_FILE" 

  generate_info
  generate_indexes_data
  generate_pointers

  cat << 'EOF' >> "$OUT_FILE"

#if defined(__APPLE__)
  .subsections_via_symbols
#endif
EOF

}

create_lookup
