#!/bin/bash
xgettext \
    --copyright-holder=SAT \
    --indent \
    --output=scenic.pot \
    --package-name=Scenic \
    --package-version=$(../py/scripts/scenic --version) \
    ../py/scenic/*.py \
    ../py/scenic/devices/*.py \
    ../data/glade/scenic.glade

