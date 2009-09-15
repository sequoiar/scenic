#!/bin/bash

# This script will (try to) download proper version of gstreamer for 
# milhouse installation
#
# The current needed modules are:
#  1. gstreamer 0.10.24
#  2. gstreamer-plugin-base 0.10.24
#  3. gst-python 0.10.16
#  4. gst-plugin-good 0.10.16
#  5. gst-plugin-bad 0.10.14
#  6. gst-plugin-ugly 0.10.12
#  7. gst-ffmpeg 0.10.10.7

DOWNLOAD_DIR=~/src/gstreamer-src
if [ ! -d $DOWNLOAD_DIR ]
then
    DOWNLOAD_DIR=$(mktemp -p /tmp -d)
fi

# makeinstall is a shell script for safe inclusion in /etc/sudoers
# don't interfere if it's not there
which makeinstall >/dev/null
if [ $? -eq 1 ]; then
    MAKEINSTALL="make install"
else
    MAKEINSTALL=$(which makeinstall)
fi

# Downloads
cd $DOWNLOAD_DIR
for uri_path in /gstreamer/gstreamer-0.10.24.tar.bz2 \
                /gst-plugins-base/gst-plugins-base-0.10.24.tar.bz2 \
                /gst-plugins-good/gst-plugins-good-0.10.16.tar.bz2 \
                /gst-plugins-bad/gst-plugins-bad-0.10.14.tar.bz2 \
                /gst-plugins-ugly/gst-plugins-ugly-0.10.12.tar.bz2 \
                /gst-ffmpeg/gst-ffmpeg-0.10.7.tar.bz2 \
                /gst-python/gst-python-0.10.16.tar.bz2 \
                /gst-plugins-gl/gst-plugins-gl-0.10.1.tar.bz2
do
    wget -c http://gstreamer.freedesktop.org/src/$uri_path
done

# Build!
for module in gstreamer-0.10.24 gst-plugins-base-0.10.24 gst-plugins-good-0.10.16 gst-plugins-bad-0.10.14 gst-plugins-ugly-0.10.12 gst-python-0.10.16 gst-ffmpeg-0.10.7 gst-plugins-gl-0.10.1
do
    echo "]2;Now building $module"
    echo "########################################"
    echo "Now building $module"
    echo "########################################"
    echo $PWD
    tar xjf ${module}.tar.bz2
    pushd ${module}
    make clean
    ./configure --disable-docbook --disable-gtk-doc
    make
    sudo $MAKEINSTALL
    popd
    echo "Done building $module"
    done

    echo "Installation completed, but who knows"
    echo "Life is full of surprises"
    echo "Downloaded files are in " $DOWNLOAD_DIR   
