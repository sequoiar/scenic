#!/bin/bash
if [ $# = 2 ]; then
a=$2
let "b=a+1"
let "c=a+2"
let "d=a+3"
let "e=a+4"
let "f=a+5"
let "g=a+6"
let "h=a+7"

echo $a $b $c $d $e $f $g $h

# server
# gst-launch -v udpsrc port=10001 ! rtpmpadepay ! queue ! mad ! audioconvert ! alsasink sync=false

#gst-launch audiotestsrc ! audioconvert   \
#    ! lame  ! \
#    rtpmpapay ! udpsink host=$1 port=$2 

#gst-launch -v audiotestsrc ! \
#    tee name=t1 ! queue ! tee name=t2 ! queue ! tee name=t3 ! queue ! tee name=t4 ! queue ! tee name=t5 ! queue ! tee name=t6 ! queue ! tee name=t7 \
#    ! lame vbr=new vbr-quality=2 !  rtpmpapay ! udpsink host=$1 port=$2 \
#    t1. ! queue ! lame vbr=new vbr-quality=2 ! rtpmpapay ! udpsink host=$1 port=$b \
#    t2. ! queue ! lame vbr=new vbr-quality=2 ! rtpmpapay ! udpsink host=$1 port=$c \
#    t3. ! queue ! lame vbr=new vbr-quality=2 ! rtpmpapay ! udpsink host=$1 port=$d \
#    t4. ! queue ! lame vbr=new vbr-quality=2 ! rtpmpapay ! udpsink host=$1 port=$e \
#    t5. ! queue ! lame vbr=new vbr-quality=2 ! rtpmpapay ! udpsink host=$1 port=$f \
#    t6. ! queue ! lame vbr=new vbr-quality=2 ! rtpmpapay ! udpsink host=$1 port=$g \
#    t7. ! queue ! lame vbr=new vbr-quality=2 ! rtpmpapay ! udpsink host=$1 port=$h


gst-launch -v gstrtpbin name=rtpbin \
              audiotestsrc is-live=true ! lame ! rtpmpapay ! rtpbin.send_rtp_sink_0                  \
            rtpbin.send_rtp_src_0 ! udpsink host=$1 port=$a                             \
            rtpbin.send_rtcp_src_0 ! udpsink host=$1 port=$b sync=false async=false     \
            udpsrc port=$c ! rtpbin.recv_rtcp_sink_0                                    \
              audiotestsrc is-live=true ! lame ! rtpmpapay ! rtpbin.send_rtp_sink_1                  \
            rtpbin.send_rtp_src_1 ! udpsink host=$1 port=$d                             \
            rtpbin.send_rtcp_src_1 ! udpsink host=$1 port=$e sync=false async=false     \
            udpsrc port=$f ! rtpbin.recv_rtcp_sink_1                                




else
    echo "$0" requires host ip and starting port
fi
