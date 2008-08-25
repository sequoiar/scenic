#!/bin/sh

# could be replaced with autoconf -v -f (verbose, force rebuild of ltmain, .in files, etc.)
aclocal
libtoolize --force
autoheader
autoconf -f
automake -a
./configure --enable-debug
