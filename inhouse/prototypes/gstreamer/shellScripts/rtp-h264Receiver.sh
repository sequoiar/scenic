#!/bin/sh

MVEC=false
CAPS="application/x-rtp,media=video,clock-rate=90000, encoding-name=H264, payload=96";

gst-launch-0.10 -v udpsrc port=5060 caps="application/x-rtp" ! $CAPS ! rtph264depay ! ffdec_h264 debug-mv=$MVEC ! ffmpegcolorspace ! ximagesink sync=false
