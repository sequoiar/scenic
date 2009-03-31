#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Miville
# Copyright (C) 2008 Société des arts technoligiques (SAT)
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
System test for the settings using the telnet UI.
Usage: trial test/systest_settings.py
""" 

import os
import sys
import time
import miville.utils.telnet_testing as testing

# starts the miville server internally !
testing.VERBOSE_CLIENT = False
testing.VERBOSE_SERVER = False
testing.START_SERVER = True  

testing.start()

class Test_001_Gen_Settings(testing.TelnetBaseTest):
    """
    System Tests for presets
    """

    def test_00_yes(self):
        self.expectTest('pof: ', 'The default prompt is not appearing.')
    
    def test_01_add_2_media_settings(self):
        self.tst("settings --type media --add med1" , "Media setting added")
        self.tst("settings --type media --list"     , "med1")
        self.tst("settings --type media --add med2" , "Media setting added")
        self.tst("settings --type media --list"     , "med2")
        self.tst("settings --type media --list"     , "med1")
        self.tst("settings --type media --erase med1"     , 'setting removed')
        self.tst("settings --type media --erase med2"     , 'setting removed')
        
    def test_02_save_basic_video_streaming_settings(self):
        # print "HOME IS: " + os.environ['HOME']
        # add a contact
        self.tst("contacts --add testmelonglongtime 127.0.0.1", "Contact added")
        self.tst("contacts --select testmelonglongtime", "Contact selected")   
        self.tst("contacts --modify port=2223","Contact modified")     
        
        # add media setting
        self.tst("settings --type media --add mpeg4_basic_tx", "Media setting added")
        # list again, check that the new setting is there
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=codec:mpeg4'           , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=bitrate:2048000'       , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=engine:Gst'            , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=GstPort:11111'         , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=GstAddress:127.0.0.1'  , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=source:videotestsrc'   , 'modified')       
        self.tst("settings --type media --list"                                             ,"mpeg4_basic_tx")
        
        # add media setting
        self.tst("settings --type media --add mpeg4_basic_rx"   , "Media setting added")
        # list media settings, check that the new setting is there
        self.tst("settings --type media --list"                 , "mpeg4_basic_rx")
        # check that first setting is still there, too
        self.tst("settings --type media --list"                 , 'mpeg4_basic_tx')
        
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=codec:mpeg4'    , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=engine:Gst'     , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=GstPort:11111'         , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=GstAddress:127.0.0.1'  , 'modified')
        self.tst("settings --type media --list"     , "mpeg4_basic_rx")
        
        # add global setting
        self.tst("settings --type global --add vid_tx_setting", "Global setting added")
        # best effort to check that the id of the vid_tx_setting's id 10000
        self.tst("settings --type global --list"                   , '10000')  
        # set the global setting id of the contact to vid_tx_setting's id
        self.tst("contacts --modify setting=10000","Contact modified") 
        
        self.tst("settings --type global --globalsetting vid_tx_setting --modify communication='2way'", "modified")
        self.tst("settings --type global --list"                   , 'vid_tx_setting')
        # add subgroup
        self.tst("settings --type streamsubgroup -g vid_tx_setting --add send"                             , "subgroup added")
        self.tst("settings --type streamsubgroup -g vid_tx_setting --subgroup send --modify enabled=True"  , "modified")
        self.tst("settings --type streamsubgroup -g vid_tx_setting --subgroup send --modify port=10000"    , "modified")
        self.tst("settings --type streamsubgroup -g vid_tx_setting --subgroup send --modify mode='send'"   , "modified")
        self.tst("settings --type streamsubgroup -g vid_tx_setting --list"                             , "send")
        
        # add media stream
        self.tst("settings --type stream --globalsetting vid_tx_setting --subgroup send --add video"                    , "Media stream added")
        self.tst("settings --type stream --globalsetting vid_tx_setting --subgroup send --mediastream video01 --modify setting=10000"  , "modified")
        self.tst("settings --type stream --globalsetting vid_tx_setting --subgroup send --mediastream video01 --modify enabled=True"  , "modified")
                
        self.tst("settings --type stream --globalsetting vid_tx_setting --subgroup send --list"                         , "video01")
        self.tst("s --type global --list"                   , 'vid_tx_setting')
        time.sleep(0.2)
        
        self.tst("settings --save"                          , "saved", timeout=5.0)
        time.sleep(0.2)
        self.tst("s --type global --list"                   , 'vid_tx_setting')

        # time.sleep(2)
        # self.tst("streams --start testmelonglongtime", "started")
        # time.sleep(5)
        
        self.tst("s -t media --list"                        , 'mpeg4_basic_tx')
        self.tst("s -t media --erase mpeg4_basic_tx"     , 'setting removed')
        self.tst("s -t media --erase mpeg4_basic_rx"     , 'setting removed')
        self.tst("c --erase testmelonglongtime"          , "Contact deleted")
        self.tst("s -t global --erase vid_tx_setting"           , 'setting removed')
        
        
