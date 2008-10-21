#!/bin/bash

gst-launch jackaudiosrc connect=0 ! audioconvert ! vorbisenc ! rtpvorbispay max-ptime=20000000 ! rtpvorbisdepay ! vorbisdec ! audioconvert ! jackaudiosink connect=0 sync=false

