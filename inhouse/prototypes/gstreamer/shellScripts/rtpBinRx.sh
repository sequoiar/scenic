gst-launch -v gstrtpbin name=rtpbin                                          \
                             udpsrc caps="application/x-rtp,media=(string)video,clock-rate=(int)90000,encoding-name=(string)H264" \
                             port=5000 ! rtpbin.recv_rtp_sink_0                                \
                             rtpbin. ! rtph264depay ! ffdec_h264 ! xvimagesink sync=false                   \
                             udpsrc port=5001 ! rtpbin.recv_rtcp_sink_0                               \
                             rtpbin.send_rtcp_src_0 ! udpsink port=5005 sync=false async=false        

