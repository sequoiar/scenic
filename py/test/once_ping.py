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
from test import lib_clientserver as testing
import os
import sys
import time

class Test_Ping(unittest.TestCase):
    def setUp(self):
        #print "------------------ test -----------------------------"
        #print "current directory:", os.getenv('PWD')
        testing.VERBOSE = False
        # CHOOSE YOUR LOG FILE HERE
        #log_dir = "/tmp/miville_test"
        #log_file_name = "%s.log" % (clientserver.make_test_name(__file__))
        #logfile = clientserver.open_logfile(log_dir, log_file_name)
        logfile = open(os.devnull, 'w')
        #logfile = sys.stdout
        #print "\n"
        #for level in range(100):
        #    print "LEVEL:", level, sys._getframe(level).f_code.co_name
        testing.kill_all_zombie_processes()
        self.local = testing.TelnetMivilleTester(port_offset=0, use_tmp_home=True, verbose=testing.VERBOSE, logfile=logfile)
        self.local.setup(self)
        self.remote = testing.TelnetMivilleTester(port_offset=1, use_tmp_home=True, verbose=testing.VERBOSE, logfile=logfile)
        self.remote.setup(self)
        self.local.sendline("c -l")
        self.local.send_expect("c -a Charlotte 127.0.0.1 2223", "added")
        self.remote.sendline("c -l")
        self.remote.send_expect("c -a Pierre 127.0.0.1 2222", "added")
        self.local.send_expect("c -s Charlotte", "selected")
        self.local.sendline("j -s")
        self.remote.expect_test("Do you accept")
        self.remote.sendline("Y")
        self.local.expect_test('accepted', 'Connection not successful.')

    def test_01_ping(self):
        # contacts we add might be already in their addressbook
        self.local.send_expect("ping", "pong")

    def test_02_network_performance_test(self):
        self.local.sendline("n -s -k dualtest -t 1") # network test for 1 second
        self.local.expect_test("Starting", 'Could not start network test')
        self.local.expect_test('jitter', 'Did not receive network test results.', 1.3)

    def tearDown(self):
        self.local.kill_children()
        self.remote.kill_children()

