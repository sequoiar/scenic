#!/bin/bash
gst-launch-0.10 dv1394src ! filesink location="test.dv"

#dvdemux ! dvdec ! ffmpegcolorspace ! x264enc byte-stream=true threads=4 ! filesink location="r.264"
