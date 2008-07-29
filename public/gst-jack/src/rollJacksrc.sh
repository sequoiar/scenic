#!/bin/sh

LEVEL=5
ELEMENT="jackaudiosrc:"
FLAGS="-m --gst-debug-level=${ELEMENT}${LEVEL}"
AUTOCONNECT="connect=0"
gst-launch $FLAGS jackaudiosrc $AUTOCONNECT ! jackaudiosink $AUTOCONNECT sync=FALSE
