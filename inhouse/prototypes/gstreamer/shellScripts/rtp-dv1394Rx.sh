
gst-launch -v udpsrc port=10010 ! capsfilter caps="application/x-rtp" ! \
            rtpdvdepay ! dvdemux name=demux demux. ! \
             queue ! dvdec ! xvimagesink sync=false demux. ! queue ! \
             audioconvert ! fakesink
