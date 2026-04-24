#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build}"
THRESHOLD="${2:-80}"

pushd "$BUILD_DIR" >/dev/null

gcov CMakeFiles/unit-tests.dir/tests/test_rdg.cpp.o >/tmp/gcov_rdg_report.txt

coverage_line=$(awk '/File '\''\/workspace\/random-dungeon-generator\/rdg.h'\''/{getline; print; exit}' /tmp/gcov_rdg_report.txt)
coverage_pct=$(echo "$coverage_line" | sed -E 's/Lines executed:([0-9.]+)%.*/\1/')

if awk -v c="$coverage_pct" -v t="$THRESHOLD" 'BEGIN { exit !(c+0 >= t+0) }'; then
  echo "Coverage OK: rdg.h lines executed ${coverage_pct}% (threshold ${THRESHOLD}%)"
else
  echo "Coverage FAIL: rdg.h lines executed ${coverage_pct}% (threshold ${THRESHOLD}%)"
  exit 1
fi

popd >/dev/null
