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

pidRx = 0
pidTx = 0

class Arg(object): # new style!!
    """ Base class for our argument classes """
    def __init__(self):
        self.address = '127.0.0.1'   # always need this guy
        self.timeout = 10000
    
    def __str__(self):
        result = ''
        for k, v in self.__dict__.iteritems():
            if v is True:
                v = ''
            result = result + ' --' + k + ' ' + str(v) # get list of members, properly formatted
        return result


class VideoArg(Arg):
    """ Base class for our video argument classes """
    def __init__(self):
        Arg.__init__(self)
        self.videocodec = 'mpeg4'
        self.videoport = 11000


class AudioArg(Arg):
    """ Base class for our Audio argument classes """
    def __init__(self):
        Arg.__init__(self)
        self.audiocodec = 'raw'
        self.audioport = 10000


class VideoTxArg(VideoArg):
    """ Class for video only sending args """
    def __init__(self):
        VideoArg.__init__(self)
        self.videosource = 'v4l2src'


class VideoRxArg(VideoArg):
    """ Class for video only receiving args """
    def __init__(self):
        VideoArg.__init__(self)
        self.screen = 0
        self.videosink = 'xvimagesink'


class AudioTxArg(AudioArg):
    """ Class for audio only sending args """
    def __init__(self):
        AudioArg.__init__(self)
        self.audiosource = 'jackaudiosrc'
        self.numchannels = 8


class AudioRxArg(AudioArg):
    """ Class for audio only receiving args """
    def __init__(self):
        AudioArg.__init__(self)
        self.audiosink = 'jackaudiosink'


class AudioVideoRxArg(AudioRxArg, VideoRxArg):
    """ Class for audio and video receiving args """
    def __init__(self):
        AudioRxArg.__init__(self)
        VideoRxArg.__init__(self)


class AudioVideoTxArg(AudioTxArg, VideoTxArg):
    """ Class for audio and video sending args """
    def __init__(self):
        AudioTxArg.__init__(self)
        VideoTxArg.__init__(self)


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
            return AudioRxArg(), AudioTxArg()
        elif argtype is 'video':
            return VideoRxArg(), VideoTxArg()
        elif argtype is 'audiovideo':
            return AudioVideoRxArg(), AudioVideoTxArg()
        else:
            raise Exception('unexpected argtype ' + argtype)

    @staticmethod
    def receiveInterrupt(signum, stack):
        print 'Received interrupt, killing all child processes'
        try: 
            os.kill(pidRx, signal.SIGKILL)
        except OSError:
            pass
        try:
            os.kill(pidTx, signal.SIGKILL)
        except OSError:
            pass
        sys.exit(0)

    @staticmethod
    def runTest(rx, tx):
        """ This method is used by our helpers to create a receiver
            process and a sender process, and wait on them. """
        pidRx = os.fork()

        if pidRx == 0:
            os.system('milhouse -r ' +  str(rx))
            sys.exit(0)
        else:
            # parent
            pidTx = os.fork()
            if pidTx == 0:
                os.system('milhouse -s ' +  str(tx))
                sys.exit(0)
            else:
                # parent
                os.waitpid(pidTx, 0)
                print 'END OF TEST'

    def test_01_defaults(self):
        """ Test with default args and 5 second timeout """
        self.countdown('START')

        rx, tx= self.argfactory('audiovideo')
        self.runTest(rx, tx)

    def test_02_jack(self):
        """ Test with 1-8 channels and 5 second timeout for jack """
        self.countdown('START')

        rx, tx= self.argfactory('audio')
        for c in xrange(1, 9): 
            tx.numchannels = c
            self.runTest(rx, tx) 
    
    def test_03_dv(self):
        """ Test dv inputs """
        self.countdown('START')

        rx, tx= self.argfactory('audiovideo')
        tx.videosource = 'dv1394src'
        tx.audiosource = 'dv1394src'
        self.runTest(rx, tx)
   
    def test_04_alsa(self):
        """ Test with 1-8 channels for alsa with a 5 second timeout """
        self.countdown('STOP')

        rx, tx = self.argfactory('audio')
        tx.audiosource = 'alsasrc'
        rx.audiosink = 'alsasink'
        for c in xrange(1, 9): 
            tx.numchannels = c
            self.runTest(rx, tx)

    def test_05_pulse(self):
        """ Test with 1-6 channels for pulse with a 5 second timeout """
        self.countdown('STOP')

        rx, tx= self.argfactory('audio')
        tx.audiosource = 'pulsesrc'
        rx.audiosink = 'pulsesink'
        for c in xrange(1, 9): 
            tx.numchannels = c
            self.runTest(rx, tx)


    def test_06_vorbis(self):
        """ Test with 1-8 channels for vorbis with jack with a 5 second timeout """
        self.countdown('START')

        audiocodec = 'vorbis'
        rx, tx = self.argfactory('audio')
        tx.audiocodec = audiocodec
        rx.audiocodec = audiocodec
        for c in xrange(1, 9): 
            tx.numchannels = c
            self.runTest(rx, tx)

    def test_07_dv_vorbis(self):
        """ Test with 1-8 channels for vorbis with dv and jack with a 5 second timeout """
        self.countdown('START')

        audiocodec = 'vorbis'
        
        rx, tx = self.argfactory('audiovideo')
        tx.audiocodec = audiocodec
        tx.videosource = 'dv1394src'
        rx.audiocodec = audiocodec
        for c in xrange(1, 9): 
            tx.numchannels = c
            self.runTest(rx, tx)
    
    def test_08_videotestsrc_h264(self):
        """ Test h264 with videotestsrc """

        rx, tx = self.argfactory('video')
        tx.videosource = 'videotestsrc'
        videocodec = 'h264'
        tx.videocodec = videocodec
        rx.videocodec = videocodec
        self.runTest(rx, tx) 
        
    def test_09_testsrc_glimagesink(self):
        """ Test glimagesink """

        rx, tx = self.argfactory('video')
        tx.videosource = 'videotestsrc'
        rx.videosink = 'glimagesink'
        self.runTest(rx, tx)

    def test_12_ximagesink(self):
        """ Test with ximagesink"""
        rx, tx = self.argfactory('video')

        rx.videosink = 'ximagesink'
        self.runTest(rx, tx)
    
    def test_13_glimagesink(self):
        """ Test with glimagesink """

        rx, tx = self.argfactory('video')
        rx.videosink = 'glimagesink'
        self.runTest(rx, tx)

    def test_14_dv1394src_ximagesink(self):
        """ Test dv with ximagesink"""

        rx, tx = self.argfactory('video')
        tx.videosource = 'dv1394src'
        rx.videosink = 'ximagesink'
        self.runTest(rx, tx)

    def test_15_dv1394src_glimagesink(self):
        """ Test dv with glimagesink"""

        rx, tx = self.argfactory('video')
        tx.videosource = 'dv1394src'
        rx.videosink = 'glimagesink'
        self.runTest(rx, tx)

    def test_16_videoOnly_deinterlace(self):
        """ Test with just video deinterlaced """

        rx, tx = self.argfactory('video')
        tx.deinterlace = True
        self.runTest(rx, tx)

    def test_17_videoOnly_deinterlace_glimagesink(self):
        """ Test with just video deinterlaced to glimagesink """

        rx, tx = self.argfactory('video')
        tx.deinterlace = True
        rx.videosink = 'glimagesink'
        self.runTest(rx, tx)

    def test_18_videotestsrc(self):
        """ Test videotestsrc """

        rx, tx = self.argfactory('video')
        tx.videosource = 'videotestsrc'
        self.runTest(rx, tx)


# here we run all the tests thanks to the wonders of reflective programming
tests = prefixedMethods(MilhouseTests(), 'test_16')

for test in tests:
    print 'TEST: '  + test.__doc__
    test()
    
