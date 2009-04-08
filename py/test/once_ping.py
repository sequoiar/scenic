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
from test import lib_clientserver as clientserver
import os

fname = __file__.split('.')[0]
logdir = "test/log/1" # put your build number here (env var)
devnull = open(os.devnull, 'w')
local = clientserver.TelnetMivilleTester(name='local', client_logfile=devnull, server_logfile=devnull, use_tmp_home=True)
# logdir=logdir, logfilename=fname + "local", use_tmp_home=True)
local.start()

remote = clientserver.TelnetMivilleTester(name='remote', client_logfile=devnull, server_logfile=devnull, use_tmp_home=True, port_offset=1)
# port_offset=1, logdir=logdir, logfilename=fname + "remote", use_tmp_home=True)
remote.start()

class Test_Ping(unittest.TestCase):
    def setUp(self):
        global local
        global remote
        self.local = local
        self.remote = remote
        self.local.setup(self)
        self.remote.setup(self)

    def test_01_ping(self):
        # contacts we add might be already in their addressbook
        self.local.client.child.sendline("c -a Charlotte 127.0.0.1 2223")
        self.local.client.expect_test("added")
        self.remote.client.child.sendline("c -a Pierre 127.0.0.1 2222")
        self.remote.client.expect_test("added")
        self.local.client.child.sendline("c -s Charlotte")
        self.local.client.expect_test("selected")
        self.local.client.child.sendline("j -s")
        self.remote.client.expect_test("Do you accept")
        self.remote.client.child.sendline("Y")
        self.local.client.expect_test('accepted', 'Connection not successful.')
        self.local.client.child.sendline("ping")
        self.local.client.expect_test('pong', 'Did not receive pong answer.')

    def test_02_network_performance_test(self):
        self.local.client.child.sendline("n -s -k dualtest -t 1") # network test for 1 second
        self.local.client.expect_test("Starting", 'Did not start network test')
        self.local.client.expect_test('jitter', 'Did not receive network test results.', 1.3)

    def test_99_close(self):
        try:
            self.local.kill_client_and_server()
            self.remote.kill_client_and_server()
        except Exception, e:
            print e.message
