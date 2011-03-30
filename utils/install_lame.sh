#!/bin/bash

# This script will (try to) download proper version of liblame for
# milhouse installation on Debian
#

which lsb_release >/dev/null
if [ $? -eq 0 ]; then
    LSB_RELEASE="lsb_release"
else
    echo "No lsb_release command found"
    exit 1
fi

DISTRO=$($LSB_RELEASE --short --id )
if [ "$DISTRO" != "Debian" ]
then
    echo "This script is made only for Debian"
    exit 1
fi

DOWNLOAD_DIR=~/src
if [ ! -d $DOWNLOAD_DIR ]
then
    DOWNLOAD_DIR=$(mktemp -p /tmp -d)
fi


ARCH=$(dpkg --print-architecture)

# Downloads
cd $DOWNLOAD_DIR
VERSION="3.98.2-0.5"
LAME="lame_${VERSION}_${ARCH}.deb"
LIBMP3LAME="libmp3lame0_${VERSION}_${ARCH}.deb"
LIBMP3LAME_DEV="libmp3lame-dev_${VERSION}_${ARCH}.deb"

for uri_path in $LAME $LIBMP3LAME $LIBMP3LAME_DEV
do
    wget -c http://debian-multimedia.org/pool/main/l/lame/$uri_path
done
set -x
sudo dpkg -i $LAME $LIBMP3LAME $LIBMP3LAME_DEV

