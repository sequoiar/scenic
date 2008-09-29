gst-launch --verbose gstrtpbin name=rtpbin \
        filesrc location="heyPredator.avi" ! decodebin ! ffmpegcolorspace ! x264enc bitrate=2048 byte-stream=true threads=4 \
    ! rtph264pay ! rtpbin.send_rtp_sink_0 \
                  rtpbin.send_rtp_src_0 ! udpsink port=10010
                  rtpbin.send_rtcp_src_0 ! udpsink port=10011 sync=false async=false    \
                  udpsrc port=10015 ! rtpbin.recv_rtcp_sink_0                           
