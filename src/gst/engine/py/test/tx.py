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

Usage: trial test/tx.py
""" 

# system imports
import twisted.trial.unittest

# to call sropulpof as if from shell
import subprocess

class Test_milhouse_tx(twisted.trial.unittest.TestCase):
    """
    Integration tests for milhouse sender.
    """
    def setup(self):
        self.timeout = 0.1

    def test_01_defaults(self):
        TIMEOUT_MS = 1000
        args = '-s -o ' + str(TIMEOUT_MS)
        proc = subprocess.Popen('../sropulpof.py ' + args, shell=True) 
        proc.wait()
        #while proc.poll():
        #    if time > timeout:
        #        proc.kill()
