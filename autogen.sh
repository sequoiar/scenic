#!/bin/sh

# could be replaced with autoconf -v -f (verbose, force rebuild of ltmain, .in files, etc.)
#uncrustify -c utils/uncrustify2.cfg --suffix "" `find . -name "*.cpp"`
#uncrustify -c utils/uncrustify2.cfg --suffix "" `find . -name "*.h"`
#exit 0
aclocal
autoconf -f
automake -a
#./configure '--enable-CXX_OPT=-O0 -g -Wall -Weffc++ -Werror -Wfatal-errors'
./configure
