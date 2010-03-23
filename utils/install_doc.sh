#!/bin/bash

if [ ! $# = 1 ]; then
    echo "Usage $(basename 0) target_directory" >&2
    exit 1
fi
target="$1"
base=$(dirname $0)/..

rsync -rv --delete $base/doc/html/ $target/doxygen/
rsync -rv --delete $base/doc/pythondocs/ $target/epydocs/
