gst-launch-0.10 -v udpsrc port=5060 ! rtpvorbisdepay ! vorbisdec ! jackaudiosink
