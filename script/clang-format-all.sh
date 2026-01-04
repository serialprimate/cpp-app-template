#!/usr/bin/env bash

# Enable strict mode (exit on error, exit on unset variable, error on pipeline)
set -euo pipefail

# Run clang-format on all .h and .cpp files in the app, lib, and test directories
for dir in app lib test; do
    if [ -d "${dir}" ]; then
        find "${dir}" \( -iname "*.h" -o -iname "*.cpp" -o -iname "*.hpp" \) -print0 | xargs -0 /usr/bin/clang-format -i
    fi
done
