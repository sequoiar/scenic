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
Starts two milhouse and tests the protocols/pinger module.
"""
import unittest
from test import lib_clientserver as clientserver
import os
import time

fname = __file__.split('.')[0]

class Test_MilhouseOneWay(unittest.TestCase):
    def setUp(self):

        self.sender = clientserver.TelnetMilhouseTester(name='sender', mode='s', serverport=9000)
        self.sender.setup(self)
        self.receiver = clientserver.TelnetMilhouseTester(name='receiver', mode='r', serverport=9001)
        self.receiver.setup(self)

    def test_01_basic_milhouse_control(self):
        self.sender.sendline('audio_init: codec="raw" port=10000 address="127.0.0.1" ')
        self.sender.expect_test('audio_init: ack="ok"', 'receiver init command failed')

    def test_02_single_audio_transmission_10sec(self):
        # we test with one milhouse sending, one receiving
        self.receiver.sendline('audio_init: codec="raw" port=10000 address="127.0.0.1" channels=2 audio_buffer_usec=50000')
        self.receiver.expect_test('audio_init: ack="ok"', 'receiver init command failed')
        self.sender.sendline('audio_init: codec="raw" port=10000 address="127.0.0.1" source="audiotestsrc" channels=2')
        self.sender.expect_test('audio_init: ack="ok"', 'sender init command failed', 1)
        self.receiver.sendline('start:')
        self.receiver.expect_test('start: ack="ok"', 'receiver start command failed', 1)
        self.sender.sendline('start:')
        self.sender.expect_test('start: ack="ok"', 'sender start command failed', 1)
        time.sleep(10)
        self.receiver.sendline('stop:')
        self.receiver.expect_test('stop: ack="ok"', 'receiver stop command failed', 1)
        self.sender.sendline('stop:')
        self.sender.expect_test('stop: ack="ok"', 'sender stop command failed', 1)
    
    def test_03_single_video_transmission(self):
        self.sender.sendline('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc"')
        self.sender.expect_test('video_init: ack="ok"', 'receiver init command failed')

    def test_04_single_audio_transmission_10sec(self):
        # we test with one milhouse sending, one receiving
        self.receiver.sendline('video_init: codec="mpeg4" port=10000 address="127.0.0.1"')
        self.receiver.expect_test('video_init: ack="ok"', 'receiver init command failed')
        self.sender.sendline('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc"')
        self.sender.expect_test('video_init: ack="ok"', 'sender init command failed', 1)
        self.receiver.sendline('start:')
        self.receiver.expect_test('start: ack="ok"', 'receiver start command failed', 1)
        self.sender.sendline('start:')
        self.sender.expect_test('start: ack="ok"', 'sender start command failed', 1)
        time.sleep(10)
        self.receiver.sendline('stop:')
        self.receiver.expect_test('stop: ack="ok"', 'receiver stop command failed', 1)
        self.sender.sendline('stop:')
        self.sender.expect_test('stop: ack="ok"', 'sender stop command failed', 1)
    
    def tearDown(self):
        self.receiver.sendline('quit:')
        self.receiver.expect_test('postQuit', 'receiver quit command failed')
        self.sender.sendline('quit:')
        self.sender.expect_test('postQuit', 'sender quit command failed')
        self.sender.kill_children()
        self.receiver.kill_children()

