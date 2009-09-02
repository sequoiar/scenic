#!/usr/bin/env python
# -*- coding: utf-8 -*-

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


try:
    import pygst
    pygst.require('0.10')
    import gst
except ImportError:
    print("import failed, please install gst-python")
    sys.exit(1)

import gobject

WIDTH = 640
HEIGHT = 480

class CodecData(object):
    """ Holds codec name, encoder/decoder names, payloader name and caps string """
    def __init__(self, codec, encoder, decoder, payloader):
        self.codec = codec
        self.encoder = encoder
        self.decoder = decoder
        self.payloader = payloader
        self.caps = ''
    def __str__(self):
        return self.caps


codec_dict = {
    'theora' : CodecData('theora', 'theoraenc', 'theoradec', 'rtptheorapay'), 
    'mpeg4'  : CodecData('mpeg4', 'ffenc_mpeg4', 'ffdec_mpeg4', 'rtpmp4vpay'),
    'h264'   : CodecData('h264', 'x264enc', 'ffdec_h264', 'rtph264pay'),
    'h263'   : CodecData('h263', 'ffenc_h263p', 'ffdec_h263p', 'rtph263ppay')
}


for codecName, codec in codec_dict.iteritems():
    print '/*----------------------------------------------*/' 
    print 'CAPS FOR CODEC ' + codecName + ':'
    launch_line = "v4l2src ! video/x-raw-yuv, width=%d, height=%d ! %s ! %s name=payloader ! fakesink" \
    % (WIDTH, HEIGHT, codec.encoder, codec.payloader)
    pipeline = gst.parse_launch(launch_line)
    pipeline.set_state(gst.STATE_PLAYING)
    mainloop = gobject.MainLoop()

    payloader = pipeline.get_by_name("payloader")
    srcpad = payloader.get_static_pad("src")

    caps = srcpad.get_negotiated_caps()

    while caps is None:
        caps = srcpad.get_negotiated_caps()
        
    codec.caps = caps.to_string().split(', ssrc')[0].strip()
    print codec
    pipeline.set_state(gst.STATE_NULL)
