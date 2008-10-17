#!/bin/bash

gst-launch jackaudiosrc connect=0 ! audioconvert ! lame ! rtpmpapay ! rtpmpadepay ! mad ! audioconvert ! jackaudiosink connect=0 sync=false

