#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technoligiques (SAT)
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

"""
Integration tests for milhouse.

Usage: trial test/rx.py
""" 

# system imports
import twisted.trial.unittest

# helper method
from timeout import *



class Test_milhouse_rx(twisted.trial.unittest.TestCase):
    """
    Integration tests for milhouse receiver.
    """

    def test_01_defaults(self):
        TIMEOUT_S = 1
        TIMEOUT_MS = 1000 * TIMEOUT_S
        args = '-r -o ' + str(TIMEOUT_MS)
        cmd = '../sropulpof.py ' + args
        if not timeoutCommand(cmd, TIMEOUT_S + 2):
            print 'took too long'
