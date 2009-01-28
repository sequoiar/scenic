#!/usr/bin/env python

import sys, os

from twisted.python.reflect import prefixedMethods


class MilhouseTests():
    def __init__(self):
        pass

    @staticmethod
    def timeouts():
        """ Returns tuple of timeout arguments """
        timeout = '-o 5000 '
        return timeout, timeout

    @staticmethod
    def runTest(rxArgs, txArgs):
        """ This method is used by our helpers to create a receiver
            process and a sender process, and wait on them. """
        pidRx = os.fork()

        if pidRx == 0:
            print "receiver process"
            os.system('../sropulpof.py -r ' +  rxArgs)
            sys.exit(0)
        else:
            # parent
            pidTx = os.fork()
            if pidTx == 0:
                print "sender process child"
                os.system('../sropulpof.py -s ' +  txArgs)
                sys.exit(0)
            else:
                # parent
                os.waitpid(pidTx, 0)
                print "finally, i'm the parent"

    def test_01_defaults(self):
        """ Test with default args and 5 second timeout """
        rxArgs, txArgs = self.timeouts()
        self.runTest(rxArgs, txArgs)

    def test_02_channels(self):
        """ Test with 1-8 channels and 5 second timeout """
        rxArgs, txArgs = self.timeouts()
        for c in xrange(1, 9): 
            self.runTest(rxArgs, txArgs + '-c ' + str(c))


# here we run all the tests thanks to the wonders of reflective programming
for test in prefixedMethods(MilhouseTests(), 'test_'):
    test()

