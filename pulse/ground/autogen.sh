#!/bin/sh

uncrustify -c uncrustify.cfg --suffix "" `find . -name "*.cpp"`
uncrustify -c uncrustify.cfg --suffix "" `find . -name "*.h"`
aclocal
autoconf
automake -a
./configure CXXFLAGS='-O0 -g -Wall'

