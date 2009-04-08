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
import time

fname = __file__.split('.')[0]
logdir = "test/log/1" # put your build number here (env var)
devnull = open(os.devnull, 'w')
sender = clientserver.TelnetMilhouseTester(name='sender', client_logfile=devnull, server_logfile=devnull, mode='s', serverport=9000)
# logdir=logdir, logfilename=fname + "sender", use_tmp_home=True)
sender.start()

receiver = clientserver.TelnetMilhouseTester(name='receiver', client_logfile=devnull, server_logfile=devnull, mode='r', serverport=9001)
# port_offset=1, logdir=logdir, logfilename=fname + "receiver", use_tmp_home=True)
receiver.start()

class Test_Milhouse(unittest.TestCase):
    def setUp(self):
        global sender
        global receiver
        self.sender = sender
        self.receiver = receiver
        self.sender.setup(self)
        self.receiver.setup(self)

    def xtest_01_basic_milhouse_control(self):
        self.receiver.client.child.sendline('audio_init: codec="raw" port=10000 address="127.0.0.1" ')
        self.receiver.client.expect_test('audio_init: ack="ok"', 'receiver init command failed')
        self.receiver.client.child.sendline('quit:')
        self.receiver.client.expect_test('DEBUG:quit', 'Command not acknowledged')
        time.sleep(2)

    def xtest_02_single_audio_transmission(self):
        print self.receiver.server.child
        print self.receiver.client.child
        self.receiver.client.child.sendline('audio_init: codec="raw" port=10000 address="127.0.0.1" ')
        self.receiver.client.expect_test('audio_init: ack="ok"', 'receiver init command failed')
        self.receiver.client.child.sendline('quit:')
        self.receiver.client.expect_test('DEBUG:quit', 'Command not acknowledged')
        time.sleep(1)

    def test_02_single_audio_transmission(self):
        # we test with one milhouse sending, one receiving
        self.receiver.client.child.sendline('audio_init: codec="raw" port=10000 address="127.0.0.1" channels=2')
        self.receiver.client.expect_test('audio_init: ack="ok"', 'receiver init command failed')
        self.sender.client.child.sendline('audio_init: codec="raw" port=10000 address="127.0.0.1" source="audiotestsrc" channels=2')
        self.sender.client.expect_test('audio_init: ack="ok"', 'sender init command failed', 1)
        self.receiver.client.child.sendline('start:')
        self.receiver.client.expect_test('start: ack="ok"', 'receiver start command failed', 1)
        self.sender.client.child.sendline('start:')
        self.sender.client.expect_test('start: ack="ok"', 'sender start command failed', 1)
        time.sleep(15)
        self.receiver.client.child.sendline('stop:')
        self.receiver.client.expect_test('stop: ack="ok"', 'receiver stop command failed', 1)
        self.sender.client.child.sendline('stop:')
        self.sender.client.expect_test('stop: ack="ok"', 'sender stop command failed', 1)
        self.receiver.client.child.sendline('quit:')
        self.receiver.client.expect_test('DEBUG:quit', 'receiver quit command failed')
        self.sender.client.child.sendline('quit:')
        self.sender.client.expect_test('DEBUG:quit', 'sender quit command failed')

    def test_99_close(self):
        try:
            self.receiver.client.child.sendline('quit:')
            self.receiver.client.expect_test('DEBUG:quit', 'Command not acknowledged')
            self.sender.client.child.sendline('quit:')
            self.sender.client.expect_test('DEBUG:quit', 'Command not acknowledged')
            
        except Exception, e:
            self.sender.kill_client_and_server()
            self.receiver.kill_client_and_server()
            print e.message
