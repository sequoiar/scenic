gst-launch-0.10 -v interleave name=i ! audioconvert ! alsasink \
 audiotestsrc volume=0.5 freq=200 ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.1 freq=300 ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.1 freq=500 ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.1 freq=700 ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.1 freq=900 ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.4 freq=1100 ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.4 freq=1300 ! audioconvert ! queue ! i. \
 audiotestsrc volume=0.4 freq=1400 ! audioconvert ! queue ! i. \
