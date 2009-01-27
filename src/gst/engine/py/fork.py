#!/usr/bin/env python

import sys, os

import time

def run():
    pid = os.fork()

    if pid == 0:
        print "first child"
        os.system('./sropulpof.py -r -o 5000')
        time.sleep(1)
        sys.exit(0)
    else:
        # parent
        pid = os.fork()
        if pid == 0:
            print "second child"
            os.system('./sropulpof.py -s -o 5000')
            time.sleep(1)
            sys.exit(0)
        else:
            os.waitpid(pid, 0)
            print "finally, i'm the parent"

run()
run()
run()
run()
run()
