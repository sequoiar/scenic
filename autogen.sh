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

# FIXME: this is because HARDY doesn't seem to make build-aux
mkdir -p build-aux

# could be replaced with autoreconf -fivI m4 (verbose, force rebuild of ltmain, .in files, etc.)
libtoolize --force
aclocal -I m4
autoheader
autoconf -f
automake -a -f -Wno-portability 
if [ ! "x$LOGNAME" = "xbbslave" ]; then
    ./configure $@ --enable-svn-revision
fi
