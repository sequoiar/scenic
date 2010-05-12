#!/bin/bash
xgettext \
    --copyright-holder=SAT \
    --indent \
    --output=scenic.poy \
    --package-name=Scenic \
    --package-version=$(../py/scripts/scenic --version) \
    ../py/scenic/*.py \
    ../py/scenic/devices/*.py \
    ../data/glade/scenic.glade

#sed -i s/CHARSET/utf-8/g scenic.poy
mv scenic.poy scenic.pot
#msginit -l fr_CA -i scenic.pot


