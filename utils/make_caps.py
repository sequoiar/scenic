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

codec_dict = {'theoraenc':'rtptheorapay','ffenc_mpeg4':'rtpmp4vpay','x264enc':'rtph264pay','ffenc_h263p':'rtph263ppay'}

for codec, pay in codec_dict.iteritems():
    print '/*----------------------------------------------*/' 
    print 'CAPS FOR ENCODER ' + codec + ':'
    launch_line = "v4l2src ! video/x-raw-yuv, width=%d, height=%d ! %s ! %s name=payloader ! fakesink" \
    % (WIDTH, HEIGHT, codec, pay)
    pipeline = gst.parse_launch(launch_line)
    pipeline.set_state(gst.STATE_PLAYING)
    mainloop = gobject.MainLoop()

    payloader = pipeline.get_by_name("payloader")
    srcpad = payloader.get_static_pad("src")

    caps = srcpad.get_negotiated_caps()

    while caps is None:
        caps = srcpad.get_negotiated_caps()
        
    print caps.to_string()
    pipeline.set_state(gst.STATE_NULL)
