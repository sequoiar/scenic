if [ $# = 2 ]; then
    gst-launch-0.10 dv1394src ! dvdemux ! dvdec ! ffmpegcolorspace ! x264enc byte-stream=true threads=4 ! rtph264pay ! udpsink host=$1 port=$2
else
    echo "$0" requires host ip and port
fi

