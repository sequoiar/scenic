#!/usr/bin/env python

import sys, os

import time

def run():
    pid = os.fork()

    if pid == 0:
        print "receiver process"
        os.system('./sropulpof.py -r -o 5000')
        sys.exit(0)
    else:
        # parent
        pid = os.fork()
        if pid == 0:
            print "sender process child"
            os.system('./sropulpof.py -s -o 5000')
            sys.exit(0)
        else:
            os.waitpid(pid, 0)
            print "finally, i'm the parent"

for i in xrange(0, 4):
    run()
