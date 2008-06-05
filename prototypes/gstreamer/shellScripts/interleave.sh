gst-launch-0.10 -v interleave name=i ! audioconvert  ! queue ! alsasink \
 audiotestsrc volume=0.5 freq=200 is-live=true ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.1 freq=300 is-live=true ! audioconvert  ! queue ! i. \
 audiotestsrc volume=0.1 freq=500 is-live=true ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.1 freq=700 is-live=true ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.1 freq=900 is-live=true ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.4 freq=1100 is-live=true ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.4 freq=1300 is-live=true ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.4 freq=1400 is-live=true ! audioconvert ! queue ! i. 
