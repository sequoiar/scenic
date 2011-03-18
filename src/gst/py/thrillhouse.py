#!/usr/bin/env python
# -*- coding: utf-8 -*-
# thrillhouse.py
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

"""
Series of tests for the milhouse command line.
"""

import sys, os

import signal
import time

from twisted.python.reflect import prefixedMethods

PID_RECV = 0
PID_SEND = 0

class Arg(object): # new style!!
    """ Base class for our argument classes """
    def __init__(self):
        """ Init with address and timeout defaults """
        self.address = '127.0.0.1'   # always need this guy
        self.timeout = 1000000000
#        self.enable_controls = True
    
    def __str__(self):
        """ Returns a list of this class' data members and their values, 
        formatted as command line arguments """
        result = ''
        for key, val in self.__dict__.iteritems():
            if val is True:
                val = ''  # boolean members don't need values in output string
            # replace underscores from member names with dashes for commandline
            result = result + ' --' + key.replace('_', '-') + ' ' + str(val) 
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


class VideoSendArg(VideoArg):
    """ Class for video only sending args """
    def __init__(self):
        """ Default for videosource is v4l2src, the video4linux2 plugin """
        VideoArg.__init__(self)
        self.videosource = 'v4l2src'


class VideoRecvArg(VideoArg):
    """ Class for video only receiving args """
    def __init__(self):
        """ Default for videosink is xvimagesink, the xvideo output plugin """
        VideoArg.__init__(self)
        self.screen = 0
        self.videosink = 'xvimagesink'


class AudioSendArg(AudioArg):
    """ Class for audio only sending args """
    def __init__(self):
        """ Default for audiosrc is 8 channel jackaudiosrc, 
         the jack input plugin """
        AudioArg.__init__(self)
        self.audiosource = 'jackaudiosrc'
        self.numchannels = 8


class AudioRecvArg(AudioArg):
    """ Class for audio only receiving args """
    def __init__(self):
        """ Default for audiosink is jackaudiosink, the jack output plugin """
        AudioArg.__init__(self)
        self.audiosink = 'jackaudiosink'


class AudioVideoRecvArg(AudioRecvArg, VideoRecvArg):
    """ Class for audio and video receiving args """
    def __init__(self):
        """ Calls init for our parent classes """
        AudioRecvArg.__init__(self)
        VideoRecvArg.__init__(self)


class AudioVideoSendArg(AudioSendArg, VideoSendArg):
    """ Class for audio and video sending args """
    def __init__(self):
        """ Calls init for our parent classes """
        AudioSendArg.__init__(self)
        VideoSendArg.__init__(self)


class MilhouseTests():
    """ Class containing the tests and some helper methods """
    def __init__(self):
        """ Installs interrupt signal handler """
        signal.signal(signal.SIGINT, self.handle_interrupt)

    @staticmethod
    def countdown(warning):
        """ Prints a countdown telling the client to start or stop 
            jack, depending on the input warning """
        countdown = 1
        while countdown > 0:
            print 'PLEASE ' + warning + ' JACK SERVER NOW, YOU HAVE ' \
                + str(countdown) + ' SECONDS' 
            time.sleep(1)
            countdown -= 1

    @staticmethod
    def argfactory(argtype):
        """Returns default send and receive args"""

        if argtype is 'audio':
            return AudioRecvArg(), AudioSendArg()
        elif argtype is 'video':
            return VideoRecvArg(), VideoSendArg()
        elif argtype is 'audiovideo':
            return AudioVideoRecvArg(), AudioVideoSendArg()
        else:
            raise Exception('unexpected argtype ' + argtype)

    @staticmethod
    def handle_interrupt(signum, stack):
        """ Handles SIGINT and kills child processes """
        print 'Got interrupt, killing all child processes'
        try: 
            os.kill(PID_RECV, signal.SIGKILL)
        except OSError:
            print "No process with pid " + PID_RECV
        try:
            os.kill(PID_SEND, signal.SIGKILL)
        except OSError:
            print "No process with pid " + PID_SEND 
        sys.exit(0)

    @staticmethod
    def run(recv, send):
        """ This method is used by our helpers to create a receiver
            process and a sender process, and wait on them. """
        PID_RECV = os.fork()

        if PID_RECV == 0:
            os.system('milhouse -r ' +  str(recv))
            sys.exit(0)
        else:
            # parent
            PID_SEND = os.fork()
            if PID_SEND == 0:
                os.system('milhouse -s ' +  str(send))
                sys.exit(0)
            else:
                # parent
                os.waitpid(PID_SEND, 0)
                print 'END OF TEST'

    def test_01_defaults(self):
        """ Test with default args and 5 second timeout """
        self.countdown('START')

        recv, send = self.argfactory('audiovideo')
        self.run(recv, send)

    def test_02_audio(self):
        """ Test with 1-8 channels and 5 second timeout for jack """
        self.countdown('START')

        recv, send = self.argfactory('audio')
        for chan in xrange(1, 9): 
            send.numchannels = chan
            self.run(recv, send) 
    
    def test_03_dv(self):
        """ Test dv inputs """
        self.countdown('START')

        recv, send = self.argfactory('audiovideo')
        send.videosource = 'dv1394src'
        send.audiosource = 'dv1394src'
        self.run(recv, send)
   
    def test_04_alsa(self):
        """ Test with 1-8 channels for alsa with a 5 second timeout """
        self.countdown('STOP')

        recv, send = self.argfactory('audio')
        send.audiosource = 'alsasrc'
        recv.audiosink = 'alsasink'
        for chan in xrange(1, 9): 
            send.numchannels = chan
            self.run(recv, send)

    def test_05_pulse(self):
        """ Test with 1-6 channels for pulse with a 5 second timeout """
        self.countdown('STOP')

        recv, send = self.argfactory('audio')
        send.audiosource = 'pulsesrc'
        recv.audiosink = 'pulsesink'
        for chan in xrange(1, 9): 
            send.numchannels = chan
            self.run(recv, send)


    def test_06_vorbis(self):
        """ Test with 1-8 channels for vorbis with a 5 second timeout """
        self.countdown('START')

        audiocodec = 'vorbis'
        recv, send = self.argfactory('audio')
        send.audiocodec = audiocodec
        recv.audiocodec = audiocodec
        for chan in xrange(1, 9): 
            send.numchannels = chan
            self.run(recv, send)

    def test_07_dv_vorbis(self):
        """ Test with 1-8 channels for vorbis with dv and jack with a 5 
            second timeout """
        self.countdown('START')

        audiocodec = 'vorbis'
        recv, send = self.argfactory('audiovideo')
        send.audiocodec = audiocodec
        send.videosource = 'dv1394src'
        recv.audiocodec = audiocodec
        for chan in xrange(1, 9): 
            send.numchannels = chan
            self.run(recv, send)
    
    def test_08_videotestsrc_h264(self):
        """ Test h264 with videotestsrc """

        recv, send = self.argfactory('video')
        send.videosource = 'videotestsrc'
        videocodec = 'h264'
        send.videocodec = videocodec
        recv.videocodec = videocodec
        self.run(recv, send) 
        
    def test_12_ximagesink(self):
        """ Test with ximagesink"""
        recv, send = self.argfactory('video')

        recv.videosink = 'ximagesink'
        self.run(recv, send)
    
    def test_14_dv1394src_ximagesink(self):
        """ Test dv with ximagesink """

        recv, send = self.argfactory('video')
        send.videosource = 'dv1394src'
        recv.videosink = 'ximagesink'
        self.run(recv, send)

    def test_16_deinterlace(self):
        """ Test with just video deinterlaced """

        recv, send = self.argfactory('video')
        recv.deinterlace = True
        self.run(recv, send)

    def test_18_videotestsrc(self):
        """ Test videotestsrc """

        recv, send = self.argfactory('video')
        send.videosource = 'videotestsrc'
        self.run(recv, send)

    def test_19_videotestsrc_jack(self):
        """ Test videotestsrc with audio """

        recv, send = self.argfactory('audiovideo')
        send.videosource = 'videotestsrc'
        self.run(recv, send)
    
    def test_20_rtpjitterbuffer(self):
        """ Test videotestsrc with audio """

        recv, send = self.argfactory('audiovideo')
        recv.jitterbuffer = 300
        self.run(recv, send)

    def test_21_videotestsrc_audiotestsrc(self):
        """ Test videotestsrc with audio """

        recv, send = self.argfactory('audiovideo')
        send.videosource = 'videotestsrc'
        send.audiosource = 'audiotestsrc'
        self.run(recv, send)

    def test_22_vorbis_audiotestsrc_v4l2src(self):
        """ Test with 1-8 channels for vorbis with a 5 second timeout """
        self.countdown('START')

        audiocodec = 'vorbis'
        recv, send = self.argfactory('audiovideo')
        send.audiocodec = audiocodec
        send.audiosource = 'audiotestsrc'
        recv.audiocodec = audiocodec
        for chan in xrange(1, 9): 
            send.numchannels = chan
            self.run(recv, send)

    def test_23_raw_h264_v4l2src(self):
        """ Test with 1-8 channels for raw with h264 with a 5 second timeout """
        self.countdown('START')

        videocodec = 'h264'
        recv, send = self.argfactory('audiovideo')
        send.videocodec = videocodec
        recv.videocodec = videocodec
        for chan in xrange(1, 9): 
            send.numchannels = chan
            self.run(recv, send)
    
    def test_24_raw_h263_v4l2src(self):
        """ Test with 8 channels for raw with h263 with a 5 second timeout """
        self.countdown('START')

        videocodec = 'h263'
        recv, send = self.argfactory('audiovideo')
        send.videocodec = videocodec
        recv.videocodec = videocodec
        send.videobitrate = 1000000
        self.run(recv, send)

    def test_25_h263_v4l2src(self):
        """ Test v4l2src with h263 """
        
        videocodec = 'h263'
        recv, send = self.argfactory('video')
        send.videocodec = videocodec
        recv.videocodec = videocodec
        self.run(recv, send)
    
    def test_26_h263_deinterlace(self):
        """ Test v4l2src with h263 """
        
        videocodec = 'h263'
        recv, send = self.argfactory('video')
        send.videocodec = videocodec
        recv.videocodec = videocodec
        recv.deinterlace = True
        self.run(recv, send)

    def test_27_mp3_v4l2src(self):
        """ Test mp3 with v4l """
        audiocodec = 'mp3'
        recv, send = self.argfactory('audiovideo')
        send.audiocodec = audiocodec
        recv.audiocodec = audiocodec
        self.run(recv, send)

    def test_28_wrong_ports(self):
        """ Feed equal videoport and audioport values to milhouse """
        recv, send = self.argfactory('audiovideo')
        recv.audioport = 10000
        recv.videoport = 10000
        send.audioport = 10000
        send.videoport = 10000
        self.run(recv, send)
    
    def test_29_wrong_channels_mp3(self):
        """ Feed wrong number of channels to milhouse for mp3 """
        recv, send = self.argfactory('audio')
        audiocodec = 'mp3'
        recv.audiocodec = audiocodec
        send.audiocodec = audiocodec
        send.numchannels = 3
        self.run(recv, send)

    def test_30_right_channels_mp3(self):
        """ Feed wrong number of channels to milhouse for mp3 """
        recv, send = self.argfactory('audio')
        audiocodec = 'mp3'
        recv.audiocodec = audiocodec
        send.audiocodec = audiocodec
        send.numchannels = 2
        send.audiosource = "audiotestsrc"
        self.run(recv, send)

    def test_31_raw_only(self):
        """ Just audio """
        recv, send = self.argfactory('audio')
        self.run(recv, send)
    
    def test_32_raw_audiofile(self):
        """ audiofile, uncompressed  """
        recv, send = self.argfactory('audio')
        send.audiosource = "filesrc"
        send.audiolocation = "/var/tmp/movie.avi"
        if os.path.exists(send.audiolocation):
            self.run(recv, send)
        else:
            print "No such file, skipping this test"
            
    def test_33_raw_audiofile_videofile_different(self):
        """ audiofile, sent in raw with different videofile """
        recv, send = self.argfactory('audiovideo')
        send.audiosource = "filesrc"
        send.videosource = "filesrc"
        send.audiolocation = "/var/tmp/things.mp3"
        send.videolocation = "/var/tmp/trailer_1080p.ogg"
        if os.path.exists(send.audiolocation) and os.path.exists(send.videolocation):
            self.run(recv, send)
        else:
            print "No such files, skipping this test"

    def test_34_raw_audiofile_videofile_same(self):
        """ one file is source for both video and audio """
        recv, send = self.argfactory('audiovideo')
        send.audiosource = "filesrc"
        send.videosource = "filesrc"
        send.audiolocation = "/var/tmp/trailer_1080p.ogg"
        send.videolocation = send.audiolocation
        if os.path.exists(send.audiolocation):
            self.run(recv, send)
        else:
            print "No such files, skipping this test"

    def test_35_v4l2_only(self):
        """ Just v4l2 """
        recv, send = self.argfactory('video')
        self.run(recv, send)
    

    def test_36_videofile_only(self):
        """ Just video file """
        recv, send = self.argfactory('video')
        send.videosource = "filesrc"
        if os.path.exists(send.videolocation):
            send.videolocation = "/var/tmp/trailer_1080p.ogg"
        self.run(recv, send)

    def test_37_audio_deinterlaced_video(self):
        """ Test with default args and 5 second timeout """
        self.countdown('START')

        recv, send = self.argfactory('audiovideo')
        recv.deinterlace = True
        recv.jitterbuffer = 60
        self.run(recv, send)
    
    def test_38_bufferTime(self):
        """ Test smaller audiobuffer to make sure the corrections is happening """

        recv, send = self.argfactory('audiovideo')
        recv.audio_buffer_usec = 11333
        self.run(recv, send)
    
    def test_39_vorbis_audiotestsrc_videotestsrc_h264(self):
        """ Test with 1-8 channels for vorbis with a 5 second timeout """
        self.countdown('START')

        recv, send = self.argfactory('audiovideo')
        send.audiocodec = 'vorbis'
        send.audiosource = 'audiotestsrc'
        send.videosource= 'videotestsrc'
        recv.audiocodec = send.audiocodec
        send.videocodec = 'h264'
        recv.videocodec = send.videocodec
        recv.jitterbuffer = 30
        for chan in xrange(1, 9): 
            send.numchannels = chan
            self.run(recv, send)
    
    def test_40_vorbis_audiotestsrc_videotestsrc_theora(self):
        """ Test with 1-8 channels for vorbis with a 5 second timeout """
        self.countdown('START')

        recv, send = self.argfactory('audiovideo')
        send.audiocodec = 'vorbis'
        send.audiosource = 'audiotestsrc'
        send.videosource= 'videotestsrc'
        recv.audiocodec = send.audiocodec
        send.videocodec = 'theora'
        recv.videocodec = send.videocodec
        for chan in xrange(1, 9): 
            send.numchannels = chan
            self.run(recv, send)
    
    def test_41_raw_jackaudiosrc_v4l2src_theora(self):
        """ Test with 1-8 channels for raw with a 5 second timeout """
        self.countdown('START')

        recv, send = self.argfactory('audiovideo')
        send.audiocodec = 'raw'
        send.audiosource = 'jackaudiosrc'
        send.videosource= 'v4l2src'
        recv.audiocodec = send.audiocodec
        send.videocodec = 'theora'
        recv.videocodec = send.videocodec
        for chan in xrange(8, 9): 
            send.numchannels = chan
            self.run(recv, send)

    def test_42_raw_jackaudiosrc_v4l2src_theora_deinterlace(self):
        """ Test with 1-8 channels for vorbis with a 5 second timeout """
        self.countdown('START')

        recv, send = self.argfactory('audiovideo')
        send.audiocodec = 'raw'
        send.audiosource = 'jackaudiosrc'
        send.videosource= 'v4l2src'
        recv.audiocodec = send.audiocodec
        send.videocodec = 'theora'
        recv.videocodec = send.videocodec
        recv.deinterlace = True
        for chan in xrange(8, 9): 
            send.numchannels = chan
            self.run(recv, send)

    def test_44_v4l2src_theora_deinterlace_xvimagesink(self):
        """ Test with 1-8 channels for vorbis with a 5 second timeout """
        self.countdown('START')

        recv, send = self.argfactory('video')
        send.videosource= 'v4l2src'
        send.videocodec = 'theora'
        recv.videocodec = send.videocodec
        recv.videosink = 'xvimagesink'
        recv.deinterlace = True
        self.run(recv, send)

    def test_45_deinterlace_ximagesink(self):
        """ Test with ximagesink"""
        recv, send = self.argfactory('video')

        recv.videosink = 'ximagesink'
        recv.deinterlace = True
        self.run(recv, send)
    
    def test_46_deinterlace_theora_ximagesink(self):
        """ Test with ximagesink"""
        recv, send = self.argfactory('video')

        recv.videosink = 'ximagesink'
        recv.videocodec = 'theora' 
        recv.deinterlace = True
        send.videocodec = recv.videocodec
        self.run(recv, send)
    
    def test_47_theora_sharedvideosink(self):
        """ Test with sharedvideosink """
        recv, send = self.argfactory('video')

        recv.videosink = 'sharedvideosink'
        recv.videocodec = 'theora' 
        recv.shared_video_id = 'shared_memory'
        send.videocodec = recv.videocodec
        self.run(recv, send)
    
    def test_48_theora_sharedvideosink_shared_id(self):
        """ Test with sharedvideosink with id shared_memory1 """
        recv, send = self.argfactory('video')

        recv.videosink = 'sharedvideosink'
        recv.videocodec = 'theora' 
        recv.shared_video_id = 'shared_memory1'
        send.videocodec = recv.videocodec
        self.run(recv, send)

    def test_49_mpeg4_sharedvideosink(self):
        """ Test with sharedvideosink """
        recv, send = self.argfactory('video')

        recv.videosink = 'sharedvideosink'
        recv.videocodec = 'mpeg4' 
        send.videocodec = recv.videocodec
        self.run(recv, send)

    def test_50_h264_sharedvideosink(self):
        """ Test with sharedvideosink """
        recv, send = self.argfactory('video')

        recv.videosink = 'sharedvideosink'
        recv.videocodec = 'h264' 
        send.videocodec = recv.videocodec
        self.run(recv, send)

    def test_51_h263_sharedvideosink(self):
        """ Test with sharedvideosink """
        recv, send = self.argfactory('video')

        recv.videosink = 'sharedvideosink'
        recv.videocodec = 'h263' 
        send.videocodec = recv.videocodec
        self.run(recv, send)

    def test_52_theora_deinterlace_sharedvideosink(self):
        """ Test with sharedvideosink """
        recv, send = self.argfactory('video')
        recv.videosink = 'sharedvideosink'
        recv.videocodec = 'theora' 
        send.videocodec = recv.videocodec
        recv.deinterlace = True
        self.run(recv, send)

if __name__ == '__main__':
    # here we run all the tests thanks to the wonders of reflective programming
    TESTS = prefixedMethods(MilhouseTests(), 'test_51')

    for test in TESTS:
        print 'TEST: '  + test.__doc__
        test()
        
