gst-launch-0.10 -v interleave name=i ! audioconvert  ! audioresample ! queue ! jackaudiosink \
 audiotestsrc volume=0.125 freq=200 is-live=true ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.125 freq=300 is-live=true ! audioconvert  ! queue ! i. \
 audiotestsrc volume=0.125 freq=500 is-live=true ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.125 freq=700 is-live=true ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.125 freq=900 is-live=true ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.125 freq=1100 is-live=true ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.125 freq=1300 is-live=true ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.125 freq=1400 is-live=true ! audioconvert ! queue ! i. 
