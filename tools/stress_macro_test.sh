#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-$ROOT_DIR/build}"
MACRO_COUNT="${2:-50000}"

if [[ ! -f "$BUILD_DIR/ukengine/libukengine.a" ]]; then
  echo "Missing $BUILD_DIR/ukengine/libukengine.a"
  echo "Build first: cmake --build $BUILD_DIR -j\$(nproc)"
  exit 1
fi

WORK_DIR="$(mktemp -d)"
cleanup() {
  rm -rf "$WORK_DIR"
}
trap cleanup EXIT

INPUT_FILE="$WORK_DIR/macro_input.txt"
OUTPUT_FILE="$WORK_DIR/macro_output.txt"
PROBE_CPP="$WORK_DIR/macro_probe.cpp"
PROBE_BIN="$WORK_DIR/macro_probe"

python3 - "$INPUT_FILE" "$MACRO_COUNT" <<'PY'
import sys

path = sys.argv[1]
count = int(sys.argv[2])

with open(path, "w", encoding="utf-8", newline="\n") as f:
    f.write("DO NOT DELETE THIS LINE*** version=1 ***\n")
    for i in range(count):
        key = f"k{i:08d}"  # <= 16 chars
        text = f"replacement_text_{i:08d}"
        if i + 1 == count:
            f.write(f"{key}:{text}")
        else:
            f.write(f"{key}:{text}\n")
PY

cat > "$PROBE_CPP" <<'CPP'
#include <cstdlib>
#include <iostream>
#include "mactab.h"

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cerr << "Usage: macro_probe <in> <out> <expected_count>\n";
    return 2;
  }

  const char* inFile = argv[1];
  const char* outFile = argv[2];
  const int expected = std::atoi(argv[3]);

  CMacroTable table;
  table.init();

  if (!table.loadFromFile(inFile)) {
    std::cerr << "Failed to load input macro file\n";
    return 1;
  }

  if (table.getCount() != expected) {
    std::cerr << "Loaded count mismatch: " << table.getCount()
              << " (expected " << expected << ")\n";
    return 1;
  }

  if (!table.writeToFile(outFile)) {
    std::cerr << "Failed to write output macro file\n";
    return 1;
  }

  table.resetContent();
  if (!table.loadFromFile(outFile)) {
    std::cerr << "Failed to reload output macro file\n";
    return 1;
  }

  if (table.getCount() != expected) {
    std::cerr << "Reloaded count mismatch: " << table.getCount()
              << " (expected " << expected << ")\n";
    return 1;
  }

  std::cout << "OK: loaded and reloaded " << table.getCount() << " macros\n";
  return 0;
}
CPP

g++ -std=c++17 -I"$ROOT_DIR/ukengine" "$PROBE_CPP" "$BUILD_DIR/ukengine/libukengine.a" -o "$PROBE_BIN"

"$PROBE_BIN" "$INPUT_FILE" "$OUTPUT_FILE" "$MACRO_COUNT"

echo "Stress test passed for $MACRO_COUNT macro definitions."
