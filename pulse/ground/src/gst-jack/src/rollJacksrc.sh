#!/bin/sh

LEVEL=2
#FLAGS="-v -m --gst-debug-level=${LEVEL}"
gst-launch $FLAGS audiotestsrc ! jackaudiosink name=SineOutput \
jackaudiosrc ! jackaudiosink name=InputOutput
