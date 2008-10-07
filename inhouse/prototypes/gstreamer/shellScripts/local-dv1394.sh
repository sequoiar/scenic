#!/bin/bash
gst-launch --verbose dv1394src ! dvdemux name=demux \
demux. ! queue ! dvdec ! xvimagesink sync=false \
demux. ! queue ! audioconvert ! alsasink sync=false
