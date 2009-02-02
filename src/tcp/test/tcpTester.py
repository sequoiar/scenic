#!/usr/bin/env python

import sys, os

import signal
import time

from twisted.python.reflect import prefixedMethods

def run_thread_server(port):
    print 'running thread server on ' + str(port)



class TcpTests():
    def __init__(self):
        signal.signal(signal.SIGINT, self.receiveInterrupt)


    @staticmethod
    def receiveInterrupt(signum, stack):
        print 'Received interrupt, exiting()'
        throw('exit')


    def test_01_thread_server(self):
        print 'thread server start'
        run_thread_server(1024)
        print 'thread server stoped'

# here we run all the tests thanks to the wonders of reflective programming
for test in prefixedMethods(TcpTests(), 'test_'):
    test()

