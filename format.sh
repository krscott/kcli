#!/usr/bin/env sh
set -eu

cd "$(dirname "$(readlink -f -- "$0")")"

for file in $(git ls-files '*.c' '*.h'); do
    clang-format -i "$file"
done
for file in $(git ls-files 'CMakeLists.txt' '*.cmake'); do
    cmake-format -i "$file"
done
for file in $(git ls-files '*.nix'); do
    nix fmt "$file"
done
