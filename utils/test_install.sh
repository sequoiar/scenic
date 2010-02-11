#!/bin/bash

INSTALL_ROOT=$(mktemp -p /var/tmp -d miville-root-XXXXXXXX)
_PREFIX=/usr/local
PYTHON_LIB=$(python -E -c 'import sys; print("python%d.%d" % sys.version_info[0:2])')

export PYTHONPATH=$INSTALL_ROOT/$_PREFIX/lib/$PYTHON_LIB/site-packages

make distclean
if [ ! -x ./autogen.sh ]
then
    echo "No ./autogen.sh script available" >&2
    if [ ! -x ./configure ]
    then
        echo "No ./configure script available" >&2
    else
        ./configure
    fi
else
    ./autogen.sh
fi &&

make &&
make DESTDIR=$INSTALL_ROOT install

PATH=$INSTALL_ROOT/$_PREFIX/bin:$PATH

