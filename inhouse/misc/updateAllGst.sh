#!/bin/bash

modules=( gstreamer gst-plugins-base gst-plugins-good gst-plugins-bad gst-plugins-ugly gst-ffmpeg )

for module in ${modules[@]}
do
./updateGst.sh $module
done

