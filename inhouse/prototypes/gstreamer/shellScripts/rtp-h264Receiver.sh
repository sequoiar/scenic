#!/bin/sh

MVEC=true

if [ $# = 1 ]; then
    gst-launch-0.10 -v udpsrc port=$1 caps="application/x-rtp" ! rtph264depay ! ffdec_h264 debug-mv=$MVEC ! xvimagesink sync=false
else
    gst-launch-0.10 -v udpsrc port=5060 caps="application/x-rtp" ! rtph264depay ! ffdec_h264 debug-mv=$MVEC ! xvimagesink sync=false
fi

