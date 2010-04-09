#!/bin/bash

# This script will (try to) download proper version of gstreamer for 
# milhouse installation
#
# The current needed modules are:
#  1. gstreamer 0.10.25
#  2. gstreamer-plugin-base 0.10.25
#  3. gst-python 0.10.17
#  4. gst-plugin-good 0.10.17
#  5. gst-plugin-bad 0.10.17
#  6. gst-plugin-ugly 0.10.13
#  7. gst-ffmpeg 0.10.10.9

SCRIPT_PATH=$(pwd)/$(dirname $0)

DOWNLOAD_DIR=~/src/gstreamer-src
if [ ! -d $DOWNLOAD_DIR ]
then
    mkdir -p $DOWNLOAD_DIR
fi

# makeinstall is a shell script for safe inclusion in /etc/sudoers
# don't interfere if it's not there
which makeinstall >/dev/null
if [ $? -eq 1 ]; then
    MAKEINSTALL="make install"
else
    MAKEINSTALL=$(which makeinstall)
fi

GST_CORE=gstreamer-0.10.25
GST_BASE=gst-plugins-base-0.10.25
GST_GOOD=gst-plugins-good-0.10.17
GST_BAD=gst-plugins-bad-0.10.17
GST_UGLY=gst-plugins-ugly-0.10.13
GST_FFMPEG=gst-ffmpeg-0.10.9
GST_PYTHON=gst-python-0.10.17
GST_GL=gst-plugins-gl-0.10.1
YASM=yasm-0.8.0
X264=x264-snapshot-20090711-2245
DC1394=libdc1394-2.1.2
MODULES="$GST_CORE $GST_BASE $GST_GOOD $GST_BAD $GST_UGLY $GST_FFMPEG $GST_PYTHON $GST_GL"

# Downloads
cd $DOWNLOAD_DIR

#get YASM, as x264 depends on a post-hardy version of it
wget -c http://www.tortall.net/projects/yasm/releases/$YASM.tar.gz
tar xzf $YASM.tar.gz
pushd $YASM
./configure 
make 
sudo $MAKEINSTALL
popd


# get x264, note that as of 09/23/2009 upstream x264 is incompatible with gstreamer
# and that there are unfortunately no real releases of x264, only daily tarballs
wget -c http://download.videolan.org/pub/videolan/x264/snapshots/$X264.tar.bz2
tar xjf $X264.tar.bz2
pushd $X264
## added enable-shared flag to make sure that we can build the gstreamer x264enc
## plugin on 64 bit platforms
./configure --enable-shared
make 
sudo $MAKEINSTALL
sudo ldconfig
popd


# get libdc1394
wget -c http://downloads.sourceforge.net/project/libdc1394/libdc1394-2/2.1.2/$DC1394.tar.gz
tar xzf $DC1394.tar.gz
pushd $DC1394
./configure
make
sudo $MAKEINSTALL
sudo ldconfig
popd


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

RTPPATCH=$SCRIPT_PATH/rtpsource.diff
DC1394PATCH=$SCRIPT_PATH/dc1394-iso-speed.diff
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
    if [ ${module} == "$GST_BAD" ]; then
        patch --dry-run -p1 -i $DC1394PATCH
        if [ $? -eq 0 ]; then
            patch -p1 -i $DC1394PATCH
        fi
    elif [ ${module} == "$GST_GOOD" ]; then
        patch --dry-run -p1 -i $RTPPATCH
        if [ $? -eq 0 ]; then
            patch -p1 -i $RTPPATCH
        fi
    fi
    make clean
    ./configure --disable-docbook --disable-gtk-doc
    make
    sudo $MAKEINSTALL
    sudo ldconfig
    popd
    echo "Done building $module"
    done

echo "Installation completed, but who knows"
echo "Life is full of surprises"
echo "Downloaded files are in " $DOWNLOAD_DIR

