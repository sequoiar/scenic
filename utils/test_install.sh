#!/bin/sh

INSTALL_ROOT=$(mktemp -p /var/tmp -d miville-root-XXXXXXXX)
_PREFIX=/usr/local
PYTHON_LIB=$(python -E -c 'import sys; print("python%d.%d" % sys.version_info[0:2])')

export PYTHONPATH=$INSTALL_ROOT/$_PREFIX/lib/$PYTHON_LIB/site-packages

if [ ! -x ./autogen.sh ]
then
    echo "No ./autogen.sh script available" >&2
    echo "Quitting" >&2
    exit 1
fi

make distclean
./autogen.sh &&
make &&
make DESTDIR=$INSTALL_ROOT install

PATH=$INSTALL_ROOT/$_PREFIX/bin:$PATH

#miville
