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
Distibuted telnet system test, local file 
Usage: On local machine: trial test/dist_telnet_sys_test3.py IP_ADDRESS
You should set your env variables first. 
""" 
import unittest
import pexpect
import os
import time
import sys

import test.lib_deprecated_miville_telnet as testing

testing.VERBOSE_CLIENT = True #True
testing.VERBOSE_SERVER = False
testing.START_SERVER = False # You must start miville manually on both local and remote host.
testing.start()



class Test_Generate_Settings(testing.TelnetBaseTest):
   
    def test_01_yes(self):
        self.expectTest('pof: ', 'The default prompt is not appearing.')
    
    def _add_media_setting_mpeg4_basic_rx(self, source):
        self.tst("settings --type media --add mpeg4_basic_rx", "Media setting added")
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=codec:mpeg4'           , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=bitrate:2048000'       , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=engine:Gst'            , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=source:%s' % source  , 'modified') 
        
    def _add_media_setting_audio_basic_rx(self):
        self.tst("settings --type media --add audio_basic_rx", "Media setting added")
        self.tst('settings --type media --mediasetting audio_basic_rx  --modify settings=codec:raw', 'modified')    
        self.tst('settings --type media --mediasetting audio_basic_rx  --modify settings=engine:Gst', 'modified')
        self.tst('settings --type media --mediasetting audio_basic_rx  --modify settings=source:audiotestsrc', 'modified') 
        self.tst('settings --type media --mediasetting audio_basic_rx  --modify settings=channels:2', 'modified')
        self.tst('settings --type media --mediasetting audio_basic_rx  --modify settings=audio_buffer_usec:30000', 'modified')                                                                      

    def _add_media_setting_mpeg4_basic_tx(self, source):
        self.tst("settings --type media --add mpeg4_basic_tx", "Media setting added")
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=codec:mpeg4'           , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=bitrate:2048000'       , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=engine:Gst'            , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=source:%s' % source   , 'modified')       

    def _add_media_setting_audio_basic_tx(self):
        self.tst("settings --type media --add audio_basic_tx", "Media setting added")
        self.tst('settings --type media --mediasetting audio_basic_tx  --modify settings=codec:raw'           , 'modified')
        self.tst('settings --type media --mediasetting audio_basic_tx  --modify settings=engine:Gst'            , 'modified')
        self.tst('settings --type media --mediasetting audio_basic_tx  --modify settings=source:audiotestsrc'   , 'modified')       
        self.tst('settings --type media --mediasetting audio_basic_tx  --modify settings=channels:2', 'modified')


    def _add_media_setting(self, ):

    def _add_global_setting_video_rx(self):
        self.tst("settings --type global --add video_rx", "Global setting added")
        # add subgroup
        self.tst("settings --type streamsubgroup -g video_rx --add recv", "subgroup added")
        self.tst("settings --type streamsubgroup -g video_rx --subgroup recv --modify enabled=True","modified")     
        self.tst("settings --type streamsubgroup -g video_rx --subgroup recv --modify mode='receive'","modified")                                                                                      
        #add media stream        
        self.tst("settings --type stream --globalsetting video_rx --subgroup recv --add video", "Media stream added")         
        self.tst("settings --type stream --globalsetting video_rx --subgroup recv --mediastream video01 --modify setting=10000", "modified")
        self.tst("settings --type stream --globalsetting video_rx --subgroup recv --mediastream video01 --modify port=6666", "modified")        
        self.tst("settings --type stream --globalsetting video_rx --subgroup recv --mediastream video01 --modify enabled=True", "modified")

    def _add_global_setting_video_tx(self):
        self.tst("settings --type global --add video_tx", "Global setting added")
        # add subgroup
        self.tst("settings --type streamsubgroup -g video_tx --add send", "subgroup added")
        self.tst("settings --type streamsubgroup -g video_tx --subgroup send --modify enabled=True","modified")      
        self.tst("settings --type streamsubgroup -g video_tx --subgroup send --modify mode='send'","modified")                                                                                       
        #add media stream        
        self.tst("settings --type stream --globalsetting video_tx --subgroup send --add video", "Media stream added")         
        self.tst("settings --type stream --globalsetting video_tx --subgroup send --mediastream video01 --modify setting=10002", "modified")
        self.tst("settings --type stream --globalsetting video_tx --subgroup send --mediastream video01 --modify port=7000", "modified")        
        self.tst("settings --type stream --globalsetting video_tx --subgroup send --mediastream video01 --modify enabled=True", "modified")     



    def _add_global_setting_audio_rx(self):   
        self.tst("settings --type global --add audio_rx", "Global setting added")
         # add subgroup
        self.tst("settings --type streamsubgroup -g audio_rx --add recv", "subgroup added")
        self.tst("settings --type streamsubgroup -g audio_rx --subgroup recv --modify enabled=True","modified")     
        self.tst("settings --type streamsubgroup -g audio_rx --subgroup recv --modify mode='receive'","modified")         

        #add media stream        
        self.tst("settings --type stream --globalsetting audio_rx --subgroup recv --add audio", "Media stream added")         
        self.tst("settings --type stream --globalsetting audio_rx --subgroup recv --mediastream audio01 --modify setting=10001", "modified")
        self.tst("settings --type stream --globalsetting audio_rx --subgroup recv --mediastream audio01 --modify port=6686", "modified")        
        self.tst("settings --type stream --globalsetting audio_rx --subgroup recv --mediastream audio01 --modify enabled=True", "modified")

    def _add_global_setting_audio_tx(self):
        self.tst("settings --type global --add audio_tx", "Global setting added")
        # add subgroup
        self.tst("settings --type streamsubgroup -g audio_tx --add send", "subgroup added")
        self.tst("settings --type streamsubgroup -g audio_tx --subgroup send --modify enabled=True","modified")      
        self.tst("settings --type streamsubgroup -g audio_tx --subgroup send --modify mode='send'","modified")                                                                                      
        #add media streams        
        self.tst("settings --type stream --globalsetting audio_tx --subgroup send --add audio", "Media stream added")         
        self.tst("settings --type stream --globalsetting audio_tx --subgroup send --mediastream audio01 --modify setting=10003", "modified")
        self.tst("settings --type stream --globalsetting audio_tx --subgroup send --mediastream audio01 --modify port=7200", "modified")        
        self.tst("settings --type stream --globalsetting audio_tx --subgroup send --mediastream audio01 --modify enabled=True", "modified")

    def _add_global_setting_video_rxtx(self):        
        ###Video Two way           
        self.tst("settings --type global --add video_rxtx", "Global setting added")
        # add subgroup
        self.tst("settings --type streamsubgroup -g video_rxtx --add recv", "subgroup added")
        self.tst("settings --type streamsubgroup -g video_rxtx --subgroup recv --modify enabled=True","modified")     
        self.tst("settings --type streamsubgroup -g video_rxtx --subgroup recv --modify mode='receive'","modified")                                                                                      
        #add media stream        
        self.tst("settings --type stream --globalsetting video_rxtx --subgroup recv --add video", "Media stream added")         
        self.tst("settings --type stream --globalsetting video_rxtx --subgroup recv --mediastream video01 --modify setting=10000", "modified")
        self.tst("settings --type stream --globalsetting video_rxtx --subgroup recv --mediastream video01 --modify port=6666", "modified")        
        self.tst("settings --type stream --globalsetting video_rxtx --subgroup recv --mediastream video01 --modify enabled=True", "modified")
        # add subgroup
        self.tst("settings --type streamsubgroup -g video_rxtx --add send", "subgroup added")
        self.tst("settings --type streamsubgroup -g video_rxtx --subgroup send --modify enabled=True","modified")      
        self.tst("settings --type streamsubgroup -g video_rxtx --subgroup send --modify mode='send'","modified")                                                                                       
        
        #add media stream        
        self.tst("settings --type stream --globalsetting video_rxtx --subgroup send --add video", "Media stream added")         
        self.tst("settings --type stream --globalsetting video_rxtx --subgroup send --mediastream video01 --modify setting=10002", "modified")
        self.tst("settings --type stream --globalsetting video_rxtx --subgroup send --mediastream video01 --modify port=7000", "modified")        
        self.tst("settings --type stream --globalsetting video_rxtx --subgroup send --mediastream video01 --modify enabled=True", "modified")         

    def _add_global_setting_audio_rxtx(self):
        ###Audio Two way
        self.tst("settings --type global --add audio_rxtx", "Global setting added")
        # add subgroup
        self.tst("settings --type streamsubgroup -g audio_rxtx --add recv", "subgroup added")
        self.tst("settings --type streamsubgroup -g audio_rxtx --subgroup recv --modify enabled=True","modified")     
        self.tst("settings --type streamsubgroup -g audio_rxtx --subgroup recv --modify mode='receive'","modified")                                                                                      
        #add media stream        
        self.tst("settings --type stream --globalsetting audio_rxtx --subgroup recv --add audio", "Media stream added")         
        self.tst("settings --type stream --globalsetting audio_rxtx --subgroup recv --mediastream audio01 --modify setting=10001", "modified")
        self.tst("settings --type stream --globalsetting audio_rxtx --subgroup recv --mediastream audio01 --modify port=6666", "modified")        
        self.tst("settings --type stream --globalsetting audio_rxtx --subgroup recv --mediastream audio01 --modify enabled=True", "modified")
        # add subgroup
        self.tst("settings --type streamsubgroup -g audio_rxtx --add send", "subgroup added")
        self.tst("settings --type streamsubgroup -g audio_rxtx --subgroup send --modify enabled=True","modified")     
        self.tst("settings --type streamsubgroup -g audio_rxtx --subgroup send --modify mode='receive'","modified")                                                                                      
        #add media stream        
        self.tst("settings --type stream --globalsetting audio_rxtx --subgroup send --add video", "Media stream added")         
        self.tst("settings --type stream --globalsetting audio_rxtx --subgroup send --mediastream video01 --modify setting=10003", "modified")
        self.tst("settings --type stream --globalsetting audio_rxtx --subgroup send --mediastream video01 --modify port=6666", "modified")        
        self.tst("settings --type stream --globalsetting audio_rxtx --subgroup send --mediastream video01 --modify enabled=True", "modified")
       
    def _add_global_setting_AV_rxtx(self):
        ####AV Two way sync
        self.tst("settings --type global --add AV_rxtx", "Global setting added")
        # add subgroup
        self.tst("settings --type streamsubgroup -g AV_rxtx --add recv", "subgroup added")
        self.tst("settings --type streamsubgroup -g AV_rxtx --subgroup recv --modify enabled=True","modified")     
        self.tst("settings --type streamsubgroup -g AV_rxtx --subgroup recv --modify mode='receive'","modified")                                                                                      
        # add media streams rx video     
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup recv --add video", "Media stream added")         
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup recv --mediastream video01 --modify setting=10000", "modified")
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup recv --mediastream video01 --modify port=6696", "modified")        
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup recv --mediastream video01 --modify enabled=True", "modified")                                                                    
        # add media streams rx audio   
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup recv --add audio", "Media stream added")         
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup recv --mediastream audio01 --modify setting=10001", "modified")
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup recv --mediastream audio01 --modify port=6666", "modified")        
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup recv --mediastream audio01 --modify enabled=True", "modified")        
        ###AV Two way not sync
        # add subgroup
        self.tst("settings --type streamsubgroup -g AV_rxtx --add send", "subgroup added")
        self.tst("settings --type streamsubgroup -g AV_rxtx --subgroup send --modify enabled=True","modified")     
        self.tst("settings --type streamsubgroup -g AV_rxtx --subgroup send --modify mode='send'","modified")                                                                                      
        #add media stream        
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup send --add video", "Media stream added")         
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup send --mediastream video01 --modify setting=10002", "modified")
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup send --mediastream video01 --modify port=6680", "modified")        
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup send --mediastream video01 --modify enabled=True", "modified")
        #add media stream        
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup send --add audio", "Media stream added")         
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup send --mediastream audio01 --modify setting=10003", "modified")
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup send --mediastream audio01 --modify port=7200", "modified")        
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup send --mediastream audio01 --modify enabled=True", "modified")
        
    def _add_contact(self, name, address, settings):
        
        port = 2222
        for i in settings:
            contact = name
            if len(settings) > 1:
                contact = '%s_%d' % (name,i)
                #port += 10
                
            self.tst("contacts --add %s %s %d" % (contact, address, port), "Contact added")
            self.tst("contacts --select %s" % contact, "Contact selected")   
            self.tst("contacts --modify port=%d" % port,"Contact modified") 
            self.tst("contacts --modify setting=%d" % i,"Contact modified")
    
    def test_02_media_settings(self):
        
        #video_src = 'v4l2src'
        video_src = 'videotestsrc'
        
        print
        print "HOME IS: " + os.environ['HOME']
        
        # add media setting video rx 
        # 10 000
        self._add_media_setting_mpeg4_basic_rx(video_src)

        # add media setting audio rx
        # 10 001 
        self._add_media_setting_audio_basic_rx()

        # mpeg4_basic_tx     
        # 10 002
        self._add_media_setting_mpeg4_basic_tx(video_src)

        # add media setting audio_basic_tx
        # 10 003
        self._add_media_setting_audio_basic_tx()
        self.tst("settings --save", "saved")
        # audio_rxtx
        


    def test_03_global_settings(self):
        # add global setting
        # 10 000
        self._add_global_setting_video_rx()
        self.tst("settings --save", "saved")
        # 10 001
        self._add_global_setting_video_tx()
        self.tst("settings --save", "saved")
        #10 002   
        self._add_global_setting_audio_rx()
        self.tst("settings --save", "saved")
        #10 003        
        self._add_global_setting_audio_tx()
        self.tst("settings --save", "saved")
        #10 004
        self._add_global_setting_video_rxtx()
        self.tst("settings --save", "saved")
        #10 005
        self._add_global_setting_audio_rxtx()
        self.tst("settings --save", "saved")
        #10 006
        self._add_global_setting_AV_rxtx() 
        self.tst("settings --save", "saved")
        print 'test_03_global_settings DONE'

    def test_04_contacts(self):  
        # add a contacts
        settings = range(10000, 10007)
        self._add_contact('brrr', '10.10.10.65',  settings)
        self._add_contact('tzing', '10.10.10.66', settings)
        self._add_contact('krrt', '10.10.10.64', settings)
        self._add_contact('toc', '10.10.10.169', settings) 
        self._add_contact('pow', '10.10.10.182', settings)
        self._add_contact('bloup', '10.10.10.72', settings)   
        self._add_contact('gloup', '10.10.10.73', settings)    
        self._add_contact('flush', '10.10.10.69', settings)
 
    
  



