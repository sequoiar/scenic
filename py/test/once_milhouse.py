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
        self.receiver = clientserver.TelnetMilhouseTester(name='receiver', mode='r', serverport=9001)
        self.receiver.setup(self)
        self.sender = clientserver.TelnetMilhouseTester(name='sender', mode='s', serverport=9000)
        self.sender.setup(self)

    def proceed(self, rxCommands = [], txCommands = []):
        """ Proceed with or without extra commands to be called once we've started """
        time.sleep(2)
        self.receiver.send_expect('start:', 'start: ack="ok"')
        time.sleep(2)
        self.sender.send_expect('start:', 'start: ack="ok"')
        
        #self.receiver.start()
        #self.sender.start()

#       # dispatch commands to be given while we're playing
#       for com in rxCommands:
#           self.receiver.send_expect(com)
#       for com in txCommands:
#           self.sender.send_expect(com)

        print 'sleeping'
        time.sleep(5)
        #self.receiver.stop()
        #self.sender.stop()
        self.receiver.send_expect('stop:', 'stop: ack="ok"')
        self.sender.send_expect('stop:', 'stop: ack="ok"')
        

#   def test_01_basic_milhouse_control(self):
#       self.sender.send_expect('audio_init: codec="raw" port=10000 address="127.0.0.1"', 'audio_init: ack="ok"')

#   def test_02_audiotestsrc_raw(self):
#       # we test with one milhouse sending, one receiving
#       self.receiver.send_expect('audio_init: codec="raw" port=10000 address="127.0.0.1"', 'audio_init: ack="ok"')
#       self.sender.send_expect('audio_init: codec="raw" port=10000 address="127.0.0.1" source="audiotestsrc" channels=2', 'audio_init: ack="ok"')
#       self.proceed()
#   
#   def test_03_videotestsrc_mpeg4(self):
#       # we test with one milhouse sending, one receiving
#       self.receiver.send_expect('video_init: codec="mpeg4" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
#       self.sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc"', 'video_init: ack="ok"')
#       self.proceed()
#       
#   def test_04_videotestsrc_mpeg4_audiotestsrc_raw(self):
#       # we test with one milhouse sending, one receiving
#       # If we init video first on one end, we must init it first on the other end
#       self.receiver.send_expect('video_init: codec="mpeg4" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
#       self.sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc"', 'video_init: ack="ok"')
#       self.receiver.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1"', 'audio_init: ack="ok"')
#       self.sender.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1" source="audiotestsrc" channels=2', 'audio_init: ack="ok"')
#       self.proceed()

#   def test_05_v4l2_mpeg4_audiotestsrc_raw(self):
#       self.receiver.send_expect('video_init: codec="mpeg4" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
#       self.sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="v4l2src"', 'video_init: ack="ok"')
#       self.receiver.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1"', 'audio_init: ack="ok"')
#       self.sender.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1" source="audiotestsrc" channels=2', 'audio_init: ack="ok"')
#       self.proceed()

#   def test_06_dv1394src_mpeg4(self):
#       self.receiver.send_expect('video_init: codec="mpeg4" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
#       self.sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="dv1394src"', 'video_init: ack="ok"')
#       self.proceed()

#   def test_07_dv1394src_raw(self):
#       self.receiver.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1"', 'audio_init: ack="ok"')
#       self.sender.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1" source="dv1394src" channels=2', 'audio_init: ack="ok"')
#       self.proceed()

#   def test_08_dv1394src_mpeg4_dv1394src_raw(self):
#       self.receiver.send_expect('video_init: codec="mpeg4" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
#       self.sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="dv1394src"', 'video_init: ack="ok"')
#       self.receiver.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1"', 'audio_init: ack="ok"')
#       self.sender.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1" source="dv1394src" channels=2', 'audio_init: ack="ok"')
#       self.proceed()

#   def test_09_jackaudiosrc_raw(self):
#       self.receiver.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1"', 'audio_init: ack="ok"')
#       self.sender.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1" source="jackaudiosrc" channels=8', 'audio_init: ack="ok"')
#       self.proceed()

    def test_10_videotestsrc_h264(self):
        self.receiver.send_expect('video_init: codec="h264" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
        time.sleep(2)
        self.sender.send_expect('video_init: codec="h264" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc"', 'success:')
        time.sleep(2)
        self.proceed()

#   def test_11_videotestsrc_h263(self):
#       self.receiver.send_expect('video_init: codec="h263" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
#       self.sender.send_expect('video_init: codec="h263" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc"', 'video_init: ack="ok"')
#       self.proceed()

#   def test_12_audiotestsrc_mp3(self):
#       self.receiver.send_expect('video_init: codec="mp3" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
#       self.sender.send_expect('video_init: codec="mp3" port=10000 address="127.0.0.1" source="audiotestsrc"', 'video_init: ack="ok"')
#       self.proceed()
#   
#   def test_13_audiotestsrc_vorbis(self):
#       self.receiver.send_expect('video_init: codec="vorbis" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
#       self.sender.send_expect('video_init: codec="vorbis" port=10000 address="127.0.0.1" source="audiotestsrc"', 'video_init: ack="ok"')
#       self.proceed()

#   def test_14_videotestsrc_mpeg4_deinterlace(self):
#       self.receiver.send_expect('video_init: codec="mpeg4" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
#       self.sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc" deinterlace=1', 'video_init: ack="ok"')
#       self.proceed()
#   
#   def test_15_videotestsrc_mpeg4_jitterbuffer(self):
#       self.receiver.send_expect('video_init: codec="mpeg4" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
#       self.sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc"', 'video_init: ack="ok"')
#       self.proceed(rxCommands = 'jitterbuffer: latency=60')
#   
#   def test_16_jackaudiosrc_raw_audiobuffer(self):
#       self.receiver.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1" audio_buffer_usec=20000', 'audio_init: ack="ok"')
#       self.sender.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1" source="jackaudiosrc" channels=8', 'audio_init: ack="ok"')
#       self.proceed()


        #   def test_17_filesrc_raw(self):
        #      self.receiver.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1"', 'audio_init: ack="ok"')
    #      self.sender.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1" source="filesrc" location="/usr/share/example-content/ubuntu\ sax.ogg"', 'audio_init: ack="ok"')
        #      self.proceed()

        #  def test_18_filesrc_mpeg4(self):
        #      self.receiver.send_expect('video_init: codec="mpeg4" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
    #      self.sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="filesrc" location="fake"', 'video_init: ack="ok"')
        #      self.proceed()

    def tearDown(self):
        self.receiver.quit()
        self.sender.quit()
        self.receiver.kill_children()
        self.sender.kill_children()

#class Test_MilhouseTwoWay(unittest.TestCase):
#   def setUp(self):
#       self.local_sender = clientserver.TelnetMilhouseTester(name='local_sender', mode='s', serverport=9000)
#       self.local_sender.setup(self)
#       self.local_receiver = clientserver.TelnetMilhouseTester(name='local_receiver', mode='r', serverport=9001)
#       self.local_receiver.setup(self)
#       self.remote_sender = clientserver.TelnetMilhouseTester(name='remote_sender', mode='s', serverport=9002)
#       self.remote_sender.setup(self)
#       self.remote_receiver = clientserver.TelnetMilhouseTester(name='remote_receiver', mode='r', serverport=9003)
#       self.remote_receiver.setup(self)

#   def proceed(self):
#       self.local_receiver.start()
#       self.remote_receiver.start()
#       self.local_sender.start()
#       self.remote_sender.start()
#       time.sleep(15)
#       self.local_receiver.stop()
#       self.remote_receiver.stop()
#       self.local_sender.stop()
#       self.remote_sender.stop()

#   def test_01_audio_transmission_5sec(self):
#       # we test with one milhouse sending, one receiving
#       self.local_receiver.send_expect('audio_init: codec="vorbis" port=10000 address="127.0.0.1"', 'audio_init: ack="ok"')
#       self.remote_receiver.send_expect('audio_init: codec="vorbis" port=10010 address="127.0.0.1"', 'audio_init: ack="ok"')
#       self.local_sender.send_expect('audio_init: codec="vorbis" port=10000 address="127.0.0.1" source="audiotestsrc" channels=2', 'audio_init: ack="ok"')
#       self.remote_sender.send_expect('audio_init: codec="vorbis" port=10010 address="127.0.0.1" source="audiotestsrc" channels=2', 'audio_init: ack="ok"')
#       self.proceed()

#   def test_02_video_transmission_5sec(self):
#       # we test with one milhouse sending, one receiving
#       self.local_receiver.send_expect('video_init: codec="mpeg4" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
#       self.remote_receiver.send_expect('video_init: codec="mpeg4" port=10010 address="127.0.0.1"', 'video_init: ack="ok"')
#       self.local_sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10010 address="127.0.0.1" source="videotestsrc"', 'video_init: ack="ok"')
#       self.remote_sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc"', 'video_init: ack="ok"')
#       self.proceed()

#   def test_03_audio_video_transmission_5sec(self):
#       # we test with one milhouse sending, one receiving
#       self.local_receiver.send_expect('audio_init: codec="raw" port=10000 address="127.0.0.1"', 'audio_init: ack="ok"')
#       self.local_receiver.send_expect('video_init: codec="mpeg4" port=10010 address="127.0.0.1"', 'video_init: ack="ok"')
#       self.remote_sender.send_expect('audio_init: codec="raw" port=10000 address="127.0.0.1" source="audiotestsrc" channels=2', 'audio_init: ack="ok"')
#       self.remote_sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10010 address="127.0.0.1" source="videotestsrc"', 'video_init: ack="ok"')
#       self.proceed()

#   def test_04_video_audio_transmission_5sec(self):
#       # we test with one milhouse sending, one receiving
#       self.local_receiver.send_expect('video_init: codec="mpeg4" port=10000 address="127.0.0.1"', 'video_init: ack="ok"')
#       self.local_receiver.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1"', 'audio_init: ack="ok"')
#       self.remote_receiver.send_expect('video_init: codec="mpeg4" port=10020 address="127.0.0.1"', 'video_init: ack="ok"')
#       self.remote_receiver.send_expect('audio_init: codec="raw" port=10030 address="127.0.0.1" ', 'audio_init: ack="ok"')
#       self.remote_sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc"', 'video_init: ack="ok"')
#       self.remote_sender.send_expect('audio_init: codec="raw" port=10010 address="127.0.0.1" source="audiotestsrc" channels=2', 'audio_init: ack="ok"')
#       self.local_sender.send_expect('video_init: codec="mpeg4" bitrate=3000000 port=10020 address="127.0.0.1" source="videotestsrc"', 'video_init: ack="ok"')
#       self.local_sender.send_expect('audio_init: codec="raw" port=10030 address="127.0.0.1" source="audiotestsrc" channels=2', 'audio_init: ack="ok"')
#       self.proceed()

#   def tearDown(self):
#       self.local_receiver.quit()
#       self.local_sender.quit()
#       self.local_receiver.kill_children()
#       self.local_sender.kill_children()
#       self.remote_receiver.quit()
#       self.remote_sender.quit()
#       self.remote_receiver.kill_children()
#       self.remote_sender.kill_children()
