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

class Test_av_sync(testing.Milhouse_IPCP_Base_Test):
    """
    One last test that does not work when done with the rest of the test suite.

    See the tests in file test_telnet_milhouse.py
    """
    def test_06_audio_video_synchronized(self): 
        """
        This test is a streaming test where audio and video are streamed 
        simultaneously with synchronisation.
        The commands for this test are not implemented yet.
        """
        self.verb('')
        telnet_rx_port = 1270
        telnet_tx_port = 1370
        
        rx_video_init = testing.VideoInit()
        rx_video_init.codec = 'mpeg4'
        rx_video_init.address = "127.0.0.1"
        rx_video_init.port = 12007
        rx_video_init_cmd = rx_video_init.to_string()
        
        rx_audio_init = testing.AudioInit()
        rx_audio_init.audio_buffer_usec = 30000
        rx_audio_init.codec = 'raw'
        rx_audio_init.port = 12107
        rx_audio_init.address = "127.0.0.1"
        rx_audio_init_cmd = rx_audio_init.to_string()
                
        # = "video_init:" + self._get_start_command(,   12007, )
           
        self.verb( 'RX video: ' + rx_video_init_cmd )
        self.verb( 'RX audio: ' + rx_audio_init_cmd )
        
        tx_video_init = testing.VideoInit()
        tx_video_init.bitrate = 3000000
        tx_video_init.codec = rx_video_init.codec
        tx_video_init.port = rx_video_init.port
        tx_video_init.address = rx_video_init.address
        tx_video_init.source = video_src
        
        tx_audio_init = testing.AudioInit()
        tx_audio_init.codec = rx_audio_init.codec
        tx_audio_init.address = rx_audio_init.address
        tx_audio_init.port = rx_audio_init.port
        tx_audio_init.source=audio_src
        tx_audio_init.channels =2
        
        tx_video_init_cmd = tx_video_init.to_string()
        tx_audio_init_cmd = tx_audio_init.to_string()
        self.verb(  'TX video: ' + tx_video_init_cmd)
        self.verb(  'TX audio: ' + tx_audio_init_cmd)
                 
        self.start_propulseart_rx(telnet_rx_port)
        self.start_propulseart_tx(telnet_tx_port)
    
        self.tst_rx(rx_video_init_cmd , video_init_ok )
        self.tst_rx(rx_audio_init_cmd , audio_init_ok  )
        
        self.tst_tx(tx_video_init_cmd,  video_init_ok)
        self.tst_tx(tx_audio_init_cmd , audio_init_ok)
                
        self.tst_tx( 'start:', start_ok )
        time.sleep(0.5)
        self.tst_rx( 'start:', start_ok )
       
        self.stream_duration(15.)
        # check number of packets TBD
        self.tst_rx("stop:", stop_but_not_ok) # CARAMBA!!!
        self.tst_tx("stop:", stop_ok)
       
