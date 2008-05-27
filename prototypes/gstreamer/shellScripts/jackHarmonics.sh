#!/bin/sh

gst-launch -v audiotestsrc volume=0.5 freq=200 wave=sine ! audioconvert ! jackaudiosink connect=0 \
              audiotestsrc volume=0.1 freq=300  wave=sine ! audioconvert ! jackaudiosink connect=0 \
              audiotestsrc volume=0.1 freq=500 wave=sine ! audioconvert ! jackaudiosink connect=0 \
              audiotestsrc volume=0.1 freq=700 wave=sine ! audioconvert ! jackaudiosink connect=0 \
              audiotestsrc volume=0.1 freq=900 wave=sine ! audioconvert ! jackaudiosink connect=0 \
              audiotestsrc volume=0.4 freq=1100 wave=sine ! audioconvert ! jackaudiosink connect=0
