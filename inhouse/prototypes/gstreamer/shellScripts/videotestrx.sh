#!/bin/bash

CAPS="application/x-rtp,media=video,clock-rate=90000, encoding-name=H264, payload=96";

gst-launch -v udpsrc port=5060 ! $CAPS ! rtph264depay ! ffdec_h264 ! ffmpegcolorspace ! ximagesink sync=false
