#!/bin/sh

LEVEL=2
FLAGS="-v -m --gst-debug-level=${LEVEL}"
AUTOCONNECT=0
gst-launch $FLAGS jackaudiosrc connect=$AUTOCONNECT ! jackaudiosink connect=$AUTOCONNECT 
