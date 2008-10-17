#!/bin/bash

gst-launch jackaudiosrc connect=0 ! audioconvert ! rtpL16pay ! rtpL16depay ! audioconvert ! jackaudiosink connect=0 sync=false

