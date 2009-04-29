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
    
    

    


    def _add_media_stream(self, global_setting, group, type, setting, port = None):
        #add media stream        
        self.tst("settings --type stream --globalsetting %s --subgroup %s --add %s" % (global_setting, group, type), "Media stream added")         
        self.tst("settings --type stream --globalsetting %s --subgroup %s --mediastream %s01 --modify setting=%d"   % (global_setting, group, type, setting), "modified")
        self.tst("settings --type stream --globalsetting %s --subgroup %s --mediastream %s01 --modify enabled=True" % (global_setting, group, type), "modified")
        if port != None:
            self.tst("settings --type stream --globalsetting %s --subgroup %s --mediastream %s01 --modify port=%d"  % (global_setting, group, type, port),  "modified")        
        
    def _add_global_setting_video_rx(self, name, setting, port):
        self.tst("settings --type global --add %s" % name, "Global setting added")
        # add subgroup
        self.tst("settings --type streamsubgroup -g %s --add recv" % name, "subgroup added")
        self.tst("settings --type streamsubgroup -g %s --subgroup recv --modify enabled=True" % name,"modified")     
        self.tst("settings --type streamsubgroup -g %s --subgroup recv --modify mode='receive'" % name,  "modified" )                                                                                      
        self._add_media_stream(name, 'recv', 'video', setting , port)

    def _add_global_setting_video_tx(self, name, setting, port):
        self.tst("settings --type global --add %s" % name, "Global setting added")
        # add subgroup
        self.tst("settings --type streamsubgroup -g %s --add send" % name, "subgroup added")
        self.tst("settings --type streamsubgroup -g %s --subgroup send --modify enabled=True" % name,"modified")      
        self.tst("settings --type streamsubgroup -g %s --subgroup send --modify mode='send'" %name ,"modified")                                                                                       
        #add media stream   
        self._add_media_stream(name, 'send', 'video', setting, port )     
        
    def _add_global_setting_audio_rx(self, name, setting, port):   
        self.tst("settings --type global --add %s" % name, "Global setting added")
         # add subgroup
        
        self.tst("settings --type streamsubgroup -g %s --add recv"% name, "subgroup added")
        self.tst("settings --type streamsubgroup -g %s --subgroup recv --modify enabled=True" % name ,"modified")     
        self.tst("settings --type streamsubgroup -g %s --subgroup recv --modify mode='receive'" % name,"modified")         
        
        #add media stream     
        self._add_media_stream(name , 'recv', 'audio', setting, port )    

    def _add_global_setting_audio_tx(self, name, setting, port):
        self.tst("settings --type global --add %s" % name , "Global setting added")
        # add subgroup
        self.tst("settings --type streamsubgroup -g %s --add send" % name, "subgroup added")
        self.tst("settings --type streamsubgroup -g %s --subgroup send --modify enabled=True" % name,"modified")      
        self.tst("settings --type streamsubgroup -g %s --subgroup send --modify mode='send'" % name ,"modified")                                                                                      
        #add media streams   
        self._add_media_stream(name , 'send', 'audio', setting ,  port)     
       
    def _add_global_setting_video_rxtx(self, name, setting, port):        
        ###Video Two way           
        self.tst("settings --type global --add %s" % name, "Global setting added")
        # add subgroup
        self.tst("settings --type streamsubgroup -g %s --add recv" % name , "subgroup added")
        self.tst("settings --type streamsubgroup -g %s --subgroup recv --modify enabled=True" % name ,"modified")     
        self.tst("settings --type streamsubgroup -g %s --subgroup recv --modify mode='receive'" % name ,"modified")                                                                                      
        #add media stream      
        self._add_media_stream(name , 'recv', 'video', setting,  port)   

        # add subgroup
        self.tst("settings --type streamsubgroup -g %s --add send" % name, "subgroup added")
        self.tst("settings --type streamsubgroup -g %s --subgroup send --modify enabled=True" % name,"modified")      
        self.tst("settings --type streamsubgroup -g %s --subgroup send --modify mode='send'" % name ,"modified")                                                                                       
        
        #add media stream     
        self._add_media_stream(name , 'send', 'video', setting ,  port)    

###################################################################################

    def _add_global_setting_audio_rxtx(self, name, setting_rx, setting_tx, port_rx=None, port_tx=None):
        ###Audio Two way
        self.tst("settings --type global --add %s" % name , "Global setting added")
        # add subgroup
        self.tst("settings --type streamsubgroup -g %s --add recv" % name, "subgroup added")
        self.tst("settings --type streamsubgroup -g %s --subgroup recv --modify enabled=True" % name ,"modified")     
        self.tst("settings --type streamsubgroup -g %s --subgroup recv --modify mode='receive'" % name ,"modified")                                                                                      
        #add media stream  
        self._add_media_stream(name , 'recv', 'audio', setting_rx, port_rx )       
        # add subgroup
        self.tst("settings --type streamsubgroup -g %s --add send" % name, "subgroup added")
        self.tst("settings --type streamsubgroup -g %s --subgroup send --modify enabled=True" % name,"modified")     
        self.tst("settings --type streamsubgroup -g %s --subgroup send --modify mode='receive'" % name ,"modified")                                                                                      
        #add media stream    
        self._add_media_stream(name , 'send', 'audio', setting_tx,  port_tx)     

#############################################################################################
      
    def _add_global_setting_AV_rxtx(self, name, setting, port):
        ####AV Two way sync
        self.tst("settings --type global --add %s" % name , "Global setting added")
        # add subgroup
        self.tst("settings --type streamsubgroup -g %s --add recv" % name, "subgroup added")
        self.tst("settings --type streamsubgroup -g %s --subgroup recv --modify enabled=True" %name ,"modified")     
        self.tst("settings --type streamsubgroup -g %s --subgroup recv --modify mode='receive'" % name ,"modified")                                                                                      
        # add media streams rx video     
        self._add_media_stream(name , 'recv', 'video', setting , port) 
       
        # add media streams rx audio   
        self._add_media_stream( name , 'recv', 'audio', setting ,  port ) 

        ###AV Two way not sync
        # add subgroup
        self.tst("settings --type streamsubgroup -g %s --add send" % name, "subgroup added")
        self.tst("settings --type streamsubgroup -g %s --subgroup send --modify enabled=True" % name ,"modified")     
        self.tst("settings --type streamsubgroup -g %s --subgroup send --modify mode='send'" % name ,"modified")                                                                                      
        #add media stream        
        self._add_media_stream(name , 'send', 'video', setting , port ) 

        #add media stream        
        self._add_media_stream( name , 'send', 'audio', setting ,  port) 
        
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
    
    def  _add_media_settings_audio_rxtx(self, name, codec, channels, source):
        self.media_settings.append("%s_tx" % name)
        self.tst("settings --type media --add %s_tx" % name, "Media setting added")
        self.tst('settings --type media --mediasetting %s_tx  --modify settings=codec:%s' % (name,codec) , 'modified')
        self.tst('settings --type media --mediasetting %s_tx  --modify settings=engine:Gst' % (name) , 'modified')
        self.tst('settings --type media --mediasetting %s_tx  --modify settings=source:%s' % (name,source)    , 'modified')       
        self.tst('settings --type media --mediasetting %s_tx  --modify settings=channels:%d' % (name, channels) , 'modified')         
            
        self.media_settings.append("%s_rx" % name)
        self.tst("settings --type media --add %s_rx" % name , "Media setting added")
        self.tst('settings --type media --mediasetting %s_rx  --modify settings=codec:%s' % (name,codec) , 'modified')    
        self.tst('settings --type media --mediasetting %s_rx  --modify settings=engine:Gst'% (name), 'modified')
        self.tst('settings --type media --mediasetting %s_rx  --modify settings=source:%s' % (name, source) , 'modified') 
        self.tst('settings --type media --mediasetting %s_rx  --modify settings=channels:%d' % (name, channels) , 'modified')
        self.tst('settings --type media --mediasetting %s_rx  --modify settings=audio_buffer_usec:30000' % (name), 'modified')              


    def  _add_media_settings_video_rxtx(self, name, codec, source, bitrate):

        self.media_settings.append("%s_rx" % name)
        self.tst("settings --type media --add %s_rx" % name, "Media setting added")
        self.tst('settings --type media --mediasetting %s_rx  --modify settings=codec:%s' % (name,codec)          , 'modified')
        self.tst('settings --type media --mediasetting %s_rx  --modify settings=bitrate:2048000' % name       , 'modified')
        self.tst('settings --type media --mediasetting %s_rx  --modify settings=engine:Gst'  % name          , 'modified')
        self.tst('settings --type media --mediasetting %s_rx  --modify settings=source:%s' % ( name , source)  , 'modified') 
                                                             

        self.media_settings.append("%s_tx" % name)
        self.tst("settings --type media --add %s_tx" % name, "Media setting added")
        self.tst('settings --type media --mediasetting %s_tx  --modify settings=codec:%s' % (name,codec)           , 'modified')
        self.tst('settings --type media --mediasetting %s_tx  --modify settings=bitrate:%d' % (name,bitrate)      , 'modified')
        self.tst('settings --type media --mediasetting %s_tx  --modify settings=engine:Gst'   % name         , 'modified')
        self.tst('settings --type media --mediasetting %s_tx  --modify settings=source:%s' % (name, source)   , 'modified')       
        


    def test_02_media_settings(self):
        
        #videosrc = 'v4l2src'
        videosrc = 'videotestsrc'
        
        print
        print "HOME IS: " + os.environ['HOME']
        
        self.media_settings = []
        
        self._add_media_settings_video_rxtx("video_mpeg4", "mpeg4", videosrc, 3000000 )
        self._add_media_settings_video_rxtx("video_h263", "h263", videosrc, 3000000)
                
              
        self._add_media_settings_audio_rxtx("audio_raw2", "raw", 2, "audiotestsrc")
        self._add_media_settings_audio_rxtx("audio_raw4", "raw", 4, "audiotestsrc")
        self._add_media_settings_audio_rxtx("audio_raw6", "raw", 6, "audiotestsrc")
        self._add_media_settings_audio_rxtx("audio_raw8", "raw", 8, "audiotestsrc")
        self._add_media_settings_audio_rxtx("audio_mp3", "mp3", 2, "audiotestsrc")
        self._add_media_settings_audio_rxtx("audio_vorbis2", "vorbis", 2, "audiotestsrc")
        self._add_media_settings_audio_rxtx("audio_vorbis4", "vorbis", 4, "audiotestsrc")
        self._add_media_settings_audio_rxtx("audio_vorbis6", "vorbis", 6, "audiotestsrc")
        self._add_media_settings_audio_rxtx("audio_vorbis8", "vorbis", 8, "audiotestsrc")
        

        counter = 10000
        for media in  self.media_settings:       
            print "media[%5d] %s" % (counter,media)
            counter += 1

        
        
       
        
        self.add_global_settings()


    def add_global_settings(self):
    

        # add global setting
        #10000      
        setting_id = 10000 + self.media_settings.index('video_mpeg4_rx')
        self._add_global_setting_video_rx('video_mpeg4_rx', setting_id, 6676)
        #10001      
        setting_id = 10000 + self.media_settings.index('video_mpeg4_tx')
        self._add_global_setting_video_tx('video_mpeg4_tx', setting_id, 6686)
        #10002
        setting_id = 10000 + self.media_settings.index('video_h263_rx')
        self._add_global_setting_video_rx('video_h263_rx', setting_id, 6696)
        #10003
        setting_id = 10000 + self.media_settings.index('video_h263_tx')
        self._add_global_setting_video_tx('video_h263_tx', setting_id, 6716)
        #10004   
        setting_id = 10000 + self.media_settings.index('audio_raw2_tx')
        self._add_global_setting_audio_rx('audio_raw2_tx', setting_id, 6726)
        #10005        
        setting_id = 10000 + self.media_settings.index('audio_raw2_rx')
        self._add_global_setting_audio_tx('audio_raw2_rx', setting_id, 6736)
        #10006
        setting_id = 10000 + self.media_settings.index('audio_raw4_rx')
        self._add_global_setting_audio_rx('audio_raw4_rx', setting_id, 6746)
        #10007
        setting_id = 10000 + self.media_settings.index('audio_raw4_tx')
        self._add_global_setting_audio_tx('audio_raw4_tx', setting_id, 6756)
        #10008
        setting_id = 10000 + self.media_settings.index('audio_raw6_rx')
        self._add_global_setting_audio_rx('audio_raw6_rx', setting_id, 6766)
        #10009
        setting_id = 10000 + self.media_settings.index('audio_raw6_tx')
        self._add_global_setting_audio_tx('audio_raw6_tx', setting_id, 6776)
        #10010
        setting_id = 10000 + self.media_settings.index('audio_raw8_rx')
        self._add_global_setting_audio_rx('audio_raw8_rx', setting_id, 6786)
        #10011
        setting_id = 10000 + self.media_settings.index('audio_raw8_tx')
        self._add_global_setting_audio_tx('audio_raw8_tx', setting_id, 6796)
        #10012
        setting_id = 10000 + self.media_settings.index('audio_mp3_tx')
        self._add_global_setting_audio_tx('audio_mp3_rx', setting_id, 6816)
        #10013
        setting_id = 10000 + self.media_settings.index('audio_mp3_rx')
        self._add_global_setting_audio_rx('audio_mp3_tx', setting_id, 6826)
        #10014
        setting_id = 10000 + self.media_settings.index('audio_vorbis2_rx')
        self._add_global_setting_audio_tx('audio_vorbis2_rx', setting_id, 6836)
        #10015
        setting_id = 10000 + self.media_settings.index('audio_vorbis2_tx')
        self._add_global_setting_audio_rx('audio_vorbis2_tx', setting_id, 6846)
        #10016
        setting_id = 10000 + self.media_settings.index('audio_vorbis4_rx')
        self._add_global_setting_audio_tx('audio_vorbis4_rx', setting_id, 6856)
        #10017
        setting_id = 10000 + self.media_settings.index('audio_vorbis4_tx')
        self._add_global_setting_audio_rx('audio_vorbis4_tx', setting_id, 6866)
        #10018
        setting_id = 10000 + self.media_settings.index('audio_vorbis6_rx')
        self._add_global_setting_audio_tx('audio_vorbis6_rx', setting_id, 6876)
        #10019
        setting_id = 10000 + self.media_settings.index('audio_vorbis6_tx')
        self._add_global_setting_audio_rx('audio_vorbis6_tx', setting_id, 6886)        
        #10020
        setting_id = 10000 + self.media_settings.index('audio_vorbis8_rx')
        self._add_global_setting_audio_tx('audio_vorbis8_rx', setting_id, 6896)
        #10021
        setting_id = 10000 + self.media_settings.index('audio_vorbis8_tx')
        self._add_global_setting_audio_rx('audio_vorbis8_tx', setting_id, 6916)
        #10022
        setting_id_tx = 10000 + self.media_settings.index('audio_raw8_tx')
        setting_id_rx = 10000 + self.media_settings.index('audio_raw8_rx')
        self._add_global_setting_audio_rxtx('audio_raw8_rxtx', setting_id_rx, setting_id_tx , 6916, 6926)
        
        self.tst("settings --save", "saved")


    def atest_04_contacts(self):  
        # add a contacts
        settings = [10000]
        self._add_contact('brrr', '10.10.10.65',  settings)
        self._add_contact('tzing', '10.10.10.66', settings)
        self._add_contact('krrt', '10.10.10.64', settings)
        self._add_contact('toc', '10.10.10.169', settings) 
        self._add_contact('pow', '10.10.10.182', settings)
        self._add_contact('bloup', '10.10.10.72', settings)   
        self._add_contact('gloup', '10.10.10.73', settings)    
        self._add_contact('flush', '10.10.10.69', settings)
 
    
  



