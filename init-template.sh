#!/usr/bin/env sh
set -eu

if [ $# -ne 1 ]; then
    echo "Usage: $(basename "$0") NAME"
    exit 1
fi

proj="$1"
proj_upper=$(echo "$proj" | tr '[:lower:]' '[:upper:]')

cd "$(dirname "$(readlink -f -- "$0")")"

for file in $(git ls-files | grep -v 'init-template.sh'); do
    if [ -e "$file" ]; then
        echo "Processing: $file"
        sed -i "s/cstart/$proj/g" "$file"
        sed -i "s/c-start/$proj/g" "$file"
        sed -i "s/CSTART/$proj_upper/g" "$file"
    fi
done

echo "Renaming files"
mv include/cstartlib.h "include/${proj}lib.h"
mv src/cstartlib.c "src/${proj}lib.c"
mv test/cstarttest.c "src/${proj}test.c"

echo "Deleting init script"
rm -- "$0"
