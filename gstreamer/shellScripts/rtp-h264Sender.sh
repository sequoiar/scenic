if [ $# = 1 ]; then
    port=10010
    gst-launch-0.10 dv1394src ! dvdemux ! dvdec ! ffmpegcolorspace ! x264enc byte-stream=true threads=4 ! rtph264pay ! udpsink host=$1 port=$port
else
    echo "$0" requires host ip
fi

