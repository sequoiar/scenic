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


stdin, stdout, stderr = os.popen3("ssh bloup")
stdin.write("""cd /home/scormier/src/miville/trunk/py;trial test/dist_telnet_sys_test7b.py\n""")
time.sleep(5)        
stdin.write("""exit\n""")
stdin.close()
stdout.close()
stderr.close()
        




class Test_001_network_streaming(testing.TelnetBaseTest):
   

    def test_01_yes(self):
        self.expectTest('pof: ', 'The default prompt is not appearing.')

    
################################Receiver settings###############################################
    
        
    def test_02_settings(self):
        print
        print "HOME IS: " + os.environ['HOME']
        # add a contact
          
        self.tst("contacts --add bloup 10.10.10.72", "Contact added")
        self.tst("contacts --select bloup", "Contact selected")   
        self.tst("contacts --modify port=2222","Contact modified")  

         # add global setting
        self.tst("settings --type global --add all_rxtx_twoway_setting", "Global setting added")
        # set the global setting id of the contact to allgdb _tx_setting's id
        self.tst("contacts --modify setting=10000","Contact modified") 

        # add media receiver setting 
        self.tst("settings --type media --add mpeg4_basic_rx", "Media setting added")
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=codec:mpeg4'           , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=bitrate:2048000'       , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=engine:Gst'            , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=GstPort:11111'         , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=GstAddress:127.0.0.1'  , 'modified')
        #self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=source:videotestsrc'   , 'modified')    
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=source:v4l2src'   , 'modified') 


        # add subgroup
        self.tst("settings --type streamsubgroup -g all_rxtx_twoway_setting --add recv", "subgroup added")
        self.tst("settings --type streamsubgroup -g all_rxtx_twoway_setting --subgroup recv --modify enabled=True","modified")     
        self.tst("settings --type streamsubgroup -g all_rxtx_twoway_setting --subgroup recv --modify mode='receive'","modified")                                                                                      
        #add media stream        
        self.tst("settings --type stream --globalsetting all_rxtx_twoway_setting --subgroup recv --add video", "Media stream added")         
        self.tst("settings --type stream --globalsetting all_rxtx_twoway_setting --subgroup recv --mediastream video01 --modify setting=10000", "modified")
        self.tst("settings --type stream --globalsetting all_rxtx_twoway_setting --subgroup recv --mediastream video01 --modify port=6666", "modified")        
        self.tst("settings --type stream --globalsetting all_rxtx_twoway_setting --subgroup recv --mediastream video01 --modify enabled=True", "modified")


        
        
################################transmit settings####################################################
     
                    
        # add media setting 
        self.tst("settings --type media --add mpeg4_basic_tx", "Media setting added")
        # list again, check that the new setting is there
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=codec:mpeg4'           , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=bitrate:2048000'       , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=engine:Gst'            , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=GstPort:11112'         , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=GstAddress:127.0.0.1'  , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=source:v4l2src'   , 'modified')       

        # add subgroup
        self.tst("settings --type streamsubgroup -g all_rxtx_twoway_setting --add send", "subgroup added")
        self.tst("settings --type streamsubgroup -g all_rxtx_twoway_setting --subgroup send --modify enabled=True","modified")      
        self.tst("settings --type streamsubgroup -g all_rxtx_twoway_setting --subgroup send --modify mode='send'","modified")                                                                                       
        
        #add media stream        
        self.tst("settings --type stream --globalsetting all_rxtx_twoway_setting --subgroup send --add video", "Media stream added")         
        self.tst("settings --type stream --globalsetting all_rxtx_twoway_setting --subgroup send --mediastream video01 --modify setting=10001", "modified")
        self.tst("settings --type stream --globalsetting all_rxtx_twoway_setting --subgroup send --mediastream video01 --modify port=6699", "modified")        
        self.tst("settings --type stream --globalsetting all_rxtx_twoway_setting --subgroup send --mediastream video01 --modify enabled=True", "modified")
        
        
       



       
    ##################################################################################################
    #####################AUDIO    
    ############################################################################################   


   
     # add media receiver setting 
        self.tst("settings --type media --add audio_basic_rx", "Media setting added")
        self.tst('settings --type media --mediasetting audio_basic_rx  --modify settings=codec:raw', 'modified')    
        self.tst('settings --type media --mediasetting audio_basic_rx  --modify settings=engine:Gst', 'modified')
        self.tst('settings --type media --mediasetting audio_basic_rx  --modify settings=GstPort:11133' , 'modified')
        self.tst('settings --type media --mediasetting audio_basic_rx  --modify settings=GstAddress:127.0.0.1', 'modified')
        self.tst('settings --type media --mediasetting audio_basic_rx  --modify settings=source:audiotestsrc', 'modified') 
        self.tst('settings --type media --mediasetting audio_basic_rx  --modify settings=channels:2', 'modified')
        self.tst('settings --type media --mediasetting audio_basic_rx  --modify settings=audio_buffer_usec:30000', 'modified')
                                                                
        
        #add media stream        
        self.tst("settings --type stream --globalsetting all_rxtx_twoway_setting --subgroup recv --add audio", "Media stream added")         
        self.tst("settings --type stream --globalsetting all_rxtx_twoway_setting --subgroup recv --mediastream audio01 --modify setting=10002", "modified")
        self.tst("settings --type stream --globalsetting all_rxtx_twoway_setting --subgroup recv --mediastream audio01 --modify port=6689", "modified")        
        #self.tst("settings --type stream --globalsetting all_rxtx_twoway_setting --subgroup recv --mediastream audio01 --modify enabled=True", "modified")
      
        
################################transmit settings###################################################
               
        # add media setting 
        self.tst("settings --type media --add audio_basic_tx", "Media setting added")
        self.tst('settings --type media --mediasetting audio_basic_tx  --modify settings=codec:raw'           , 'modified')
        self.tst('settings --type media --mediasetting audio_basic_tx  --modify settings=engine:Gst'            , 'modified')
        self.tst('settings --type media --mediasetting audio_basic_tx  --modify settings=GstPort:11144'         , 'modified')
        self.tst('settings --type media --mediasetting audio_basic_tx  --modify settings=GstAddress:127.0.0.1'  , 'modified')
        self.tst('settings --type media --mediasetting audio_basic_tx  --modify settings=source:audiotestsrc'   , 'modified')       
        self.tst('settings --type media --mediasetting audio_basic_tx  --modify settings=channels:2', 'modified')
                                                       
        
        #add media stream        
        self.tst("settings --type stream --globalsetting all_rxtx_twoway_setting --subgroup send --add audio", "Media stream added")         
        self.tst("settings --type stream --globalsetting all_rxtx_twoway_setting --subgroup send --mediastream audio01 --modify setting=10003", "modified")
        self.tst("settings --type stream --globalsetting all_rxtx_twoway_setting --subgroup send --mediastream audio01 --modify port=7000", "modified")        
        #self.tst("settings --type stream --globalsetting all_rxtx_twoway_setting --subgroup send --mediastream audio01 --modify enabled=True", "modified")


####################################################################################################################


       
        self.tst("settings --save", "saved")
        time.sleep(2)
        self.tst("c -s bloup", "")
        self.tst("j -s ", "")
        time.sleep(2)
        #self.tst("j -s ", "")
        self.tst("streams --start bloup", "")
        time.sleep(2)
        #self.tst("streams --start bloup", "")
        time.sleep(50)
    



