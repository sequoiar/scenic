#!/bin/sh

# could be replaced with autoconf -vf (verbose, force rebuild of ltmain, .in files, etc.)
#uncrustify -c uncrustify.cfg --suffix "" `find . -name "*.cpp"`
#uncrustify -c uncrustify.cfg --suffix "" `find . -name "*.h"`
aclocal
autoconf
automake -a
./configure CXXFLAGS='-O0 -g -Wall -Werror -Wfatal-errors'

