#!/bin/bash

gst-launch -v jackaudiosrc connect=0 ! audioconvert ! rtpL16pay ! rtpL16depay ! audioconvert ! jackaudiosink connect=0 sync=false 


#gst-launch -v filesrc location="/home/tristan/Desktop/test_signal8.wav" ! decodebin ! audioconvert ! rtpL16pay ! rtpL16depay ! rtpL16depay ! audioconvert ! jackaudiosink connect=0 sync=false 
