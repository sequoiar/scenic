#!/usr/bin/env python
""" Run two milhouse servers as well as telnet clients """

import multiprocessing 

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

    while not serverUp:
        try: 
            rxClient = telnetlib.Telnet('localhost', receiverServerport)
            serverUp = True
        except socket.error:
                pass

    serverUp = False
    while not serverUp:
        try: 
            txClient = telnetlib.Telnet('localhost', senderServerport)
            serverUp = True
        except socket.error:
                pass

    return rxClient, txClient


def runClients(rxTn, txTn):
    TEST_LENGTH = 10
    rxTn.write('video_init: codec="h264" port=10000 address="127.0.0.1"\n')
    rxTn.read_until('video_init: ack="ok"')

    txTn.write('video_init: codec="h264" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc"\n')
    txTn.read_until('video_init: ack="ok"')

    rxTn.write('start:\n')
    rxTn.read_until('start: ack="ok"')
    txTn.write('start:\n')
    txTn.read_until('start: ack="ok"')

    # wait a while
    time.sleep(TEST_LENGTH)

    rxTn.write('stop:\n')
    rxTn.read_until('stop: ack="ok"')
    txTn.write('stop:\n')
    txTn.read_until('stop: ack="ok"')

    rxTn.write('quit:\n')
    txTn.write('quit:\n')


if __name__ == '__main__':
    createServers()
    rx, tx = createClients()
    runClients(rx, tx)

