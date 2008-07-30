#!/bin/sh

# could be replaced with autoconf -v -f (verbose, force rebuild of ltmain, .in files, etc.)
aclocal
libtoolize --force
autoconf -f
automake -a
./configure 'CXXFLAGS=-O0 -g'
