#!/bin/sh

# could be replaced with autoconf -v -f (verbose, force rebuild of ltmain, .in files, etc.)
#uncrustify -c utils/uncrustify2.cfg --no-backup `find src/ -name "*.cpp"`
#uncrustify -c utils/uncrustify2.cfg --no-backup `find src/ -name "*.h"`
#exit 0
aclocal
libtoolize --force
autoconf -f
automake -a
./configure 'CXXFLAGS=-O0 -g'
#./configure
