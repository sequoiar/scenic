#!/bin/bash

gst-launch jackaudiosrc connect=0 ! audioconvert ! lame ! rtpmpapay max-ptime=20000000 ! rtpmpadepay ! mad ! audioconvert ! jackaudiosink connect=0 sync=false

