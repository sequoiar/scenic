#!/bin/sh

gst-launch -v audiotestsrc volume=0.3 freq=100 wave=sine ! audioconvert  ! alsasink \
              audiotestsrc volume=0.1 freq=300  wave=sine ! audioconvert  ! alsasink \
              audiotestsrc volume=0.1 freq=500 wave=sine ! audioconvert  ! alsasink \
              audiotestsrc volume=0.1 freq=700 wave=sine ! audioconvert  ! alsasink \
              audiotestsrc volume=0.1 freq=900 wave=sine ! audioconvert  ! alsasink \
              audiotestsrc volume=0.1 freq=1100 wave=sine ! audioconvert  ! alsasink 
