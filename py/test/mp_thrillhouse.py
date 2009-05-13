#!/usr/bin/env python
""" Run two milhouse servers as well as telnet clients """

try:
    import multiprocessing 
except ImportError:
    print "import failed, please install multiprocessing: \nsudo easy-install multiprocessing"
    sys.exit(1)

import sys

try:
    from twisted.python.reflect import prefixedMethods
except ImportError:
    print "import failed, please install twisted: http://twistedmatrix.com/trac/"
    sys.exit(1)

from time import sleep
import os
import telnetlib
import socket


def milhouse_worker(args):
    """ a milhouse_worker will run milhouse as its own process """
    command = 'milhouse ' + args
    os.system(command)
    print '/*----------------------------------------------*/'
    print 'PROCESS ' + command + ' HAS EXITTED'
    print '/*----------------------------------------------*/'
    return


def createServers():
    """ Create workers which will wrap our milhouse server processes """
    workers = []
    commands = ['-r --serverport 9000', '-s --serverport 9001']
    for command in commands: 
        # create server
        p = multiprocessing.Process(target=milhouse_worker, args=(command,))
        workers.append(p)
        p.start()


def createClients():
    """ Create telnet clients which will interact with our servers using telnetlib """
    rx_server_port = 9000
    tx_server_port = 9001

    rx_server_up, tx_server_up = False, False

    while not rx_server_up:     # loop until server responds
        try: 
            rx_client = telnetlib.Telnet('localhost', rx_server_port)
            rx_server_up = True
        except socket.error:
                pass

    while not tx_server_up:     # loop until server responds
        try: 
            tx_client = telnetlib.Telnet('localhost', tx_server_port)
            tx_server_up = True
        except socket.error:
                pass

    return rx_client, tx_client


def runClients(rxTn, txTn, args):
    """ Here our telnet clients issue their commands to their respective milhouse servers """

    if 'rxAudioArg' in args:
        rxAudioArg = str(args['rxAudioArg'])
        print "SENDING " + rxAudioArg
        rxTn.write(rxAudioArg + '\n')
        rxTn.read_until('audio_init: ack="ok"')

    if 'txAudioArg' in args:
        txAudioArg = str(args['txAudioArg']) 
        print "SENDING " + txAudioArg
        txTn.write(txAudioArg + '\n')
        txTn.read_until('audio_init: ack="ok"')
    
    if 'rxVideoArg' in args:
        rxVideoArg = str(args['rxVideoArg'])
        print "SENDING " + rxVideoArg
        rxTn.write(rxVideoArg + '\n')
        rxTn.read_until('video_init: ack="ok"')

    if 'txVideoArg' in args:
        txVideoArg = str(args['txVideoArg'])
        print "SENDING " + txVideoArg
        txTn.write(txVideoArg + '\n')
        txTn.read_until('video_init: ack="ok"')
    

    rxTn.write('start:\n')
    rxTn.read_until('start: ack="ok"')
    txTn.write('start:\n')
    txTn.read_until('start: ack="ok"')


    # let the test run a bit
    TEST_LENGTH = 20
    sleep(TEST_LENGTH)

    rxTn.write('stop:\n')
    rxTn.read_until('stop: ack="ok"')
    txTn.write('stop:\n')
    txTn.read_until('stop: ack="ok"')

    rxTn.write('quit:\n')
    txTn.write('quit:\n')

def proceed(args):
    createServers()
    rxTn, txTn = createClients()
    runClients(rxTn, txTn, args)


class Arg(object): # new style!!
    """ Base class for our argument classes """
    def __init__(self):
        """ Init with address and timeout defaults """
        self.address = '127.0.0.1'   # always need this guy
    
    def argString(self):
        """ Returns a list of this class' data members and their values, 
        formatted as command line arguments """
        result = '_init:'
        for key, val in self.__dict__.iteritems():
            if val is True:
                val = 1  # boolean members don't need values in output string
                result = result + ' ' + key + '=' + str(val) 
            elif isinstance(val, str):
                result = result + ' ' + key + '=' + '"' + str(val) + '"'
            else:
                result = result + ' ' + key + '=' + str(val) 
        return result


class VideoArg(Arg):
    """ Base class for our video argument classes """
    def __init__(self):
        """ Defaults are mpeg4 videocodec and 11000 for videoport """
        Arg.__init__(self)
        self.codec = 'mpeg4'
        self.port = 11000
    
    def __str__(self):
        return 'video'  + self.argString()


class AudioArg(Arg):
    """ Base class for our Audio argument classes """
    def __init__(self):
        """ Defaults are uncompressed (raw) audio and 10000 for audioport """
        Arg.__init__(self)
        self.codec = 'raw'
        self.port = 10000

    def __str__(self):
        return 'audio'  + self.argString()

class VideoSendArg(VideoArg):
    """ Class for video only sending args """
    def __init__(self):
        """ Default for videosource is videotestsrc """
        VideoArg.__init__(self)
        self.source = 'videotestsrc'
        self.bitrate = 3000000

class VideoRecvArg(VideoArg):
    """ Class for video only receiving args """
    def __init__(self):
        """ Default for videosink is xvimagesink, the xvideo output plugin """
        VideoArg.__init__(self)
        self.sink = 'xvimagesink'


class AudioSendArg(AudioArg):
    """ Class for audio only sending args """
    def __init__(self):
        """ Default for audiosrc is 8 channel audiotestsrc, 
         the jack input plugin """
        AudioArg.__init__(self)
        self.source = 'audiotestsrc'
        self.channels = 8


class AudioRecvArg(AudioArg):
    """ Class for audio only receiving args """
    def __init__(self):
        """ Default for audiosink is jackaudiosink, the jack output plugin """
        AudioArg.__init__(self)
        self.sink = 'jackaudiosink'


def argfactory(argtype):
    """Returns default send and receive args"""
    if argtype is 'audio':
        return AudioRecvArg(), AudioSendArg()
    elif argtype is 'video':
        return VideoRecvArg(), VideoSendArg()
    else:
        raise Exception('unexpected argtype ' + argtype)


class VideoTests(object):
    def __init__(self):
        pass

    def test_videotestsrc_mpeg4_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_videotestsrc_theora_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "theora"
        txVideoArg.codec = "theora"
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_h264_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h264"
        txVideoArg.codec = "h264"
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_h263_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h263"
        txVideoArg.codec = "h263"
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_dv1394src_mpeg4_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.source = 'dv1394src'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_dv1394src_h264_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h264"
        txVideoArg.codec = "h264"
        txVideoArg.source = "dv1394src"
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_theora_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "theora"
        txVideoArg.codec = "theora"
        txVideoArg.source = "dv1394src"
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_dv1394src_h263_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h263"
        txVideoArg.codec = "h263"
        txVideoArg.source = "dv1394src"
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_v4l2src_mpeg4_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.source = "v4l2src"
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_v4l2src_h264_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h264"
        txVideoArg.codec = "h264"
        txVideoArg.source = "v4l2src"
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_v4l2src_theora_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "theora"
        txVideoArg.codec = "theora"
        txVideoArg.source = "v4l2src"
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_v4l2src_h263_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h263"
        txVideoArg.codec = "h263"
        txVideoArg.source = "v4l2src"
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_mpeg4_deinterlace_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.deinterlace = True
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_h264_deinterlace_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h264"
        txVideoArg.codec = "h264"
        txVideoArg.deinterlace = True
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_theora_deinterlace_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "theora"
        txVideoArg.codec = "theora"
        txVideoArg.deinterlace = True
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_h263_deinterlace_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h263"
        txVideoArg.codec = "h263"
        txVideoArg.deinterlace = True
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_v4l2src_mpeg4_deinterlace_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.source = "v4l2src"
        txVideoArg.deinterlace = True
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_v4l2src_h264_deinterlace_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h264"
        txVideoArg.codec = "h264"
        txVideoArg.source = "v4l2src"
        txVideoArg.deinterlace = True
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_v4l2src_theora_deinterlace_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "theora"
        txVideoArg.codec = "theora"
        txVideoArg.source = "v4l2src"
        txVideoArg.deinterlace = True
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_v4l2src_h263_deinterlace_xvimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h263"
        txVideoArg.codec = "h263"
        txVideoArg.source = "v4l2src"
        txVideoArg.deinterlace = True
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_mpeg4_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_h264_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h264"
        txVideoArg.codec = "h264"
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_theora_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "theora"
        txVideoArg.codec = "theora"
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_h263_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h263"
        txVideoArg.codec = "h263"
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_dv1394src_mpeg4_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.source = 'dv1394src'
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_dv1394src_h264_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h264"
        txVideoArg.codec = "h264"
        txVideoArg.source = "dv1394src"
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_theora_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "theora"
        txVideoArg.codec = "theora"
        txVideoArg.source = "dv1394src"
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_dv1394src_h263_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h263"
        txVideoArg.codec = "h263"
        txVideoArg.source = "dv1394src"
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_v4l2src_mpeg4_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.source = "v4l2src"
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_v4l2src_h264_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h264"
        txVideoArg.codec = "h264"
        txVideoArg.source = "v4l2src"
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_v4l2src_theora_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "theora"
        txVideoArg.codec = "theora"
        txVideoArg.source = "v4l2src"
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_v4l2src_h263_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h263"
        txVideoArg.codec = "h263"
        txVideoArg.source = "v4l2src"
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_mpeg4_deinterlace_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.deinterlace = True
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_h264_deinterlace_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h264"
        txVideoArg.codec = "h264"
        txVideoArg.deinterlace = True
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_videotestsrc_h264_deinterlace_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "theora"
        txVideoArg.codec = "theora"
        txVideoArg.deinterlace = True
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_h263_deinterlace_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h263"
        txVideoArg.codec = "h263"
        txVideoArg.deinterlace = True
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_v4l2src_mpeg4_deinterlace_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.source = "v4l2src"
        txVideoArg.deinterlace = True
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_v4l2src_h264_deinterlace_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h264"
        txVideoArg.codec = "h264"
        txVideoArg.source = "v4l2src"
        txVideoArg.deinterlace = True
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_v4l2src_theora_deinterlace_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "theora"
        txVideoArg.codec = "theora"
        txVideoArg.source = "v4l2src"
        txVideoArg.deinterlace = True
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_v4l2src_h263_deinterlace_glimagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h263"
        txVideoArg.codec = "h263"
        txVideoArg.source = "v4l2src"
        txVideoArg.deinterlace = True
        rxVideoArg.sink = 'glimagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_mpeg4_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_h264_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h264"
        txVideoArg.codec = "h264"
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_theora_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "theora"
        txVideoArg.codec = "theora"
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_h263_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h263"
        txVideoArg.codec = "h263"
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_dv1394src_mpeg4_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.source = 'dv1394src'
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_dv1394src_h264_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h264"
        txVideoArg.codec = "h264"
        txVideoArg.source = "dv1394src"
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_dv1394src_theora_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "theora"
        txVideoArg.codec = "theora"
        txVideoArg.source = "dv1394src"
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_dv1394src_h263_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h263"
        txVideoArg.codec = "h263"
        txVideoArg.source = "dv1394src"
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_v4l2src_mpeg4_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.source = "v4l2src"
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_v4l2src_h264_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h264"
        txVideoArg.codec = "h264"
        txVideoArg.source = "v4l2src"
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_v4l2src_theora_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "theora"
        txVideoArg.codec = "theora"
        txVideoArg.source = "v4l2src"
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_v4l2src_h263_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h263"
        txVideoArg.codec = "h263"
        txVideoArg.source = "v4l2src"
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_mpeg4_deinterlace_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.deinterlace = True
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_h264_deinterlace_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h264"
        txVideoArg.codec = "h264"
        txVideoArg.deinterlace = True
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_theora_deinterlace_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "theora"
        txVideoArg.codec = "thera"
        txVideoArg.deinterlace = True
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_videotestsrc_h263_deinterlace_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h263"
        txVideoArg.codec = "h263"
        txVideoArg.deinterlace = True
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_v4l2src_mpeg4_deinterlace_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.source = "v4l2src"
        txVideoArg.deinterlace = True
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_v4l2src_h264_deinterlace_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h264"
        txVideoArg.codec = "h264"
        txVideoArg.source = "v4l2src"
        txVideoArg.deinterlace = True
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_v4l2src_theora_deinterlace_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "theora"
        txVideoArg.codec = "theora"
        txVideoArg.source = "v4l2src"
        txVideoArg.deinterlace = True
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_v4l2src_h263_deinterlace_ximagesink(self):
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h263"
        txVideoArg.codec = "h263"
        txVideoArg.source = "v4l2src"
        txVideoArg.deinterlace = True
        rxVideoArg.sink = 'ximagesink'
        proceed(dict(rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))


class AudioTests(object):
    def __init__(self):
        pass

    def test_audiotestsrc_raw_jackaudiosink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg))

    def test_jackaudiosrc_raw_jackaudiosink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        txAudioArg.source = "jackaudiosrc"
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg))
    
    def test_dv1394src_raw_jackaudiosink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        txAudioArg.source = "dv1394src"
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg))

    def test_audiotestsrc_vorbis_jackaudiosink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg))

    def test_jackaudiosrc_vorbis_jackaudiosink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg))

    def test_dv1394src_vorbis_jackaudiosink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg))

    def test_audiotestsrc_mp3_jackaudiosink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg))

    def test_jackaudiosrc_mp3_jackaudiosink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg))

    def test_dv1394src_mp3_jackaudiosink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg))

class AudioNoJackTests(object):
    def __init__(self):
        pass

    def test_alsasrc_raw_alsasink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxAudioArg.sink = "alsasink"
        txAudioArg.source = "alsasrc"
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg))

    def test_alsasrc_vorbis_alsasink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxAudioArg.sink = "alsasink"
        txAudioArg.source = "alsasrc"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg))
    
    def test_alsasrc_mp3_alsasink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxAudioArg.sink = "alsasink"
        txAudioArg.source = "alsasrc"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg))

    def test_pulsesrc_raw_pulsesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxAudioArg.sink = "pulsesink"
        txAudioArg.source = "pulsesrc"
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg))

    def test_pulsesrc_vorbis_pulsesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxAudioArg.sink = "pulsesink"
        txAudioArg.source = "pulsesrc"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg))
    
    def test_pulsesrc_mp3_pulsesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxAudioArg.sink = "pulsesink"
        txAudioArg.source = "pulsesrc"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg))

class AudioVideoTests(object):
    def __init__(self):
        pass
    
    def test_audiotestsrc_raw_jackaudiosink_videotestsrc_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_raw_jackaudiosink_videotestsrc_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_raw_jackaudiosink_videotestsrc_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_vorbis_jackaudiosink_videotestsrc_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_vorbis_jackaudiosink_videotestsrc_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_vorbis_jackaudiosink_videotestsrc_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_mp3_jackaudiosink_videotestsrc_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_mp3_jackaudiosink_videotestsrc_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_mp3_jackaudiosink_videotestsrc_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))


    def test_audiotestsrc_raw_jackaudiosink_v4l2src_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.source = "v4l2src"
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_raw_jackaudiosink_v4l2src_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        txVideoArg.source = "v4l2src"
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_raw_jackaudiosink_v4l2src_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        txVideoArg.source = "v4l2src"
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_vorbis_jackaudiosink_v4l2src_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_vorbis_jackaudiosink_v4l2src_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_vorbis_jackaudiosink_v4l2src_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_mp3_jackaudiosink_v4l2src_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_mp3_jackaudiosink_v4l2src_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_mp3_jackaudiosink_v4l2src_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_audiotestsrc_raw_jackaudiosink_dv1394src_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.source = "dv1394src"
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_raw_jackaudiosink_dv1394src_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        txVideoArg.source = "dv1394src"
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_raw_jackaudiosink_dv1394src_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        txVideoArg.source = "dv1394src"
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_vorbis_jackaudiosink_dv1394src_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_vorbis_jackaudiosink_dv1394src_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_vorbis_jackaudiosink_dv1394src_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_mp3_jackaudiosink_dv1394src_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_mp3_jackaudiosink_dv1394src_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_mp3_jackaudiosink_dv1394src_mpeg4_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_audiotestsrc_raw_jackaudiosink_videotestsrc_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_raw_jackaudiosink_videotestsrc_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_raw_jackaudiosink_videotestsrc_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_vorbis_jackaudiosink_videotestsrc_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_vorbis_jackaudiosink_videotestsrc_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_vorbis_jackaudiosink_videotestsrc_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_mp3_jackaudiosink_videotestsrc_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_mp3_jackaudiosink_videotestsrc_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_mp3_jackaudiosink_videotestsrc_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))


    def test_audiotestsrc_raw_jackaudiosink_v4l2src_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_raw_jackaudiosink_v4l2src_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_raw_jackaudiosink_v4l2src_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_vorbis_jackaudiosink_v4l2src_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_vorbis_jackaudiosink_v4l2src_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_vorbis_jackaudiosink_v4l2src_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_mp3_jackaudiosink_v4l2src_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_mp3_jackaudiosink_v4l2src_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_mp3_jackaudiosink_v4l2src_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_audiotestsrc_raw_jackaudiosink_dv1394src_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_raw_jackaudiosink_dv1394src_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_raw_jackaudiosink_dv1394src_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_vorbis_jackaudiosink_dv1394src_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_vorbis_jackaudiosink_dv1394src_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_vorbis_jackaudiosink_dv1394src_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_mp3_jackaudiosink_dv1394src_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_mp3_jackaudiosink_dv1394src_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_mp3_jackaudiosink_dv1394src_h264_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "h264"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_raw_jackaudiosink_videotestsrc_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_raw_jackaudiosink_videotestsrc_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_raw_jackaudiosink_videotestsrc_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_vorbis_jackaudiosink_videotestsrc_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_vorbis_jackaudiosink_videotestsrc_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_vorbis_jackaudiosink_videotestsrc_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_mp3_jackaudiosink_videotestsrc_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_mp3_jackaudiosink_videotestsrc_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_mp3_jackaudiosink_videotestsrc_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))


    def test_audiotestsrc_raw_jackaudiosink_v4l2src_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_raw_jackaudiosink_v4l2src_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_raw_jackaudiosink_v4l2src_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_vorbis_jackaudiosink_v4l2src_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_vorbis_jackaudiosink_v4l2src_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_vorbis_jackaudiosink_v4l2src_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_mp3_jackaudiosink_v4l2src_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_mp3_jackaudiosink_v4l2src_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_mp3_jackaudiosink_v4l2src_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_audiotestsrc_raw_jackaudiosink_dv1394src_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_raw_jackaudiosink_dv1394src_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_raw_jackaudiosink_dv1394src_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_vorbis_jackaudiosink_dv1394src_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_vorbis_jackaudiosink_dv1394src_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_vorbis_jackaudiosink_dv1394src_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_mp3_jackaudiosink_dv1394src_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_mp3_jackaudiosink_dv1394src_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_mp3_jackaudiosink_dv1394src_theora_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "theora"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_raw_jackaudiosink_videotestsrc_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_raw_jackaudiosink_videotestsrc_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_raw_jackaudiosink_videotestsrc_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_vorbis_jackaudiosink_videotestsrc_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_vorbis_jackaudiosink_videotestsrc_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_vorbis_jackaudiosink_videotestsrc_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_mp3_jackaudiosink_videotestsrc_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_mp3_jackaudiosink_videotestsrc_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_mp3_jackaudiosink_videotestsrc_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))


    def test_audiotestsrc_raw_jackaudiosink_v4l2src_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_raw_jackaudiosink_v4l2src_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_raw_jackaudiosink_v4l2src_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_vorbis_jackaudiosink_v4l2src_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_vorbis_jackaudiosink_v4l2src_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_vorbis_jackaudiosink_v4l2src_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_mp3_jackaudiosink_v4l2src_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_mp3_jackaudiosink_v4l2src_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_mp3_jackaudiosink_v4l2src_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "v4l2src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

    def test_audiotestsrc_raw_jackaudiosink_dv1394src_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_raw_jackaudiosink_dv1394src_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_raw_jackaudiosink_dv1394src_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_vorbis_jackaudiosink_dv1394src_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_vorbis_jackaudiosink_dv1394src_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 9):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_vorbis_jackaudiosink_dv1394src_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "vorbis"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_audiotestsrc_mp3_jackaudiosink_dv1394src_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_jackaudiosrc_mp3_jackaudiosink_dv1394src_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "jackaudiosrc"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))
    
    def test_dv1394src_mp3_jackaudiosink_dv1394src_h263_xvimagesink(self):
        rxAudioArg, txAudioArg = argfactory('audio')
        rxVideoArg, txVideoArg = argfactory('video')
        txAudioArg.source = "dv1394src"
        rxAudioArg.codec = "mp3"
        txAudioArg.codec = rxAudioArg.codec
        txVideoArg.source = "dv1394src"
        rxVideoArg.codec = "h263"
        txVideoArg.codec = rxVideoArg.codec
        for channel in xrange(1, 3):
            txAudioArg.channels = channel
            proceed(dict(rxAudioArg=rxAudioArg, txAudioArg=txAudioArg, rxVideoArg=rxVideoArg, txVideoArg=txVideoArg))

if __name__ == '__main__':
    if len(sys.argv) == 2:
        prefix = sys.argv[1]
    else:
        prefix = 'test_v4l2src_theora_glimage'

    print "Running tests which start with '" + prefix + "'\n\n"
    # here we run all the tests thanks to the wonders of reflective programming
    tests = prefixedMethods(VideoTests(), prefix)

    if tests == []:
        print "No matching tests found."

    for test in tests:
        print '/*----------------------------------------------*/'
        print 'RUNNING TEST: '  + test.__name__
        print '/*----------------------------------------------*/'
        test()
