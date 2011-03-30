#!/bin/bash

# This script will download and install libboost 1.35 for Ubuntu 8.04.
# Don't use on other distributions

DOWNLOAD_DIR=~/src
if [ ! -d $DOWNLOAD_DIR ]
then
    DOWNLOAD_DIR=$(mktemp -p /tmp -d)
fi

which lsb_release >/dev/null
if [ $? -eq 0 ]; then
    LSB_RELEASE="lsb_release"
else
    echo "No lsb_release command found"
    exit 1
fi

DISTRO=$($LSB_RELEASE --short --code )
if [ "$DISTRO" != "hardy" ]
then
    echo "This script is made only for Ubuntu Hardy 8.0.4"
    echo "Other supported distros have the proper packages available"
    exit 1
fi

arch=$(dpkg --print-architecture)

baseurl=http://archive.ubuntu.com/ubuntu/pool/main/b/boost1.35/
version=1.35
release=8
subrelease=5
#This script will download boost deb packages from intrepid
for i in date-time filesystem graph iostreams program-options python regex serialization signals system test thread wave; do
    wget -c ${baseurl}libboost-${i}${version}{.0,-dev}_${version}.0-${release}ubuntu${subrelease}_${arch}.deb;
done

wget -c ${baseurl}libboost${version}-dev_${version}.0-${release}ubuntu${subrelease}_${arch}.deb;

set -x # we're using sudo, so show the user what's going on

sudo aptitude remove $(dpkg -l | awk '/^i.*libboost/{packages=packages $2 " "}END {print packages}' )
sudo aptitude install libicu38 libicu-dev gccxml
sudo dpkg -i libboost*${release}ubuntu${subrelease}_${arch}.deb

echo "Downloaded files are in " $DOWNLOAD_DIR
