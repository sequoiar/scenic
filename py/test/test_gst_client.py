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

from twisted.trial import unittest
from twisted.internet import reactor 


from miville.settings import Settings, GlobalSetting, MediaSetting
import miville.utils.log
import time
import miville.services

#del gst_client.log
#gst_client.log = miville.utils.log.start('error', 1, 0, 'gst')

def go(duration=0.1): # num=999
    """
    Runs the reactor for n seconds
    """
    end = time.time() + duration
    while time.time() < end:
        reactor.iterate()

def create_simple_video_tx_setting(settings):
    id = settings.add_media_setting('mpeg4_basic_tx')
    media_setting_mpeg4_basic = settings.get_media_setting('mpeg4_basic_tx')
    media_setting_mpeg4_basic.settings['bitrate'] = 2048000
    media_setting_mpeg4_basic.settings['codec'] = 'mpeg4'
    media_setting_mpeg4_basic.settings['service'] = 'Gst'
    media_setting_mpeg4_basic.settings['source'] = 'videotestsrc'
    media_setting_mpeg4_basic.settings['GstPort'] = 11111
    media_setting_mpeg4_basic.settings['GstAddress'] = "127.0.0.1"    
    return (id,media_setting_mpeg4_basic)

def create_simple_video_rx_setting(settings):
    id = settings.add_media_setting('mpeg4_basic_rx')
    media_setting_mpeg4_basic = settings.get_media_setting('mpeg4_basic_rx')
    media_setting_mpeg4_basic.settings['bitrate'] = 2048000
    media_setting_mpeg4_basic.settings['codec'] = 'mpeg4'
    media_setting_mpeg4_basic.settings['service'] = 'Gst'
    media_setting_mpeg4_basic.settings['GstPort'] = 11111
    media_setting_mpeg4_basic.settings['GstAddress'] = "127.0.0.1"
    return (id,media_setting_mpeg4_basic)

class GstTestListener(object):
    """
    Simple class to get feedback from...
    """
    def pof(self):
        print "pof"
    
    def notify(self, state, message):
        print 'GstTestListener.Notify("%s", "%s")' % (str(state), str(message) )

class Test_GST_client(unittest.TestCase):


    def test_01_simple_video_sender(self):
        settings = Settings()
        global_setting = GlobalSetting('gs_simple_video_sender')
        global_setting.communication = 'unidirectional'
        # no others
        global_setting.add_stream_subgroup('send')
        subgroup = global_setting.get_stream_subgroup('send')
        subgroup.enabled = True
        subgroup.port = 14000
        subgroup.mode  = 'send'
        video_stream =  subgroup.add_media_stream('video')
        video_stream.enabled = True
        
        id, media_setting_mpeg4_basic = create_simple_video_tx_setting(settings)
        video_stream.setting = media_setting_mpeg4_basic.id
        
        listener = GstTestListener()
        # create video settings
        address = "127.0.0.1"
#        go(0.1)
#        #global_setting.start_streaming(listener, address, 'toto')
#        go(5.0)
#        
#        #global_setting.stop_streaming()
#        go(1.0)
        

#    def xtest_02_simple_video_rx_tx(self):        
#        tx_settings = Settings()
#        tx_global_setting = GlobalSetting('gs_simple_video_tx')
#        tx_global_setting.communication = 'bidirectional'
#        tx_global_setting.add_stream_subgroup('send')
#        tx_subgroup = tx_global_setting.get_stream_subgroup('send')
#        tx_subgroup.enabled = True
#        tx_subgroup.port = 14000
#        tx_subgroup.mode  = 'send'
#        tx_video_stream =  tx_subgroup.add_media_stream('video')
#        tx_video_stream.enabled = True
#        
#        id, tx_media_setting_mpeg4_basic = create_simple_video_rx_setting(tx_settings)
#        tx_video_stream.setting = tx_media_setting_mpeg4_basic.id
#        
#        tx_listener = GstTestListener()
#        go(0.1)
#        rx_settings = Settings()
#        rx_global_setting = GlobalSetting('gs_simple_video_rx')
#        rx_global_setting.communication = 'bidirectional'
#        rx_global_setting.add_stream_subgroup('receive')
#        rx_subgroup = rx_global_setting.get_stream_subgroup('receive')
#        rx_subgroup.enabled = True
#        rx_subgroup.port = 14000
#        rx_subgroup.mode  = 'receive'
#        rx_video_stream =  rx_subgroup.add_media_stream('video')
#        rx_video_stream.enabled = True
#        
#        id, tx_media_setting_mpeg4_basic = create_simple_video_tx_setting(rx_settings)
#        tx_video_stream.setting = tx_media_setting_mpeg4_basic.id
#        
#        tx_listener = GstTestListener()
#        
#        go(0.1)
#        tx_global_setting.start_streaming(tx_listener, "127.0.0.1")
#        go(0.5)
#        rx_global_setting.start_streaming(tx_listener, "127.0.0.1")
#        go(5.0)
#        
#        
#        tx_global_setting.stop_streaming()
#        go(0.1)
#        rx_global_setting.stop_streaming()
#        go(0.5)
  
        
        