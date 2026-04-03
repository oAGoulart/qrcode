#!/usr/bin/env sh

OUT_FILE="lookup.S"
CSV_FILE="qrinfo.csv"
SCRIPT_DIR="$(dirname "$0")"
MAX_VERSION=40
NUM_STEPS=5

generate_info() {
  printf "[1/%i] Generating version info table\n" "$NUM_STEPS"
  printf "\nglobale_ sym_(%s)\n" "qrinfo" >> "$OUT_FILE"
  python3 "$SCRIPT_DIR"/info.py "$SCRIPT_DIR"/"$CSV_FILE" >> "$OUT_FILE"
}

generate_indexes_data() {
  printf "[2/%i] Generating align pattern indexes data\n" "$NUM_STEPS"
  i=2
  while [ "$i" -le "$MAX_VERSION" ]; do
    printf "\nglobale_ sym_(%s_%i)\n" "qralign" "$i" >> "$OUT_FILE"
    python3 "$SCRIPT_DIR"/indexes.py "$i" align >> "$OUT_FILE"
    i=$((i + 1))
  done

  printf "[3/%i] Generating masks xor pattern data\n" "$NUM_STEPS"
  i=1
  while [ "$i" -le "$MAX_VERSION" ]; do
    printf "\nglobale_ sym_(%s_%i)\n" "qrmasks" "$i" >> "$OUT_FILE"
    python3 "$SCRIPT_DIR"/indexes.py "$i" masks >> "$OUT_FILE"
    i=$((i + 1))
  done

  printf "[4/%i] Generating module indexes data\n" "$NUM_STEPS"
  i=1
  while [ "$i" -le "$MAX_VERSION" ]; do
    printf "\nglobale_ sym_(%s_%i)\n" "qrindex" "$i" >> "$OUT_FILE"
    python3 "$SCRIPT_DIR"/indexes.py "$i" >> "$OUT_FILE"
    i=$((i + 1))
  done
}

generate_pointers() {
  printf "[5/%i] Generating pointer tables\n" "$NUM_STEPS"
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
  i=2
  while [ "$i" -le "$MAX_VERSION" ]; do
    printf "  pointer_ sym_(%s_%i)\n" "qralign" "$i" >> "$OUT_FILE"
    i=$((i + 1))
  done

  printf "\nglobale_ sym_(%s)\n" "qrmasks" >> "$OUT_FILE"
  i=1
  while [ "$i" -le "$MAX_VERSION" ]; do
    printf "  pointer_ sym_(%s_%i)\n" "qrmasks" "$i" >> "$OUT_FILE"
    i=$((i + 1))
  done

  printf "\nglobale_ sym_(%s)\n" "qrindex" >> "$OUT_FILE"
  i=1
  while [ "$i" -le "$MAX_VERSION" ]; do
    printf "  pointer_ sym_(%s_%i)\n" "qrindex" "$i" >> "$OUT_FILE"
    i=$((i + 1))
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
