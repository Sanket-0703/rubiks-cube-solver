#!/usr/bin/env bash
# Runs clang-tidy on every C++ source file, in parallel, using a configured build directory.
#   scripts/tidy.sh [build-dir]     (default build/debug; configure it first)
# Fails on any warning.
set -euo pipefail
cd "$(dirname "$0")/.."

BUILD_DIR=${1:-build/debug}
if [[ ! -f "$BUILD_DIR/compile_commands.json" ]]; then
    echo "error: $BUILD_DIR/compile_commands.json missing — run: cmake --preset debug" >&2
    exit 1
fi

if [[ -n "${CLANG_TIDY:-}" ]]; then
    :
elif command -v clang-tidy >/dev/null 2>&1; then
    CLANG_TIDY=$(command -v clang-tidy)
elif [[ -x /opt/homebrew/opt/llvm/bin/clang-tidy ]]; then
    CLANG_TIDY=/opt/homebrew/opt/llvm/bin/clang-tidy
else
    echo "error: clang-tidy not found (brew install llvm, or set CLANG_TIDY)" >&2
    exit 1
fi

# The configure step generates headers (config.hpp) that the sources include.
cmake --build "$BUILD_DIR" --target cubecore >/dev/null

JOBS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)
echo "$("$CLANG_TIDY" --version | grep -i version | head -1) — $JOBS jobs, $BUILD_DIR"

git ls-files --cached --others --exclude-standard '*.cpp' |
    xargs -P "$JOBS" -n 1 "$CLANG_TIDY" -p "$BUILD_DIR" --quiet --warnings-as-errors='*' \
        2> >(grep -vE '^[0-9]+ warnings? generated\.$|^Suppressed [0-9]+ warnings|^Use -header-filter|^Use .*NOLINT' >&2)
echo "clang-tidy OK"
