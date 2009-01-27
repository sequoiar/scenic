#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Sropulpof is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Sropulpof.  If not, see <http:#www.gnu.org/licenses/>.
#
#

"""
Integration test for Milhouse.

Usage: trial test/tests.py
"""


import os, sys
import twisted.trial.unittest 
import sropulpof
import time


def run(rxArgs, txArgs):
    try:
        pidRx = os.fork()
        # child
        if pidRx == 0:
            sropulpof.run(['-r'] + rxArgs)
            print "DONE RECEIVING"
            sys.exit(0)
        else:
            try:
                pidTx = os.fork()
                # child
                if pidTx == 0:
                    sropulpof.run(['-s'] + txArgs)
                    print "DONE SENDING"
                    sys.exit(0)
                else:
                # parent
                wait()
                wait()
            except OSError, e:
                print >>sys.stderr, "sender process failed: %d (%s)" % (e.errno, e.strerror)
                sys.exit(1)
    except OSError, e:
        print >>sys.stderr, "receiver process failed: %d (%s)" % (e.errno, e.strerror)
        sys.exit(1)



class MilhouseTest(twisted.trial.unittest.TestCase):

    def test_01_defaults(self):
        test_time = 5000
        rxArgs = ['-o', str(test_time) ]
        txArgs = ['-o', str(test_time) ]
        run(rxArgs, txArgs)
        time.sleep(test_time/1000 + 1)
        return True

    def test_02_defaults(self):
        test_time = 5000
        rxArgs = ['-o', str(test_time) ]
        txArgs = ['-o', str(test_time) ]
        run(rxArgs, txArgs)
        time.sleep(test_time/1000 + 1)
        return True

    
