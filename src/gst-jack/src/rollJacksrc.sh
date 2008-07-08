#!/bin/sh

LEVEL=2
#FLAGS="-v -m --gst-debug-level=${LEVEL}"
gst-launch $FLAGS jackaudiosrc connect=0 ! fakesink
