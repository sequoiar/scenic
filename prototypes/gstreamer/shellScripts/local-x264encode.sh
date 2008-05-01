#!/bin/bash
gst-launch-0.10 filesrc location = "test.dv" ! dvdemux ! dvdec ! ffmpegcolorspace ! x264enc byte-stream=true threads=4 ! filesink location="r.264"
