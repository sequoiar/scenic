gst-launch-0.10 udpsrc port=10010 caps="application/x-rtp" ! rtph264depay \ 
! ffdec_h264 ! xvimagesink sync=false

