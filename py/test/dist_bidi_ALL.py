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





class Test_001_network_streaming(testing.TelnetBaseTest):
   

    def test_01_yes(self):
        self.expectTest('pof: ', 'The default prompt is not appearing.')

    

    
        
    def test_02_settings(self):
        print
        print "HOME IS: " + os.environ['HOME']
        # add a contact
          
        self.tst("contacts --add bloup 10.10.10.72", "Contact added")
        self.tst("contacts --select bloup", "Contact selected")   
        self.tst("contacts --modify port=2222","Contact modified")  
    

        # add global setting
        #10000
        self.tst("settings --type global --add video_rx", "Global setting added")
        #10001        
        self.tst("settings --type global --add video_tx", "Global setting added")
        #10002        
        self.tst("settings --type global --add audio_rx", "Global setting added")
        #10003        
        self.tst("settings --type global --add audio_tx", "Global setting added")
        #10004
        ###Video Two way           
        self.tst("settings --type global --add video_rxtx", "Global setting added")
        #10005        
        ###Audio Two way
        self.tst("settings --type global --add audio_rxtx", "Global setting added")
        #10006
        ####AV Two way sync
        self.tst("settings --type global --add AV_rxtx", "Global setting added")
        
        ###AV Two way not sync
        

####################################################################################################
################################Receiver settings###############################################
#############################################################################################

        
################################VIDEO###############################################

      
       
        # add media receiver setting 
        self.tst("settings --type media --add mpeg4_basic_rx", "Media setting added")
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=codec:mpeg4'           , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=bitrate:2048000'       , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=engine:Gst'            , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_rx  --modify settings=source:v4l2src'   , 'modified') 

################For one way video##########

        # add subgroup
        self.tst("settings --type streamsubgroup -g video_rx --add recv", "subgroup added")
        self.tst("settings --type streamsubgroup -g video_rx --subgroup recv --modify enabled=True","modified")     
        self.tst("settings --type streamsubgroup -g video_rx --subgroup recv --modify mode='receive'","modified")                                                                                      
        #add media stream        
        self.tst("settings --type stream --globalsetting video_rx --subgroup recv --add video", "Media stream added")         
        self.tst("settings --type stream --globalsetting video_rx --subgroup recv --mediastream video01 --modify setting=10000", "modified")
        self.tst("settings --type stream --globalsetting video_rx --subgroup recv --mediastream video01 --modify port=6666", "modified")        
        self.tst("settings --type stream --globalsetting video_rx --subgroup recv --mediastream video01 --modify enabled=True", "modified")


################For two way video##########

        
          # add subgroup
        self.tst("settings --type streamsubgroup -g video_rxtx --add recv", "subgroup added")
        self.tst("settings --type streamsubgroup -g video_rxtx --subgroup recv --modify enabled=True","modified")     
        self.tst("settings --type streamsubgroup -g video_rxtx --subgroup recv --modify mode='receive'","modified")                                                                                      
        #add media stream        
        self.tst("settings --type stream --globalsetting video_rxtx --subgroup recv --add video", "Media stream added")         
        self.tst("settings --type stream --globalsetting video_rxtx --subgroup recv --mediastream video01 --modify setting=10000", "modified")
        self.tst("settings --type stream --globalsetting video_rxtx --subgroup recv --mediastream video01 --modify port=6666", "modified")        
        self.tst("settings --type stream --globalsetting video_rxtx --subgroup recv --mediastream video01 --modify enabled=True", "modified")


#################################################################################################
    #####################audio    
    ############################################################################################ 

#####one way audio#####


        #self.tst("contacts --modify setting=10002","Contact modified") 
        # add media receiver setting 
        self.tst("settings --type media --add audio_basic_rx", "Media setting added")
        self.tst('settings --type media --mediasetting audio_basic_rx  --modify settings=codec:raw', 'modified')    
        self.tst('settings --type media --mediasetting audio_basic_rx  --modify settings=engine:Gst', 'modified')
        self.tst('settings --type media --mediasetting audio_basic_rx  --modify settings=source:audiotestsrc', 'modified') 
        self.tst('settings --type media --mediasetting audio_basic_rx  --modify settings=channels:2', 'modified')
        self.tst('settings --type media --mediasetting audio_basic_rx  --modify settings=audio_buffer_usec:30000', 'modified')                                                                      
        
         # add subgroup
        self.tst("settings --type streamsubgroup -g audio_rx --add recv", "subgroup added")
        self.tst("settings --type streamsubgroup -g audio_rx --subgroup recv --modify enabled=True","modified")     
        self.tst("settings --type streamsubgroup -g audio_rx --subgroup recv --modify mode='receive'","modified")         

        #add media stream        
        self.tst("settings --type stream --globalsetting audio_rx --subgroup recv --add audio", "Media stream added")         
        self.tst("settings --type stream --globalsetting audio_rx --subgroup recv --mediastream audio01 --modify setting=10001", "modified")
        self.tst("settings --type stream --globalsetting audio_rx --subgroup recv --mediastream audio01 --modify port=6686", "modified")        
        self.tst("settings --type stream --globalsetting audio_rx --subgroup recv --mediastream audio01 --modify enabled=True", "modified")


################For two way audio##########
       
          # add subgroup
        self.tst("settings --type streamsubgroup -g audio_rxtx --add recv", "subgroup added")
        self.tst("settings --type streamsubgroup -g audio_rxtx --subgroup recv --modify enabled=True","modified")     
        self.tst("settings --type streamsubgroup -g audio_rxtx --subgroup recv --modify mode='receive'","modified")                                                                                      
        #add media stream        
        self.tst("settings --type stream --globalsetting audio_rxtx --subgroup recv --add audio", "Media stream added")         
        self.tst("settings --type stream --globalsetting audio_rxtx --subgroup recv --mediastream audio01 --modify setting=10001", "modified")
        self.tst("settings --type stream --globalsetting audio_rxtx --subgroup recv --mediastream audio01 --modify port=6666", "modified")        
        self.tst("settings --type stream --globalsetting audio_rxtx --subgroup recv --mediastream audio01 --modify enabled=True", "modified")
        
        
####################################################
#####Two way audio and video
####################################################

  # add subgroup
        self.tst("settings --type streamsubgroup -g AV_rxtx --add recv", "subgroup added")
        self.tst("settings --type streamsubgroup -g AV_rxtx --subgroup recv --modify enabled=True","modified")     
        self.tst("settings --type streamsubgroup -g AV_rxtx --subgroup recv --modify mode='receive'","modified")                                                                                      
        #add media stream        
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup recv --add video", "Media stream added")         
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup recv --mediastream video01 --modify setting=10000", "modified")
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup recv --mediastream video01 --modify port=6696", "modified")
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup recv --mediastream video01 --modify sync_group=\"video\"", "modified")        
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup recv --mediastream video01 --modify enabled=True", "modified")

                                                                                 
        #add media stream        
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup recv --add audio", "Media stream added")         
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup recv --mediastream audio01 --modify setting=10001", "modified")
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup recv --mediastream audio01 --modify sync_group=\"audio\"", "modified")
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup recv --mediastream audio01 --modify port=6666", "modified")        
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup recv --mediastream audio01 --modify enabled=True", "modified")







        
#######################################################################################################        
################################TRANSMIT SETTINGS####################################################
#######################################################################################################
  
        

################################VIDEO###############################################

        self.tst("contacts --modify setting=10001","Contact modified") 
        
       
        self.tst("settings --type media --add mpeg4_basic_tx", "Media setting added")
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=codec:mpeg4'           , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=bitrate:2048000'       , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=engine:Gst'            , 'modified')
        self.tst('settings --type media --mediasetting mpeg4_basic_tx  --modify settings=source:v4l2src'   , 'modified')       

        # add subgroup
        self.tst("settings --type streamsubgroup -g video_tx --add send", "subgroup added")
        self.tst("settings --type streamsubgroup -g video_tx --subgroup send --modify enabled=True","modified")      
        self.tst("settings --type streamsubgroup -g video_tx --subgroup send --modify mode='send'","modified")                                                                                       
        
        #add media stream        
        self.tst("settings --type stream --globalsetting video_tx --subgroup send --add video", "Media stream added")         
        self.tst("settings --type stream --globalsetting video_tx --subgroup send --mediastream video01 --modify setting=10002", "modified")
        self.tst("settings --type stream --globalsetting video_tx --subgroup send --mediastream video01 --modify port=7000", "modified")        
        self.tst("settings --type stream --globalsetting video_tx --subgroup send --mediastream video01 --modify enabled=True", "modified")

###################for two way video###########################

# add subgroup
        self.tst("settings --type streamsubgroup -g video_rxtx --add send", "subgroup added")
        self.tst("settings --type streamsubgroup -g video_rxtx --subgroup send --modify enabled=True","modified")      
        self.tst("settings --type streamsubgroup -g video_rxtx --subgroup send --modify mode='send'","modified")                                                                                       
        
        #add media stream        
        self.tst("settings --type stream --globalsetting video_rxtx --subgroup send --add video", "Media stream added")         
        self.tst("settings --type stream --globalsetting video_rxtx --subgroup send --mediastream video01 --modify setting=10002", "modified")
        self.tst("settings --type stream --globalsetting video_rxtx --subgroup send --mediastream video01 --modify port=7000", "modified")        
        self.tst("settings --type stream --globalsetting video_rxtx --subgroup send --mediastream video01 --modify enabled=True", "modified")


###############################################################################################
    #####################AUDIO    
    ############################################################################################   

        
        self.tst("contacts --modify setting=10003","Contact modified") 
        # add media setting 
        self.tst("settings --type media --add audio_basic_tx", "Media setting added")
        self.tst('settings --type media --mediasetting audio_basic_tx  --modify settings=codec:raw'           , 'modified')
        self.tst('settings --type media --mediasetting audio_basic_tx  --modify settings=engine:Gst'            , 'modified')
        self.tst('settings --type media --mediasetting audio_basic_tx  --modify settings=source:audiotestsrc'   , 'modified')       
        self.tst('settings --type media --mediasetting audio_basic_tx  --modify settings=channels:2', 'modified')

        # add subgroup
        self.tst("settings --type streamsubgroup -g audio_tx --add send", "subgroup added")
        self.tst("settings --type streamsubgroup -g audio_tx --subgroup send --modify enabled=True","modified")      
        self.tst("settings --type streamsubgroup -g audio_tx --subgroup send --modify mode='send'","modified")                                                                                    
        
        #add media stream        
        self.tst("settings --type stream --globalsetting audio_tx --subgroup send --add audio", "Media stream added")         
        self.tst("settings --type stream --globalsetting audio_tx --subgroup send --mediastream audio01 --modify setting=10003", "modified")
        self.tst("settings --type stream --globalsetting audio_tx --subgroup send --mediastream audio01 --modify port=7200", "modified")        
        self.tst("settings --type stream --globalsetting audio_tx --subgroup send --mediastream audio01 --modify enabled=True", "modified")

###################for two way audio###########################
  # add subgroup
        self.tst("settings --type streamsubgroup -g audio_rxtx --add send", "subgroup added")
        self.tst("settings --type streamsubgroup -g audio_rxtx --subgroup send --modify enabled=True","modified")     
        self.tst("settings --type streamsubgroup -g audio_rxtx --subgroup send --modify mode='send'","modified")                                                                                      
        #add media stream        
        self.tst("settings --type stream --globalsetting audio_rxtx --subgroup send --add audio", "Media stream added")         
        self.tst("settings --type stream --globalsetting audio_rxtx --subgroup send --mediastream audio01 --modify setting=10003", "modified")
        self.tst("settings --type stream --globalsetting audio_rxtx --subgroup send --mediastream audio01 --modify port=6686", "modified")        
        self.tst("settings --type stream --globalsetting audio_rxtx --subgroup send --mediastream audio01 --modify enabled=True", "modified")


################For two way video and audio ##########

        # add subgroup
        self.tst("settings --type streamsubgroup -g AV_rxtx --add send", "subgroup added")
        self.tst("settings --type streamsubgroup -g AV_rxtx --subgroup send --modify enabled=True","modified")     
        self.tst("settings --type streamsubgroup -g AV_rxtx --subgroup send --modify mode='send'","modified")                                                                                      

        #add media stream        
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup send --add video", "Media stream added")         
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup send --mediastream video01 --modify setting=10002", "modified")
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup send --mediastream video01 --modify port=6680", "modified") 
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup send --mediastream video01 --modify sync_group=\"video\"", "modified")       
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup send --mediastream video01 --modify enabled=True", "modified")

        
        #add media stream        
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup send --add audio", "Media stream added")         
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup send --mediastream audio01 --modify setting=10003", "modified")
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup send --mediastream video01 --modify sync_group=\"audio\"", "modified")
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup send --mediastream audio01 --modify port=7200", "modified")        
        self.tst("settings --type stream --globalsetting AV_rxtx --subgroup send --mediastream audio01 --modify enabled=True", "modified")





####################################################################################################################



        
        time.sleep(2)
        self.tst("c -s bloup", "")
        self.tst("j -s ", "")
        time.sleep(2)

        
        
        #self.tst("contacts --modify setting=10000","Contact modified")
        #self.tst("streams --start bloup", "")
        #time.sleep(2)
        #self.tst("contacts --modify setting=10001","Contact modified")

        #self.tst("streams --start bloup", "")

        #time.sleep(2)
        
        #self.tst("contacts --modify setting=10002","Contact modified")

        #self.tst("streams --start bloup", "")

        #time.sleep(2)

        #self.tst("contacts --modify setting=10003","Contact modified")

        #self.tst("streams --start bloup", "")    
        

        self.tst("settings --save", "saved")
        time.sleep(2)
        time.sleep(50)
    
  



