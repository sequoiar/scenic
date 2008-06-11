gst-launch-0.10 -v interleave name=i ! audioconvert ! "audio/x-raw-float, channels=8" ! vorbisenc ! rtpvorbispay ! udpsink host=localhost port=5060 \
audiotestsrc volume=0.5 freq=200 is-live=false ! audioconvert ! queue ! i. \
audiotestsrc volume=0.5 freq=300 is-live=false ! audioconvert ! queue ! i. \
audiotestsrc volume=0.1 freq=400 is-live=false ! audioconvert ! queue ! i. \
audiotestsrc volume=0.1 freq=500 is-live=false ! audioconvert ! queue ! i. \
audiotestsrc volume=0.1 freq=600 is-live=false ! audioconvert ! queue ! i. \
audiotestsrc volume=0.1 freq=700 is-live=false ! audioconvert ! queue ! i. \
audiotestsrc volume=0.1 freq=800 is-live=false ! audioconvert ! queue ! i. \
audiotestsrc volume=0.1 freq=900 is-live=false !  audioconvert ! queue ! i. 
