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


def run(rxArgs, txArgs):
    try:
        pidRx = os.fork()
        # child
        if pidRx == 0:
            result = sropulpof.run(['-r'] + rxArgs)
            print "DONE RECEIVING"
            return result
    except OSError, e:
        print >>sys.stderr, "receiver process failed: %d (%s)" % (e.errno, e.strerror)
        sys.exit(1)

    try:
        pidTx = os.fork()
        # child
        if pidTx == 0:
            result = sropulpof.run(['-s'] + txArgs)
            print "DONE SENDING"
            return result
    except OSError, e:
        print >>sys.stderr, "sender process failed: %d (%s)" % (e.errno, e.strerror)
        sys.exit(1)


class MilhouseTest(twisted.trial.unittest.TestCase):

    def test_01_defaults(self):
        rxArgs = ['-o', '5000']
        txArgs = ['-o', '5000']
        self.failUnless(run(rxArgs, txArgs))

