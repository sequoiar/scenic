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

class Profile(object):
    """ Holds codec name, encoder names, payloader name and caps string """
    filename = 'caps.txt'
    def __init__(self, codec, encoder, payloader):
        self.codec = codec
        self.encoder = encoder
        self.payloader = payloader
        self.caps = ''
    def __str__(self):
        return self.caps

class VideoProfile(Profile):
    """ Holds codec name, encoder names, payloader name and caps string """
    def __init__(self, codec, encoder, payloader, width=640, height=480):
        Profile.__init__(self, codec, encoder, payloader)
        self.width = width
        self.height = height
        self.src = "videotestsrc ! video/x-raw-yuv, width=%d, height=%d" % (self.width, self.height) 



class AudioProfile(Profile):
    """ Holds codec name, encoder names, payloader name, num channels and caps string """

    def __init__(self, codec, encoder, payloader, channels=2, rate=48000):
        Profile.__init__(self, codec, encoder, payloader)
        self.channels = channels
        self.rate = rate
        self.src = "audiotestsrc ! audio/x-raw-int, channels=%d, rate=%d ! audioconvert " % (self.channels, self.rate)


def generate_caps(profiles):
    """ Generate caps for a set of profiles """
    for profile_name, profile in profiles.iteritems():
        #print ('/*----------------------------------------------*/' )
        #print ('CAPS FOR CODEC ' + codecName + ':')
        launch_line = profile.src + " ! %s ! %s name=payloader ! fakesink" \
        % (profile.encoder, profile.payloader)
        pipeline = gst.parse_launch(launch_line)
        pipeline.set_state(gst.STATE_PLAYING)
        mainloop = gobject.MainLoop()

        payloader = pipeline.get_by_name("payloader")
        srcpad = payloader.get_static_pad("src")

        caps = srcpad.get_negotiated_caps()

        while caps is None:
            caps = srcpad.get_negotiated_caps()
            
        profile.caps = caps.to_string().split(', ssrc')[0].strip()

        #print codec
        pipeline.set_state(gst.STATE_NULL)

    return profiles 


def save_caps(profiles, filename):
    """ Write codec/caps dict to file <filename> """
    try:
        """ Write to file """
        file = open(filename, 'w')

        for profileName, profile in profiles.iteritems():
                file.write(profileName + '\n')
                file.write(profile.caps + '\n\n\n')

    except IOError, e:
        sys.stderr.write(e)
    finally:
            file.close()


profiles = {
    'theora' : VideoProfile('theora', 'theoraenc', 'rtptheorapay'), 
    'mpeg4'  : VideoProfile('mpeg4', 'ffenc_mpeg4','rtpmp4vpay'),
    'h264'   : VideoProfile('h264', 'x264enc', 'rtph264pay'),
    'h263'   : VideoProfile('h263', 'ffenc_h263p', 'rtph263ppay'),
    'vorbis' : AudioProfile('vorbis', 'vorbisenc', 'rtpvorbispay'), 
    'mp3'    : AudioProfile('mp3', 'lame', 'rtpmpapay'),
    'raw'    : AudioProfile('raw', 'identity', 'rtpL16pay')
}


profiles = generate_caps(profiles)
save_caps(profiles, Profile.filename)

