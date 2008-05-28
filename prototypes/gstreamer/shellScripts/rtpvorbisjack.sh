#!/bin/sh

# FIXME: Only supports 6 channels, is advertised as supporting 8

gst-launch-0.10 -v interleave name=i ! vorbisenc ! rtpvorbispay ! rtpvorbisdepay ! vorbisdec ! jackaudiosink \
 audiotestsrc volume=0.5 freq=200 is-live=false ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.1 freq=300 is-live=false ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.1 freq=500 is-live=false ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.1 freq=700 is-live=false ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.1 freq=900 is-live=false ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.1 freq=1100 is-live=false ! audioconvert ! queue ! i. 
#  audiotestsrc volume=0.1 freq=1300 is-live=false ! audioconvert ! queue ! i. \
#  audiotestsrc volume=0.1 freq=1400 is-live=false ! audioconvert ! queue ! i. 
