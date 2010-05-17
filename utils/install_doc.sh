#!/bin/bash

if [ ! $# = 1 ]; then
    echo "Usage $(basename 0) target_directory" >&2
    exit 1
fi
target="$1"
base=$(dirname $0)/..

rsync -rlv --delete $base/doc/html/ $target/doxygen/
rsync -rlv --delete $base/doc/pythondocs/ $target/epydocs/
rsync -rlv --delete $base/doc/docbook/html/installation-manual/ $target/installation-manual/
rsync -rlv --delete $base/doc/docbook/html/user-manual/ $target/user-manual/
