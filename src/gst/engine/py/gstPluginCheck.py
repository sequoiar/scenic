#!/usr/bin/env python

import os
import gst



GST_PLUGINS = ['level', 'audioconvert', 'alsasink', 'pulsesink', 'jackaudiosink', 
            'decodebin', 'capsfilter', 'dvdemux', 'queue', 'ffmpegcolorspace', 
            'x264enc', 'ffdec_h264', 'ffdec_h263', 'ffenc_h263', 'ffenc_mpeg4', 
            'ffdec_mpeg4', 'vorbisenc', 'vorbisdec', 'lame', 'mad', 'glupload', 
            'glimagesink', 'interleave', 'gstrtpbin', 'rtph264pay', 'rtph264depay', 
            'rtph263pay', 'rtph263depay', 'rtpmp4vpay', 'rtpmp4vdepay', 
            'rtpL16pay', 'rtpL16depay', 'rtpmpapay', 'rtpmpadepay', 'udpsrc', 'udpsink', 
            'xvimagesink', 'ximagesink', 'dvdec', 'audiotestsrc', 'filesrc', 'alsasrc', 
            'jackaudiosrc', 'dv1394src', 'pulsesrc', 'jackaudiosink', 'alsasink', 'pulsesink', 
            'videotestsrc', 'v4l2src', 'rtpvorbispay', 'rtpvorbisdepay']


for plug in GST_PLUGINS:
    if gst.element_factory_find(plug) is None: 
        raise gst.PluginNotFoundError("Gstreamer Plugin " + plug + " is not found")
    else:
        print plug + " installed"

    

print "-------------------------------"
print "All necessary plugins installed"
