#!/usr/bin/env python
""" Run two milhouse servers as well as telnet clients """

try:
    import multiprocessing 
except ImportError:
    print "import failed, please install multiprocessing: \nsudo easy-install multiprocessing"
    sys.exit(1)

try:
    from twisted.python.reflect import prefixedMethods
except ImportError:
    print "import failed, please install twisted: http://twistedmatrix.com/trac/"
    sys.exit(1)

from time import sleep
import os


import telnetlib
import socket
import time


def serverWorker(args):
    """ milhouse servers """
    command = 'milhouse ' + args
    os.system(command)
    print 'PROCESS ' + command + ' HAS EXITTED'
    return


def createServers():
    """ Create workers which will wrap our milhouse server processes """
    workers = []
    # create receiver server
    p = multiprocessing.Process(target=serverWorker, args=('-r --serverport 9000',))
    workers.append(p)
    p.start()

    # create sender server
    p = multiprocessing.Process(target=serverWorker, args=('-s --serverport 9001',))
    workers.append(p)
    p.start()


def createClients():
    receiverServerport = 9000
    senderServerport = 9001

    serverUp = False

    while not serverUp:     # loop until server responds
        try: 
            rxClient = telnetlib.Telnet('localhost', receiverServerport)
            serverUp = True
        except socket.error:
                pass

    serverUp = False
    while not serverUp:     # loop until server responds
        try: 
            txClient = telnetlib.Telnet('localhost', senderServerport)
            serverUp = True
        except socket.error:
                pass

    return rxClient, txClient


def runClients(rxTn, txTn, rxArg, txArg):
    TEST_LENGTH = 10
    rxTn.write(rxArg + '\n')
    rxTn.read_until('video_init: ack="ok"')

    txTn.write(txArg + '\n')
    txTn.read_until('video_init: ack="ok"')

    rxTn.write('start:\n')
    rxTn.read_until('start: ack="ok"')
    txTn.write('start:\n')
    txTn.read_until('start: ack="ok"')

    # wait a while
    sleep(TEST_LENGTH)

    rxTn.write('stop:\n')
    rxTn.read_until('stop: ack="ok"')
    txTn.write('stop:\n')
    txTn.read_until('stop: ack="ok"')

    rxTn.write('quit:\n')
    txTn.write('quit:\n')

def proceed(rxArg, txArg):
    createServers()
    rx, tx = createClients()
    runClients(rx, tx, rxArg, txArg)

class TelnetTests(object):
    def __init__(self):
        pass

    def test01_videotestsrc_h264(self):
        rxArg = 'video_init: codec="h264" port=10000 address="127.0.0.1"'
        txArg = 'video_init: codec="h264" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc"'
        proceed(rxArg, txArg)

    def test02_videotestsrc_h263(self):
        rxArg = 'video_init: codec="h263" port=10000 address="127.0.0.1"'
        txArg = 'video_init: codec="h263" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc"'
        proceed(rxArg, txArg)

    def test03_videotestsrc_mpeg4(self):
        rxArg = 'video_init: codec="mpeg4" port=10000 address="127.0.0.1"'
        txArg = 'video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc"'
        proceed(rxArg, txArg)

if __name__ == '__main__':
    # here we run all the tests thanks to the wonders of reflective programming
    tests = prefixedMethods(TelnetTests(), 'test')

    for test in tests:
        print '/*----------------------------------------------*/'
        print 'RUNNING TEST: '  + test.__name__
        print '/*----------------------------------------------*/'
        test()
