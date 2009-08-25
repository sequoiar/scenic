#!/bin/bash

# This script will download and install libboost 1.35 for Ubuntu 8.04.
# Don't use on other distributions

DOWNLOAD_DIR=~/src
if [ ! -d $DOWNLOAD_DIR ]
then
    DOWNLOAD_DIR=$(mktemp -p /tmp -d)
fi

uname=$(uname -m)
arch="i386"
if [ "x$uname" == "xx86_64" ]
then
    arch="amd64"
fi
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
