#!/usr/bin/env bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly repo_root

cd "${repo_root}"

if [[ ! -e "external/vcpkg/.git" ]]; then
  printf '%s\n' "Initialising vcpkg submodule..."
  git submodule update --init --recursive
fi

if [[ ! -x "external/vcpkg/vcpkg" ]]; then
  printf '%s\n' "Bootstrapping vcpkg..."
  ./external/vcpkg/bootstrap-vcpkg.sh -disableMetrics
fi

mkdir -p "vcpkg/cache"

printf '%s\n' "vcpkg is ready."
