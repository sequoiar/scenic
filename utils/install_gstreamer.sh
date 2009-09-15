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

GST_CORE=gstreamer-0.10.24
GST_BASE=gst-plugins-base-0.10.24
GST_GOOD=gst-plugins-good-0.10.16
GST_BAD=gst-plugins-bad-0.10.14
GST_UGLY=gst-plugins-ugly-0.10.12
GST_FFMPEG=gst-ffmpeg-0.10.7
GST_PYTHON=gst-python-0.10.16
GST_GL=gst-plugins-gl-0.10.1
MODULES="$GST_CORE $GST_BASE $GST_GOOD $GST_BAD $GST_UGLY $GST_FFMPEG $GST_PYTHON $GST_GL"

# Downloads
cd $DOWNLOAD_DIR
for uri_path in gstreamer/$GST_CORE.tar.bz2 \
                gst-plugins-base/$GST_BASE.tar.bz2 \
                gst-plugins-good/$GST_GOOD.tar.bz2 \
                gst-plugins-bad/$GST_BAD.tar.bz2 \
                gst-plugins-ugly/$GST_UGLY.tar.bz2 \
                gst-ffmpeg/$GST_FFMPEG.tar.bz2 \
                gst-python/$GST_PYTHON.tar.bz2 \
                gst-plugins-gl/$GST_GL.tar.bz2
do
    wget -c http://gstreamer.freedesktop.org/src/$uri_path
done

# Build!
for module in $MODULES
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
