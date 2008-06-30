#!/bin/sh

aclocal
autoconf
automake -a
./configure CXXFLAGS='-O0 -g -Wall -pedantic'

