#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technoligiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, eithercd sr version 2 of the License, or
# (at your option) any later version.
#
# Sropulpof is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Sropulpof.  If not, see <http:#www.gnu.org/licenses/>.

"""

System test for ditributed telnet, remote script
Usage: 
On local machine:trial test/dist_tenet_sys_test3.py



""" 

import os
import sys





import test.systest_telnet
import time


#VERBOSE_CLIENT = False
VERBOSE_CLIENT = True

#VERBOSE_SERVER = False
VERBOSE_SERVER = True

class TestBase(test.systest_telnet.TelnetBaseTest):
    """
    Convenient little gem of a class with a tst method that
    sends a telnet request and tests for an expected reply.
    """  
    def tst(self, command, expected, timeout=2, errorMsg = None):
        
#        
        self.client.sendline(command)
        err = errorMsg or 'The command did not return: "%s" as expected' % expected
        self.expectTest(expected, err, timeout=timeout)
        time.sleep(0.5)
        b = self.client.buffer.rstrip()
        b = b.lstrip()
        print "\n============================================"    
        print command
        b = b.replace ('\r', '')
        print b

class Test_001_Gen_Settings(TestBase):
    """
    System Tests for presets
    """

    def test_00_yes(self):
        self.expectTest('pof: ', 'The default prompt is not appearing.')
    
 ################################Receiver settings###############################################   
        
    def test_02_save_basic_video_streaming_settings(self):
        print
        print "HOME IS: " + os.environ['HOME']
        # add a contact
	#self.tst("contacts -e bloup", "Contact removed")
        self.tst("contacts --add bloup 10.10.10.72", "Contact added")
        self.tst("contacts --select bloup", "Contact selected")   
        self.tst("contacts --modify port=2222","Contact modified")     
        
        
        
	# add media setting
        self.tst("settings --type media --add mpeg4_basic_rx"   , "Media setting added")
        # list media settings, check that the new setting is there
      
	self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=codec:mpeg4', 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=bitrate:2048000','modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=engine:Gst', 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=GstPort:11111' , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=GstAddress:127.0.0.1', 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=source:videotestsrc', 'modified')       
      

        
        # add global setting

        self.tst("settings --type global --add vid_rx_setting", "Global setting added")

        # set the global setting id of the contact to vidgdb _rx_setting's id

        self.tst("contacts --modify setting=10001","Contact modified") 
        
        self.tst("settings --type global --globalsetting vid_rx_setting --modify communication='2way'", "modified")
        
        # add subgroup

        self.tst("settings --type streamsubgroup -g vid_rx_setting --add recv", "subgroup added")
        self.tst("settings --type streamsubgroup -g vid_rx_setting --subgroup recv --modify enabled=True","modified")
        self.tst("settings --type streamsubgroup -g vid_rx_setting --subgroup recv --modify mode='receive'","modified")                                                                                      

        # add media stream
        self.tst("settings --type stream --globalsetting vid_rx_setting --subgroup recv --add video" , "Media stream added")
        self.tst("settings --type stream --globalsetting vid_rx_setting --subgroup recv --mediastream video01 --modify setting=10000", "modified")
 	self.tst("settings --type stream --globalsetting vid_rx_setting --subgroup recv --mediastream video01 --modify port=6666","modified")   
	self.tst("settings --type stream --globalsetting vid_rx_setting --subgroup recv --mediastream video01 --modify enabled=True", "modified")              
      
       

################################transmit settings####################################################
        
        

        # add a contact
        self.tst("contacts --add gloup 10.10.10.72", "Contact added")
        self.tst("contacts --select gloup", "Contact selected")   
        self.tst("contacts --modify port=2222","Contact modified")     


               
# add media setting 
        self.tst("settings --type media --add mpeg4_basic_tx", "Media setting added")
        # list again, check that the new setting is there
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=codec:mpeg4'           , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=bitrate:2048000'       , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=engine:Gst'            , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=GstPort:11111'         , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=GstAddress:127.0.0.1'  , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=source:videotestsrc'   , 'modified')       

	# add global setting

        self.tst("settings --type global --add vid_tx_setting", "Global setting added")

        # set the global setting id of the contact to vidgdb _rx_setting's id

        self.tst("contacts --modify setting=10000","Contact modified") 
        
        self.tst("settings --type global --globalsetting vid_tx_setting --modify communication='2way'", "modified")
        
        # add subgroup

        self.tst("settings --type streamsubgroup -g vid_tx_setting --add send", "subgroup added")
        self.tst("settings --type streamsubgroup -g vid_tx_setting --subgroup send --modify enabled=True","modified")
        self.tst("settings --type streamsubgroup -g vid_tx_setting --subgroup send --modify mode='send'","modified")                                                                                      



#add media stream        

        self.tst("settings --type stream --globalsetting vid_tx_setting --subgroup send --add video", "Media stream added")         
        self.tst("settings --type stream --globalsetting vid_tx_setting --subgroup send --mediastream video01 --modify setting=10000", "modified")
        self.tst("settings --type stream --globalsetting vid_tx_setting --subgroup send --mediastream video01 --modify port=6666", "modified")        
        self.tst("settings --type stream --globalsetting vid_tx_setting --subgroup send --mediastream video01 --modify enabled=True", "modified")

	time.sleep(2)
        self.tst("streams --start bloup", "")
        time.sleep(10)


       
        self.tst("streams --start gloup", "")
        time.sleep(50000000)      
        
