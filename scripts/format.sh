#!/usr/bin/env bash
# Formats every C++ file in the repo with clang-format.
#   scripts/format.sh          rewrite files in place
#   scripts/format.sh --check  change nothing; fail if any file needs formatting (CI)
set -euo pipefail
cd "$(dirname "$0")/.."

find_tool() {
    local name=$1 override=$2
    if [[ -n "$override" ]]; then echo "$override"; return; fi
    if command -v "$name" >/dev/null 2>&1; then command -v "$name"; return; fi
    if [[ -x "/opt/homebrew/opt/llvm/bin/$name" ]]; then echo "/opt/homebrew/opt/llvm/bin/$name"; return; fi
    echo "error: $name not found (brew install llvm, or set ${name//-/_} env var)" >&2
    exit 1
}
CLANG_FORMAT=$(find_tool clang-format "${CLANG_FORMAT:-}")

files=()
while IFS= read -r f; do files+=("$f"); done < <(git ls-files --cached --others --exclude-standard '*.cpp' '*.hpp')
[[ ${#files[@]} -eq 0 ]] && { echo "no C++ files"; exit 0; }

echo "$("$CLANG_FORMAT" --version | head -1) — ${#files[@]} files"
if [[ "${1:-}" == "--check" ]]; then
    "$CLANG_FORMAT" --dry-run --Werror "${files[@]}"
    echo "formatting OK"
else
    "$CLANG_FORMAT" -i "${files[@]}"
fi
