#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Miville
# Copyright (C) 2008 Société des arts technologiques (SAT)
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
Simply tests if miville can stream. 
"""

import unittest
from test import lib_miville_telnet as libmi
import os

# nullfile = open(os.devnull, 'w')
print("Kill all miville")
libmi.kill_all_running_miville()
# TESTER 1
print("Launch 2 miville instances.")
tester_1 = libmi.MivilleTester(port_offset=0, use_tmp_home=True, verbose=True)#, telnet_logfile=nullfile, miville_logfile=nullfile)
tester_1.start_miville_process()
tester_1.start_telnet_process()
# TESTER 2
tester_2 = libmi.MivilleTester(port_offset=1, use_tmp_home=True, verbose=True)#, telnet_logfile=nullfile, miville_logfile=nullfile)
tester_2.start_miville_process()
tester_2.start_telnet_process()

class Test_Start_Two_Miville(unittest.TestCase):
    """
    Tests if miville can stream.
    """
    def setUp(self):
        global tester_1
        global tester_2
        tester_1.unittest = self
        tester_2.unittest = self
        self.tester_1 = tester_1
        self.tester_2 = tester_2
    def test_01_prompt(self):
        print("Starrting tests")
        self.tester_1.tst('c -l', 'pof:')
        self.tester_2.tst('c -l', 'pof:')
    def test_02_add_contact(self):
        self.tester_1.tst('c -a two 127.0.0.1 2223', 'added')
        self.tester_2.tst('c -a one 127.0.0.1 2222', 'added')
        self.tester_1.tst('c -s two', 'selected')
        self.tester_1.tst(" c -m setting=10003", "modified")
    def test_03_join(self):
        self.tester_1.tst("c -s two", "selected")
        self.tester_1.tst("j -s", "Trying to connect")
        self.tester_2.tst("Y", "connected")
    def test_04_start_stream(self):
        self.tester_1.tst("streams --start=two", "treaming") # TODO
        for i in range(10): # seconds
            self.tester_1.sleep(0.5)
            self.tester_2.sleep(0.5)
            
    def test_99_kill_miville(self):
        self.tester_1.kill_miville_and_telnet()
        self.tester_2.kill_miville_and_telnet()
        # global nullfile
        #nullfile.close()
