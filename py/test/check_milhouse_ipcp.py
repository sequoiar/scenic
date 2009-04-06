#!/usr/bin/env python
# -*- coding: utf-8 -*-

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
System test for the GST telnet UI.

Usage: trial test/systest_telnet.py

pexpect expected strings are regular expression. See the re module.
""" 
import test.lib_milhouse_ipcp as testing
import time
import os

# global constants

EXECUTABLE = "milhouse" 
# used to be "propulseart"
#video_src = "videotestsrc"
video_src = "videotestsrc"

audio_src = 'audiotestsrc'
#audio_src = 'alsrc'

video_init_ok = 'video_init: ack="ok"'
audio_init_ok = 'audio_init: ack="ok"'
start_ok = 'start: ack="ok"'
stop_ok =  'stop: ack="ok"'
stop_but_not_ok =  ''


class Test_milhouse_ipcp(testing.Milhouse_IPCP_Base_Test):
    """
    Tests for the IPCP communication with milhouse process.

    Runs a series of streaming tests with milhouse.
    The tests starts and stop various milhouse processes in sender and receiver mode,
    and controls them via a telnet protocol.  
    """

    def test_02_simple_video_sender(self):
        """
        create a video sender process that waits for connections. 
        
        The process is controlled from port 1220 and its videostream 
        is set on port 12020 
        The port numbers are printed for debuggind convenience.
        The process is then stopped without any streaming.
        
        This tests that the process exists and responds to basic commands.
        """
        telnet_tx_port = 1220
        stream_port = 12020 
        bitrate = 3000000
        
        video_init = testing.VideoInit()
        video_init.codec = 'h264'
        video_init.port = stream_port
        video_init.address = "127.0.0.1"
        video_init.bitrate=bitrate
        video_init.source=video_src
        tx_init_msg = video_init.to_string()
        
        self.verb('')
        self.verb( tx_init_msg) 
        # start process and a telnet on port 1200 for control
        self.start_propulseart_tx(telnet_tx_port) 
        # start a sender on port 12000
        self.tst_tx(tx_init_msg, video_init_ok)
        self.tst_tx("start:", start_ok) # start streaming
        self.stream_duration(2.)
        self.tst_tx("stop:", stop_but_not_ok)
        
    def test_03_simple_video_receiver(self):
        """
        Same as test2, but this time the process is started in receiver mode
        """
        telnet_rx_port = 1230
        rx_video_init = testing.VideoInit()
        rx_video_init.port = 12030
        rx_video_init.address = "127.0.0.1"
        rx_video_init.codec = 'h264'
        
        rx_init_msg = rx_video_init.to_string()
        self.verb('')
        self.verb(rx_init_msg)
        self.start_propulseart_rx(telnet_rx_port)
        
        self.tst_rx(rx_init_msg, video_init_ok)
        self.tst_rx("start:", start_ok) # start streaming
        
        self.stream_duration(2.)
        
        self.tst_rx("stop:", stop_ok)
 
    def test_04_video_sender_receiver(self):
        """
        Finally, we get some streaming going: a receiver and a sender are started and stopped.
        During the 5 second sleep, the video stream should appear on the screen.
        
        This test should check for actual transfered data when the command is available.
        """
        self.verb('')
        telnet_tx_port = 1340
        telnet_rx_port = 1240
        
        rx_video_init = testing.VideoInit()
        rx_video_init.port = 12040
        rx_video_init.address =  "127.0.0.1"
        rx_video_init.codec = 'mpeg4' 
        rx_init_command = rx_video_init.to_string()
        self.verb("RX> " + rx_init_command)
        
        tx_video_init = testing.VideoInit()
        tx_video_init.bitrate = 3000000
        tx_video_init.codec = rx_video_init.codec
        tx_video_init.address = "127.0.0.1"
        tx_video_init.port = rx_video_init.port
        tx_video_init.source=video_src
        tx_init_command = tx_video_init.to_string()
        self.verb( "TX> " + tx_init_command)
        
        self.start_propulseart_rx(telnet_rx_port)
        self.start_propulseart_tx(telnet_tx_port)
        
        self.tst_rx( rx_init_command, video_init_ok )
        self.tst_tx( tx_init_command, video_init_ok )
        
        self.tst_rx("start:", start_ok)
        self.tst_tx("start:", start_ok)
        self.stream_duration(5.)
        # check for transfered data
        # should be well above 0 bytes at this point
        
        self.tst_tx("stop:", stop_ok)
        self.tst_rx("stop:", stop_but_not_ok)
        
    def test_05_audio_send_receiv(self):
        """
        Streaming audio test betweeen 2 instances of propulseart.
        """
        
        self.verb('')
        telnet_rx_port = 1250
        telnet_tx_port = 1350
        
        rx_init = testing.AudioInit()
        rx_init.codec = 'vorbis'
        rx_init.port = telnet_rx_port
        rx_init.address = "127.0.0.1"
        rx_init.audio_buffer_usec = 30000
        rx_init_cmd = rx_init.to_string()
        self.verb( "RX:>" + rx_init_cmd)
        
        tx_init = testing.AudioInit()
        tx_init.codec = 'vorbis'
        tx_init.port = telnet_rx_port
        tx_init.address = "127.0.0.1"
        tx_init.source=audio_src 
        tx_init.channels=2
        tx_init_cmd = rx_init.to_string()
        self.verb( "TX:>" + tx_init_cmd)
        self.start_propulseart_rx(telnet_rx_port)
        self.start_propulseart_tx(telnet_tx_port)
        
        self.tst_rx(rx_init_cmd, audio_init_ok )
        self.tst_tx(tx_init_cmd, audio_init_ok)
        # make it so
        self.tst_rx("start:", start_ok)
        self.tst_tx("start:", start_ok)
        # check for transfered data
        # should be well above 0 bytes at this point
        
        
        self.stream_duration(5.)
        # cleanup and go home
        self.verb("stopping the streaming...")
 
 
        self.tst_tx("stop:", stop_but_not_ok) # CARAMBA!
        self.tst_rx("stop:", stop_ok)
        #
 
