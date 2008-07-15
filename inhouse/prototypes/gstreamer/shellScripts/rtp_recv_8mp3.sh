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
            udpsrc caps="application/x-rtp,media=(string)audio,clock-rate=(int)90000,encoding-name=(string)MPA,ssrc=(guint)2316751886, payload=(int)96, clock-base=(guint)1106648936, seqnum-base=(guint)19537"  port=$a ! rtpbin.recv_rtp_sink_0      \
            rtpbin. ! rtpmpadepay ! mad ! alsasink sync=false                           \
            udpsrc port=$b ! rtpbin.recv_rtcp_sink_0                                    \
            rtpbin.send_rtcp_src_0 ! udpsink host=$1 port=$c                            \
            udpsrc caps="application/x-rtp,media=(string)audio,clock-rate=(int)90000,encoding-name=(string)MPA, ssrc=(guint)2657808050, payload=(int)96, clock-base=(guint)1780013254, seqnum-base=(guint)56362"  port=$d ! rtpbin.recv_rtp_sink_1      \
            rtpbin. ! rtpmpadepay ! mad ! alsasink sync=false                          \
            udpsrc port=$e ! rtpbin.recv_rtcp_sink_1                                    \
            rtpbin.send_rtcp_src_1 ! udpsink host=$1 port=$f





else
    echo "$0" requires host ip and starting port
fi
