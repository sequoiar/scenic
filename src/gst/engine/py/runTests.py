#!/usr/bin/env python

import os
from twisted.internet import reactor, error
from twisted.internet.protocol import ProcessProtocol

proc = None

class EchoingProcessProtocol(ProcessProtocol):
    def outReceived(self, data):
        """Will get called when the subprocess has data on stdout."""
        print data

    def errReceived(self, data):
        """Will get called when the subprocess has data on stderr"""
        print 'STDERR:', data

    def connectionMade(self):
        """Will get called when the subprocess starts"""
        print 'Started running subprocess'

    def processEnded(self, reason):
        """Will get called when the subprocess ends"""
        print 'Completed running subprocess'
        

def run():
    """Spawn the process and copy across the environment"""
    proc = reactor.spawnProcess(EchoingProcessProtocol(), 'trial', ['trial', 'test/rx.py'], env=os.environ)
    reactor.run()
    try:
        proc.signalProcess('KILL')
    except error.ProcessExitedAlready:
        print "don't kill me, i'm Already dead"

if __name__ == '__main__':
    run()

