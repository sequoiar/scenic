#!/usr/bin/env python

import sys, os

import signal
import time

from twisted.python.reflect import prefixedMethods

pidRx = 0
pidTx = 0

class MilhouseTests():
    def __init__(self):
        signal.signal(signal.SIGINT, self.receiveInterrupt)

    @staticmethod
    def timeouts():
        """ Returns tuple of timeout arguments """
        timeout = '-o 0'
        return timeout, timeout

    @staticmethod
    def countdown(warning):
        countdown = 1
        while countdown > 0:
            print "PLEASE " + warning + " JACK SERVER NOW, YOU HAVE " + str(countdown) + " SECONDS" 
            time.sleep(1)
            countdown -= 1

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
    def runTest(rxArgs, txArgs):
        """ This method is used by our helpers to create a receiver
            process and a sender process, and wait on them. """
        pidRx = os.fork()

        if pidRx == 0:
            os.system('../sropulpof.py -r ' +  rxArgs)
            sys.exit(0)
        else:
            # parent
            pidTx = os.fork()
            if pidTx == 0:
                os.system('../sropulpof.py -s ' +  txArgs)
                sys.exit(0)
            else:
                # parent
                os.waitpid(pidTx, 0)
                print 'END OF TEST'

    def test_01_defaults(self):
        """ Test with default args and 5 second timeout """
        self.countdown("START")

        rxArgs, txArgs = self.timeouts()
        self.runTest(rxArgs, txArgs)

    def test_02_jack(self):
        """ Test with 1-8 channels and 5 second timeout for jacksrc and dv1394src """
        self.countdown("START")

        rxArgs, txArgs = self.timeouts()
        for c in xrange(1, 9): 
            self.runTest(rxArgs, txArgs + '-c ' + str(c)) 
    
    def test_03_dv(self):
        """ Test dv inputs """
        self.countdown("START")

        rxArgs, txArgs = self.timeouts()
        self.runTest(rxArgs, txArgs + ' --videosource dv1394src --audiosource dv1394src')
   
    def test_04_alsa(self):
        """ Test with 1-8 channels for alsa with a 5 second timeout """
        self.countdown("STOP")

        rxArgs, txArgs = self.timeouts()
        source = 'alsasrc'
        sink = 'alsasink'
        for c in xrange(1, 7): 
            self.runTest(rxArgs + ' --audiosink ' + sink, txArgs + '-c ' + str(c) + ' --audiosource ' + source)

    def test_05_pulse(self):
        """ Test with 1-6 channels for pulse with a 5 second timeout """
        self.countdown("STOP")

        rxArgs, txArgs = self.timeouts()
        source = 'pulsesrc'
        sink = 'pulsesink'
        for c in xrange(1, 7): 
            self.runTest(rxArgs + ' --audiosink ' + sink, txArgs + '-c ' + str(c) + ' --audiosource ' + source)

    def test_06_vorbis(self):
        """ Test with 1-8 channels for vorbis with jack with a 5 second timeout """
        self.countdown("START")

        audiocodec = 'vorbis'

        rxArgs, txArgs = self.timeouts()
        for c in xrange(1, 9): 
            self.runTest(rxArgs + ' --audiocodec ' + audiocodec, txArgs + '-c ' + str(c) + ' --audiocodec ' + audiocodec)

    def test_07_dv_vorbis(self):
        """ Test with 1-8 channels for vorbis with dv and jack with a 5 second timeout """
        self.countdown("START")

        audiocodec = 'vorbis'

        rxArgs, txArgs = self.timeouts()
        txArgs += ' --audiosource dv1394src --videosource dv1394src --audiocodec vorbis'
        rxArgs += ' --audiocodec vorbis '
        self.runTest(rxArgs, txArgs)
    
    def test_08_videotestsrc_h264(self):
        """ Test h264 with videotestsrc """
        self.countdown("START")

        rxArgs, txArgs = self.timeouts()
        self.runTest(rxArgs + ' --videocodec h264', txArgs + ' --videosource videotestsrc --videocodec h264')

    def test_09_glImagesink_testsrc(self):
        """ Test glimagesink """
        self.countdown("START")

        rxArgs, txArgs = self.timeouts()
        self.runTest(rxArgs + ' --videosink glimagesink', txArgs + ' --videosource videotestsrc')

    def test_10_audioOnly(self):
        """ Test with just audio """
        self.countdown("START")

        rxArgs, txArgs = self.timeouts()
        self.runTest(rxArgs + ' --disable-video', txArgs + ' --disable-video')
    
    def test_11_videoOnly(self):
        """ Test with just video """
        self.countdown("START")

        rxArgs, txArgs = self.timeouts()
        self.runTest(rxArgs + ' --disable-audio', txArgs + ' --disable-audio')

    def test_12_ximagesink_v4l2src(self):
        """ Test with ximagesink"""
        self.countdown("START")

        rxArgs, txArgs = self.timeouts()
        self.runTest(rxArgs + ' --videosink ximagesink', txArgs)
    
    def test_13_glImagesink_v4l2src(self):
        """ Test v4l with glimagesink """
        self.countdown("START")

        rxArgs, txArgs = self.timeouts()
        self.runTest(rxArgs + ' --videosink glimagesink', txArgs + ' --videosource v4l2src')

    def test_14_ximagesink_dv1394src(self):
        """ Test dv with ximagesink"""
        self.countdown("START")

        rxArgs, txArgs = self.timeouts()
        self.runTest(rxArgs + ' --videosink ximagesink', txArgs + ' --videosource dv1394src')

    def test_15_glimagesink_dv1394src(self):
        """ Test dv with glimagesink"""
        self.countdown("START")

        rxArgs, txArgs = self.timeouts()
        self.runTest(rxArgs + ' --videosink glimagesink', txArgs + ' --videosource dv1394src')


# here we run all the tests thanks to the wonders of reflective programming
tests = prefixedMethods(MilhouseTests(), 'test_10')

for test in tests:
    print "TEST: "  + test.__doc__
    test()

