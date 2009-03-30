#!/usr/bin/env python
# -*- coding: utf-8 -*-
# thrillHouse.py
# Copyright (C) 2008-2009 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is part of [propulse]ART.
#
# [propulse]ART is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# [propulse]ART is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
#

import sys, os

import signal
import time

from twisted.python.reflect import prefixedMethods

pidrecv = 0
pidsend = 0

class Arg(object): # new style!!
    """ Base class for our argument classes """
    def __init__(self):
        """ Init with address and timeout defaults """
        self.address = '127.0.0.1'   # always need this guy
        self.timeout = 10000
    
    def __str__(self):
        """ Returns a list of this class' data members and their values, 
        formatted as command line arguments """
        result = ''
        for k, v in self.__dict__.iteritems():
            if v is True:
                v = ''  # boolean members need only have keys be present, value is always true
            result = result + ' --' + k + ' ' + str(v) 
        return result


class VideoArg(Arg):
    """ Base class for our video argument classes """
    def __init__(self):
        """ Defaults are mpeg4 videocodec and 11000 for videoport """
        Arg.__init__(self)
        self.videocodec = 'mpeg4'
        self.videoport = 11000


class AudioArg(Arg):
    """ Base class for our Audio argument classes """
    def __init__(self):
        """ Defaults are uncompressed (raw) audio and 10000 for audioport """
        Arg.__init__(self)
        self.audiocodec = 'raw'
        self.audioport = 10000


class VideosendArg(VideoArg):
    """ Class for video only sending args """
    def __init__(self):
        """ Default for videosource is v4l2src, the video4linux2 plugin """
        VideoArg.__init__(self)
        self.videosource = 'v4l2src'


class VideorecvArg(VideoArg):
    """ Class for video only receiving args """
    def __init__(self):
        """ Default for videosink is xvimagesink, the xvideo output plugin """
        VideoArg.__init__(self)
        self.screen = 0
        self.videosink = 'xvimagesink'


class AudiosendArg(AudioArg):
    """ Class for audio only sending args """
    def __init__(self):
        """ Default for audiosrc is 8 channel jackaudiosrc, the jack input plugin """
        AudioArg.__init__(self)
        self.audiosource = 'jackaudiosrc'
        self.numchannels = 8


class AudiorecvArg(AudioArg):
    """ Class for audio only receiving args """
    def __init__(self):
        """ Default for audiosink is jackaudiosink, the jack output plugin """
        AudioArg.__init__(self)
        self.audiosink = 'jackaudiosink'


class AudioVideorecvArg(AudiorecvArg, VideorecvArg):
    """ Class for audio and video receiving args """
    def __init__(self):
        AudiorecvArg.__init__(self)
        VideorecvArg.__init__(self)


class AudioVideosendArg(AudiosendArg, VideosendArg):
    """ Class for audio and video sending args """
    def __init__(self):
        AudiosendArg.__init__(self)
        VideosendArg.__init__(self)


class MilhouseTests():
    def __init__(self):
        signal.signal(signal.SIGINT, self.receiveInterrupt)

    @staticmethod
    def countdown(warning):
        countdown = 1
        while countdown > 0:
            print 'PLEASE ' + warning + ' JACK SERVER NOW, YOU HAVE ' + str(countdown) + ' SECONDS' 
            time.sleep(1)
            countdown -= 1

    @staticmethod
    def argfactory(argtype):
        """Returns default send and receive args"""

        if argtype is 'audio':
            return AudiorecvArg(), AudiosendArg()
        elif argtype is 'video':
            return VideorecvArg(), VideosendArg()
        elif argtype is 'audiovideo':
            return AudioVideorecvArg(), AudioVideosendArg()
        else:
            raise Exception('unexpected argtype ' + argtype)

    @staticmethod
    def receiveInterrupt(signum, stack):
        print 'Received interrupt, killing all child processes'
        try: 
            os.kill(pidrecv, signal.SIGKILL)
        except OSError:
            pass
        try:
            os.kill(pidsend, signal.SIGKILL)
        except OSError:
            pass
        sys.exit(0)

    @staticmethod
    def runTest(recv, send):
        """ This method is used by our helpers to create a receiver
            process and a sender process, and wait on them. """
        pidrecv = os.fork()

        if pidrecv == 0:
            os.system('milhouse -r ' +  str(recv))
            sys.exit(0)
        else:
            # parent
            pidsend = os.fork()
            if pidsend == 0:
                os.system('milhouse -s ' +  str(send))
                sys.exit(0)
            else:
                # parent
                os.waitpid(pidsend, 0)
                print 'END OF TEST'

    def test_01_defaults(self):
        """ Test with default args and 5 second timeout """
        self.countdown('START')

        recv, send= self.argfactory('audiovideo')
        self.runTest(recv, send)

    def test_02_jack(self):
        """ Test with 1-8 channels and 5 second timeout for jack """
        self.countdown('START')

        recv, send= self.argfactory('audio')
        for c in xrange(1, 9): 
            send.numchannels = c
            self.runTest(recv, send) 
    
    def test_03_dv(self):
        """ Test dv inputs """
        self.countdown('START')

        recv, send= self.argfactory('audiovideo')
        send.videosource = 'dv1394src'
        send.audiosource = 'dv1394src'
        self.runTest(recv, send)
   
    def test_04_alsa(self):
        """ Test with 1-8 channels for alsa with a 5 second timeout """
        self.countdown('STOP')

        recv, send = self.argfactory('audio')
        send.audiosource = 'alsasrc'
        recv.audiosink = 'alsasink'
        for c in xrange(1, 9): 
            send.numchannels = c
            self.runTest(recv, send)

    def test_05_pulse(self):
        """ Test with 1-6 channels for pulse with a 5 second timeout """
        self.countdown('STOP')

        recv, send= self.argfactory('audio')
        send.audiosource = 'pulsesrc'
        recv.audiosink = 'pulsesink'
        for c in xrange(1, 9): 
            send.numchannels = c
            self.runTest(recv, send)


    def test_06_vorbis(self):
        """ Test with 1-8 channels for vorbis with jack with a 5 second timeout """
        self.countdown('START')

        audiocodec = 'vorbis'
        recv, send = self.argfactory('audio')
        send.audiocodec = audiocodec
        recv.audiocodec = audiocodec
        for c in xrange(1, 9): 
            send.numchannels = c
            self.runTest(recv, send)

    def test_07_dv_vorbis(self):
        """ Test with 1-8 channels for vorbis with dv and jack with a 5 second timeout """
        self.countdown('START')

        audiocodec = 'vorbis'
        
        recv, send = self.argfactory('audiovideo')
        send.audiocodec = audiocodec
        send.videosource = 'dv1394src'
        recv.audiocodec = audiocodec
        for c in xrange(1, 9): 
            send.numchannels = c
            self.runTest(recv, send)
    
    def test_08_videotestsrc_h264(self):
        """ Test h264 with videotestsrc """

        recv, send = self.argfactory('video')
        send.videosource = 'videotestsrc'
        videocodec = 'h264'
        send.videocodec = videocodec
        recv.videocodec = videocodec
        self.runTest(recv, send) 
        
    def test_09_testsrc_glimagesink(self):
        """ Test glimagesink """

        recv, send = self.argfactory('video')
        send.videosource = 'videotestsrc'
        recv.videosink = 'glimagesink'
        self.runTest(recv, send)

    def test_12_ximagesink(self):
        """ Test with ximagesink"""
        recv, send = self.argfactory('video')

        recv.videosink = 'ximagesink'
        self.runTest(recv, send)
    
    def test_13_glimagesink(self):
        """ Test with glimagesink """

        recv, send = self.argfactory('video')
        recv.videosink = 'glimagesink'
        self.runTest(recv, send)

    def test_14_dv1394src_ximagesink(self):
        """ Test dv with ximagesink"""

        recv, send = self.argfactory('video')
        send.videosource = 'dv1394src'
        recv.videosink = 'ximagesink'
        self.runTest(recv, send)

    def test_15_dv1394src_glimagesink(self):
        """ Test dv with glimagesink"""

        recv, send = self.argfactory('video')
        send.videosource = 'dv1394src'
        recv.videosink = 'glimagesink'
        self.runTest(recv, send)

    def test_16_videoOnly_deinterlace(self):
        """ Test with just video deinterlaced """

        recv, send = self.argfactory('video')
        send.deinterlace = True
        self.runTest(recv, send)

    def test_17_videoOnly_deinterlace_glimagesink(self):
        """ Test with just video deinterlaced to glimagesink """

        recv, send = self.argfactory('video')
        send.deinterlace = True
        recv.videosink = 'glimagesink'
        self.runTest(recv, send)

    def test_18_videotestsrc(self):
        """ Test videotestsrc """

        recv, send = self.argfactory('video')
        send.videosource = 'videotestsrc'
        self.runTest(recv, send)


# here we run all the tests thanks to the wonders of reflective programming
tests = prefixedMethods(MilhouseTests(), 'test_16')

for test in tests:
    print 'TEST: '  + test.__doc__
    test()
    
