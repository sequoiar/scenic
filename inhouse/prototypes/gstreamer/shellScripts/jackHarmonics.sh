#!/bin/sh

gst-launch -v audiotestsrc volume=0.5 freq=100 ! audioconvert ! queue ! jackaudiosink connect=0 \
              audiotestsrc volume=0.1 freq=200 ! audioconvert ! queue ! jackaudiosink connect=0 \
              audiotestsrc volume=0.1 freq=300 ! audioconvert ! queue ! jackaudiosink connect=0 \
              audiotestsrc volume=0.1 freq=500 ! audioconvert ! queue ! jackaudiosink connect=0 \
              audiotestsrc volume=0.1 freq=700 ! audioconvert ! queue ! jackaudiosink connect=0 \
              audiotestsrc volume=0.4 freq=900 ! audioconvert ! queue ! jackaudiosink connect=0 \
              audiotestsrc volume=0.4 freq=1100 ! audioconvert ! queue ! jackaudiosink connect=0 \
              audiotestsrc volume=0.4 freq=1300 ! audioconvert ! queue ! jackaudiosink connect=0
