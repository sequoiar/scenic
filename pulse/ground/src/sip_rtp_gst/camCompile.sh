#!/bin/sh

BASE=~/devel/miville/trunk/pulse/ground/
BINDIR=${BASE}bin/
gcc -Wall $(pkg-config --cflags --libs gstreamer-0.10) cam.c -o ${BINDIR}cam
