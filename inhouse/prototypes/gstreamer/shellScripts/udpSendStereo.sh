gst-launch-0.10 -v interleave name=i ! vorbisenc ! rtpvorbispay ! udpsink host=localhost port=5060 \
audiotestsrc volume=0.5 freq=200 is-live=false ! audioconvert ! queue ! i. \
audiotestsrc volume=0.5 freq=400 is-live=false ! audioconvert ! queue ! i. 
