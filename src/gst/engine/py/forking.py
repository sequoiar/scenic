#!/usr/bin/env python

import os, sys

import sropulpof

timeout = '10000'

def rx():
    sropulpof.run(['-r', '-o', timeout])

def tx():
    sropulpof.run(['-s', '-o', timeout])


if __name__ == "__main__":
    try:
        pidRx = os.fork()
        # child
        if pidRx == 0:
            rx()
            print "DONE RECEIVING"
            sys.exit(0)
    except OSError, e:
        print >>sys.stderr, "fork #1 failed: %d (%s)" % (e.errno, e.strerror)
        sys.exit(1)

    # do second fork
    try:
        pidTx = os.fork()
        # child
        if pidTx == 0:
            tx()
            print "DONE SENDING"
            sys.exit(0)
    except OSError, e:
        print >>sys.stderr, "fork #2 failed: %d (%s)" % (e.errno, e.strerror)
        sys.exit(1)

