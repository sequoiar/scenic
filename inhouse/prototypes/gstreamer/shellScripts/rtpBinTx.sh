gst-launch --verbose gstrtpbin name=rtpbin \
        filesrc location="heyPredator.avi" ! decodebin ! ffmpegcolorspace ! x264enc bitrate=2048 byte-stream=true threads=4 \
    ! rtph264pay ! rtpbin.send_rtp_sink_0 \
                  rtpbin.send_rtp_src_0 ! udpsink port=5000                            \
                  rtpbin.send_rtcp_src_0 ! udpsink port=5001 sync=false async=false    \
                  udpsrc port=5005 ! rtpbin.recv_rtcp_sink_0                           
