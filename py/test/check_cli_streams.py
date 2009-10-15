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
Tests if miville can stream between two alice mivilles.
"""

import unittest
from test import lib_miville_telnet as libmi

libmi.kill_all_running_miville()
VERBOSE = True

alice = libmi.MivilleTester(port_offset=2, verbose=VERBOSE, use_tmp_home=True)
alice.start_miville_process()
alice.start_telnet_process()
bob = libmi.MivilleTester(port_offset=3, verbose=VERBOSE, use_tmp_home=True)
bob.start_miville_process()
bob.start_telnet_process()

class Test_Miville_Streams(unittest.TestCase):
    def setUp(self):
        global alice
        alice.unittest = self
        self.alice = alice
        global bob
        bob.unittest = self
        self.bob = bob

    def _flush_both(self):
        self.alice.flush()
        self.bob.flush()
        
    def test_01_cli_works(self):
        self.alice.tst('c -l', 'pof:')
        self._flush_both()
        self.bob.tst('c -l', 'pof:')
        self._flush_both()

    def test_02_add_contacts(self):
        self.alice.tst("c -a Bob 127.0.0.1 2225", "added")
        self._flush_both()
        self.bob.tst("c -a Alice 127.0.0.1 2224", "added")
        self._flush_both()
        self.alice.tst("c -s Bob", "selected")
        self._flush_both()
        self.bob.tst("c -s Alice", "selected") #useless
        self._flush_both()

    def test_03_join(self):
        self.alice.telnet_process.sendline("j -s")
        self.bob.expectTest("Do you accept")
        self.bob.telnet_process.sendline("Y")
        self.alice.expectTest('accepted', 'Connection not successful.')
        self._flush_both()
    
#    def test_04_start_streams(self):
#        self.alice.tst("z -s Bob", "Successfully started to stream with", timeout=5) # very long timeout
#        self._flush_both()
#
#    def test_05_stop_streams(self):
#        self.bob.tst("z -i Alice", "Successfully stopped to stream with", timeout=5) # very long timeout
#        self._flush_both()

    def test_04_start_streams(self):
        # FIXME: doesnt actually test anything
        self.alice.telnet_process.sendline("z -s Bob")
        for t in range(3):
            duration = float(t + 1)
            self.alice.sleep(duration)
            self.bob.sleep(duration)

    def test_05_stop_streams(self):
        # FIXME: doesnt actually test anything
        self.alice.telnet_process.sendline("z -i Bob")
        duration = 1.0
        self.alice.sleep(duration)
        self.bob.sleep(duration)

    def test_06_disconnect(self):
        # mandatory prior to delete contacts
        self.alice.telnet_process.sendline("j -i")
        self.bob.expectTest("Connection has been closed by")
        self._flush_both()
    
    def test_98_quit(self):
        self.alice.telnet_process.sendline("c -e Bob")
        self.bob.telnet_process.sendline("c -e Alice")
        self._flush_both()
        self.alice.telnet_process.sendline("quit")
        self.bob.telnet_process.sendline("quit")
        self._flush_both()

    def test_99_kill_miville(self):
        self._flush_both()
        self.alice.kill_miville_and_telnet()
        self.bob.kill_miville_and_telnet()

