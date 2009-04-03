#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Miville
# Copyright (C) 2008 Société des arts technoligiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Miville is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Miville.  If not, see <http://www.gnu.org/licenses/>.

"""
Simply tests if miville starts. 

This is also a demo of the new test lib
"""

import unittest
from test import lib_miville_telnet as libmi

libmi.kill_all_running_miville()

tester = libmi.MivilleTester()
tester.start_miville_process()
tester.start_telnet_process()

class Test_Start_One_Miville(unittest.TestCase):
    def setUp(self):
        global tester
        tester.unittest = self
        self.tester = tester
    def test_01_simple(self):
        self.tester.tst('c -l', 'pof:')
    def test_99_kill_miville(self):
        self.tester.kill_miville_and_telnet()
