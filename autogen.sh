#!/bin/sh

# could be replaced with autoreconf -fivI m4 (verbose, force rebuild of ltmain, .in files, etc.)
autoreconf --install

if [ $! != 0 ]; then 
    exit 1
fi

if [ ! "x$LOGNAME" = "xbbslave" ]; then
    ./configure $@ --enable-svn-revision
fi
