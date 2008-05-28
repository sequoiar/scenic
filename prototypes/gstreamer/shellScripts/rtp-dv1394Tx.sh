#!/bin/bash

#gst-launch -v dv1394src ! dvdemux ! dvdec ! ffmpegcolorspace ! ffenc_h263p ! rtph263ppay ! udpsink host=192.168.1.217 port=10010
#gst-launch -v dv1394src !  dvdemux ! rtpdvpay ! udpsink host=192.168.1.217 port=10010
gst-launch -v dv1394src ! rtpdvpay ! udpsink host=localhost port=10010 

