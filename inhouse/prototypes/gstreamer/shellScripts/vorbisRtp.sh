#!/bin/bash

gst-launch jackaudiosrc connect=0 ! audioconvert ! vorbisenc ! rtpvorbispay ! rtpvorbisdepay ! vorbisdec ! audioconvert ! jackaudiosink connect=0 sync=false

