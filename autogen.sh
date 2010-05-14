#!/bin/sh
if test ! -e NEWS ; then
touch NEWS
fi

if test ! -e INSTALL ; then
touch INSTALL
fi

if test ! -e AUTHORS ; then
touch AUTHORS
fi

if test ! -e README ; then
touch README
fi

if test ! -e ChangeLog ; then
touch ChangeLog
fi

# could be replaced with autoreconf -fivI m4 (verbose, force rebuild of ltmain, .in files, etc.)
autoreconf --install

if [ $! != 0 ]; then 
    exit 1
fi

if [ ! "x$LOGNAME" = "xbbslave" ]; then
    ./configure $@ --enable-svn-revision
fi
