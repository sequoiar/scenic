#!/bin/sh

# could be replaced with autoreconf -fivI m4 (verbose, force rebuild of ltmain, .in files, etc.)
aclocal -I m4
libtoolize --force
autoheader
autoconf -f
automake -a
./configure
