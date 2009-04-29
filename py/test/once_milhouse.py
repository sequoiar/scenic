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

    def proceed(self):
        self.receiver.start()
        self.sender.start()
        time.sleep(5)
        self.receiver.stop()
        self.sender.stop()

    def test_01_basic_milhouse_control(self):
        self.sender.send_expect('audio_init: codec="raw" port=10000 address="127.0.0.1"', 'audio_init: ack="ok"')

    def test_02_audiotestsrc_raw(self):
        # we test with one milhouse sending, one receiving
        self.receiver.send_expect('audio_init: codec="raw" port=10000 address="127.0.0.1"', 'audio_init: ack="ok"')
        self.sender.send_expect('audio_init: codec="raw" port=10000 address="127.0.0.1" source="audiotestsrc" channels=2', 'audio_init: ack="ok"')
        self.proceed()
    
    def test_03_videotestsrc_mpeg4(self):
        # we test with one milhouse sending, one receiving
        self.receiver.send_expect('video_init: codec="mpeg4" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
        self.sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc"', 'video_init: ack="ok"')
        self.proceed()
        
    def test_04_videotestsrc_mpeg4_audiotestsrc_raw(self):
        # we test with one milhouse sending, one receiving
        # If we init video first on one end, we must init it first on the other end
        self.receiver.send_expect('video_init: codec="mpeg4" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
        self.sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc"', 'video_init: ack="ok"')
        self.receiver.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1"', 'audio_init: ack="ok"')
        self.sender.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1" source="audiotestsrc" channels=2', 'audio_init: ack="ok"')
        self.proceed()

    def test_05_v4l2_mpeg4_audiotestsrc_raw(self):
        self.receiver.send_expect('video_init: codec="mpeg4" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
        self.sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="v4l2src"', 'video_init: ack="ok"')
        self.receiver.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1"', 'audio_init: ack="ok"')
        self.sender.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1" source="audiotestsrc" channels=2', 'audio_init: ack="ok"')
        self.proceed()

    def test_06_dv1394src_mpeg4(self):
        self.receiver.send_expect('video_init: codec="mpeg4" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
        self.sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="dv1394src"', 'video_init: ack="ok"')
        self.proceed()

    def test_07_dv1394src_raw(self):
        self.receiver.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1"', 'audio_init: ack="ok"')
        self.sender.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1" source="dv1394src" channels=2', 'audio_init: ack="ok"')
        self.proceed()

    def test_08_dv1394src_mpeg4_dv1394src_raw(self):
        self.receiver.send_expect('video_init: codec="mpeg4" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
        self.sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="dv1394src"', 'video_init: ack="ok"')
        self.receiver.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1"', 'audio_init: ack="ok"')
        self.sender.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1" source="dv1394src" channels=2', 'audio_init: ack="ok"')
        self.proceed()

    def test_09_filesrc_raw(self):
        self.receiver.send_expect('video_init: codec="mpeg4" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
        self.sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="dv1394src"', 'video_init: ack="ok"')
        self.receiver.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1"', 'audio_init: ack="ok"')
        self.sender.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1" source="dv1394src" channels=2', 'audio_init: ack="ok"')
        self.proceed()

    
    def tearDown(self):
        self.receiver.quit()
        self.sender.quit()
        self.receiver.kill_children()
        self.sender.kill_children()

class Test_MilhouseTwoWay(unittest.TestCase):
    def setUp(self):
        self.local_sender = clientserver.TelnetMilhouseTester(name='local_sender', mode='s', serverport=9000)
        self.local_sender.setup(self)
        self.local_receiver = clientserver.TelnetMilhouseTester(name='local_receiver', mode='r', serverport=9001)
        self.local_receiver.setup(self)
        self.remote_sender = clientserver.TelnetMilhouseTester(name='remote_sender', mode='s', serverport=9002)
        self.remote_sender.setup(self)
        self.remote_receiver = clientserver.TelnetMilhouseTester(name='remote_receiver', mode='r', serverport=9003)
        self.remote_receiver.setup(self)

    def proceed(self):
        self.local_receiver.start()
        self.remote_receiver.start()
        self.local_sender.start()
        self.remote_sender.start()
        time.sleep(15)
        self.local_receiver.stop()
        self.remote_receiver.stop()
        self.local_sender.stop()
        self.remote_sender.stop()

    def test_01_audio_transmission_5sec(self):
        # we test with one milhouse sending, one receiving
        self.local_receiver.send_expect('audio_init: codec="raw" port=10000 address="127.0.0.1"', 'audio_init: ack="ok"')
        self.remote_receiver.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1"', 'audio_init: ack="ok"')
        self.local_sender.send_expect('audio_init: codec="raw" port=10000 address="127.0.0.1" source="audiotestsrc" channels=2', 'audio_init: ack="ok"')
        self.remote_sender.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1" source="audiotestsrc" channels=2', 'audio_init: ack="ok"')
        self.proceed()

    def test_02_video_transmission_5sec(self):
        # we test with one milhouse sending, one receiving
        self.local_receiver.send_expect('video_init: codec="mpeg4" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
        self.remote_receiver.send_expect('video_init: codec="mpeg4" port=10010 address="127.0.0.1"', 'video_init: ack="ok"')
        self.local_sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10010 address="127.0.0.1" source="videotestsrc"', 'video_init: ack="ok"')
        self.remote_sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc"', 'video_init: ack="ok"')
        self.proceed()

    def test_03_audio_video_transmission_5sec(self):
        # we test with one milhouse sending, one receiving
        self.local_receiver.send_expect('audio_init: codec="raw" port=10000 address="127.0.0.1"', 'audio_init: ack="ok"')
        self.local_receiver.send_expect('video_init: codec="mpeg4" port=10010 address="127.0.0.1"', 'video_init: ack="ok"')
        self.remote_sender.send_expect('audio_init: codec="raw" port=10000 address="127.0.0.1" source="audiotestsrc" channels=2', 'audio_init: ack="ok"')
        self.remote_sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10010 address="127.0.0.1" source="videotestsrc"', 'video_init: ack="ok"')
        self.proceed()

    def test_04_video_audio_transmission_5sec(self):
        # we test with one milhouse sending, one receiving
        self.local_receiver.send_expect('video_init: codec="mpeg4" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
        self.local_receiver.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1"', 'audio_init: ack="ok"')
        self.remote_receiver.send_expect('video_init: codec="mpeg4" port=10020 address="127.0.0.1"', 'video_init: ack="ok"')
        self.remote_receiver.send_expect('audio_init: codec="raw" port=10030 address="127.0.0.1" ', 'audio_init: ack="ok"')
        self.remote_sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc"', 'video_init: ack="ok"')
        self.remote_sender.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1" source="audiotestsrc" channels=2', 'audio_init: ack="ok"')
        self.local_sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10020 address="127.0.0.1" source="videotestsrc"', 'video_init: ack="ok"')
        self.local_sender.send_expect('audio_init: codec="raw" port=10030 address="127.0.0.1" source="audiotestsrc" channels=2', 'audio_init: ack="ok"')
        self.proceed()

    def tearDown(self):
        self.local_receiver.quit()
        self.local_sender.quit()
        self.local_receiver.kill_children()
        self.local_sender.kill_children()
        self.remote_receiver.quit()
        self.remote_sender.quit()
        self.remote_receiver.kill_children()
        self.remote_sender.kill_children()
