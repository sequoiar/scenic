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
Starts two miville and tests the protocols/pinger module.
"""
import unittest
from test import lib_miville_telnet as libmi

libmi.kill_all_running_miville()

fname = __file__.split('.')[0]
local = libmi.MivilleTester(use_tmp_home=True, logfile_prefix=fname + "local")
local.start_miville_process()
local.start_telnet_process()

remote = libmi.MivilleTester(use_tmp_home=True, port_offset=1, logfile_prefix=fname + "remote")
remote.start_miville_process()
remote.start_telnet_process()

class Test_Ping(unittest.TestCase):
    def setUp(self):
        global local
        global remote
        remote.unittest = self
        local.unittest = self
        self.local = local
        self.remote = remote

    def test_01_ping(self):
        # contacts we add might be already in their addressbook
        self.local.telnet_process.sendline("c -a Charlotte 127.0.0.1 2223")
        self.local.expectTest("added")
        self.remote.telnet_process.sendline("c -a Pierre 127.0.0.1 2222")
        self.remote.expectTest("added")
        self.local.telnet_process.sendline("c -s Charlotte")
        self.local.expectTest("selected")
        self.local.telnet_process.sendline("j -s")
        self.remote.expectTest("Do you accept")
        self.remote.telnet_process.sendline("Y")
        self.local.expectTest('accepted', 'Connection not successful.')
        self.local.telnet_process.sendline("ping")
        self.local.expectTest('pong', 'Did not receive pong answer.')

    def test_02_network_performance_test(self):
        self.local.telnet_process.sendline("n -s -k dualtest -t 1") # network test for 1 second
        self.local.expectTest("Starting", 'Did not start network test')
        self.local.expectTest('jitter', 'Did not receive network test results.', 1.3)

    def test_99_kill_miville(self):
        self.local.kill_miville_and_telnet()
        self.remote.kill_miville_and_telnet()

        local.telnet_logfile.close()
        local.miville_logfile.close()
        remote.telnet_logfile.close()
        remote.miville_logfile.close()
