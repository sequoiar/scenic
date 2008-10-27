#!/bin/sh

LEVEL=0
ELEMENT="jackaudiosrc:"
FLAGS="-m --gst-debug-level=${ELEMENT}${LEVEL} -v"
AUTOCONNECT="connect=0"
gst-launch $FLAGS jackaudiosrc $AUTOCONNECT ! jackaudiosink $AUTOCONNECT sync=FALSE
