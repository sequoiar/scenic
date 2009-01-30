#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Sropulpof is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Sropulpof.  If not, see <http:#www.gnu.org/licenses/>.
#

import sys

try:
    import gst
except ImportError:
    print "import gst failed, gst-python must be installed before running this script"
    sys.exit(0)

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

missingPlugins = []

for plug in GST_PLUGINS:
    if gst.element_factory_find(plug) is None: 
        print "Error: plugin " + plug + " is NOT installed"
        missingPlugins.append(plug)
    else:
        print plug + " installed"

print "-------------------------------"
if missingPlugins == []:
    print "All necessary plugins installed"
else:
    print "The following gstreamer plugins need to be installed: "
    for plug in missingPlugins:
        print plug
    print "You may have to install the corresponding development headers (i.e. lib<MODULE>-dev) " 
    print "before building the missing gstreamer plugins"
        
