#!/bin/bash
gst-launch-0.10 dv1394src ! dvdemux ! dvdec ! ffmpegcolorspace ! x264enc bitrate=12000 byte-stream=true threads=4 ! ffdec_h264 ! xvimagesink sync=false
