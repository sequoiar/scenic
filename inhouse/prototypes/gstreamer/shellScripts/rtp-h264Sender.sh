#!/bin/bash

if [ $# = 3 ]; then
    if [ $1 = "dv" ]; then
        gst-launch-0.10 dv1394src ! dvdemux ! dvdec ! ffmpegcolorspace ! x264enc bitrate=2000 byte-stream=true threads=4 ! rtph264pay ! udpsink host=$2 port=$3
    else
        gst-launch-0.10 v4l2src ! ffmpegcolorspace ! x264enc bitrate=2000 byte-stream=true threads=4 ! rtph264pay ! udpsink host=$2 port=$3
    fi
else
    echo "$0" requires dv or v4l  - host ip and port 
fi

