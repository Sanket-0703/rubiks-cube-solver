#!/usr/bin/env bash
# Builds with coverage, runs every test, and reports line/function/branch coverage of cubecore.
#   scripts/coverage.sh                 summary + HTML report in build/coverage/html/index.html
#   scripts/coverage.sh --fail-under N  also fail if cubecore line coverage is below N percent
# Uses clang's source-based coverage (llvm-profdata + llvm-cov). The tools must match the
# compiler: on macOS they come from Xcode's toolchain (xcrun); elsewhere set LLVM_PROFDATA and
# LLVM_COV (e.g. llvm-profdata-18 / llvm-cov-18 for clang 18).
set -euo pipefail
cd "$(dirname "$0")/.."

FAIL_UNDER=""
if [[ "${1:-}" == "--fail-under" ]]; then FAIL_UNDER=${2:?percentage required}; fi

if [[ "$(uname)" == "Darwin" ]]; then
    LLVM_PROFDATA=${LLVM_PROFDATA:-"xcrun llvm-profdata"}
    LLVM_COV=${LLVM_COV:-"xcrun llvm-cov"}
else
    LLVM_PROFDATA=${LLVM_PROFDATA:-llvm-profdata}
    LLVM_COV=${LLVM_COV:-llvm-cov}
fi

BUILD=build/coverage
cmake --preset coverage >/dev/null
cmake --build --preset coverage
rm -rf "$BUILD/profiles" && mkdir -p "$BUILD/profiles"
ctest --preset coverage

$LLVM_PROFDATA merge -sparse "$BUILD"/profiles/*.profraw -o "$BUILD/coverage.profdata"

OBJECTS=("$BUILD/tests/cubecore_tests" -object "$BUILD/apps/cubesolver/cubesolver")
SOURCES=(cubecore/)

$LLVM_COV report "${OBJECTS[@]}" -instr-profile="$BUILD/coverage.profdata" "${SOURCES[@]}" |
    tee "$BUILD/summary.txt"
$LLVM_COV show "${OBJECTS[@]}" -instr-profile="$BUILD/coverage.profdata" "${SOURCES[@]}" \
    -format=html -output-dir="$BUILD/html" -show-line-counts-or-regions -show-branches=count
echo "HTML report: $BUILD/html/index.html"

if [[ -n "$FAIL_UNDER" ]]; then
    # TOTAL row columns: regions, missed, cover%, functions, missed, exec%, lines, missed, cover%, ...
    LINE_PCT=$(awk '/^TOTAL/ { gsub("%", "", $10); print $10 }' "$BUILD/summary.txt")
    echo "cubecore line coverage: ${LINE_PCT}% (required: ${FAIL_UNDER}%)"
    awk -v got="$LINE_PCT" -v need="$FAIL_UNDER" 'BEGIN { exit !(got + 0 >= need + 0) }' ||
        { echo "error: coverage below ${FAIL_UNDER}%" >&2; exit 1; }
fi
