#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technoligiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
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
System test for the settings using the telnet UI.
Usage: trial test/systest_settings.py
""" 

import os
import sys
import test.systest_telnet


class TestBase(test.systest_telnet.TelnetBaseTest):
    
    def tst(self, command, expected, errorMsg = None):
        self.client.sendline(command)
        self.sleep()
        err = errorMsg or 'The command did not return: "%s" as expected' % expected
        self.expectTest(expected, err)

class Test_004_Settings(TestBase):
    """
    System Tests for presets
    """
    def test_00_yes(self):
        self.expectTest('pof: ', 'The default prompt is not appearing.')
        
    def test_01_add_list_delete_modify_global_setting(self):
       
        # list settings -> empty
        self.tst('settings --type global --list', 'no global settings')
        # add setting
        self.tst("s -t global -a MyFirstGlobalSetting","Global setting added")
        # list again, check that the new setting is there
        self.tst("s -t global -l", "MyFirstGlobalSetting")
        # select bogus setting
        self.tst("s -t global -s twit", '"twit" does not exist')
        # select real setting 
        self.tst("s -t global -s MyFirstGlobalSetting", 'Global setting selected')
        # list again, check that the new setting is selected
        self.tst("s -t global -l", "<---")
        # delete bogus setting
        self.tst("s -t global -e twit", '"twit" does not exist')
        # delete setting
        self.tst("s -t global -e MyFirstGlobalSetting", 'setting removed')
    
    def test_02_modify_global_setting(self):
        # modify
        # add setting named foobarbaz
        self.tst("s -t global -a foobarbaz","Global setting added")
        # check that it's there
        self.tst('settings --type global --list', 'foobarbaz')
        # rename it
        self.tst("s -t global -g foobarbaz -m name='zabraboof'", "modified")
        # check that the new name is there 
        self.tst('settings --type global --list', 'zabraboof')
        self.tst("s -t global -e zabraboof", 'setting removed')

    def test_03_add_list_delete_media_setting(self):   
        # list settings -> empty
        self.tst('settings -t media --list', 'no media settings')
        # add setting
        self.tst("s -t media -a aMediaSetting","Media setting added")
        # list again, check that the new setting is there
        self.tst("s -t media -l", "aMediaSetting")
        # select bogus setting
        self.tst("s -t media -s twit", '"twit" does not exist')
        # select real setting 
        self.tst("s -t media -s aMediaSetting", 'Media setting selected')
        # list again, check that the new setting is selected
        self.tst("s -t media -l", "<---")
        # delete bogus setting
        self.tst("s -t media -e twit", '"twit" does not exist')
        # delete setting
        self.tst("s -t media -e aMediaSetting", 'removed')
        
        
    def test_04_modify_media_setting(self):    
        # modify
        # add setting named foobarbaz
        self.tst("settings --type media -a foobarbaz","added")
        # check that it's there
        self.tst('settings --type media -l', 'foobarbaz')
        # rename it
        self.tst("settings --type media --mediasetting foobarbaz --modify name='zabraboof'", "modified")
        # check that the new name is there 
        self.tst('settings -t media --list', 'zabraboof')
        # clean up
        self.tst("s -t media -e zabraboof", 'removed')     

    def test_05_add_list_delete_streamsubgroup_setting(self):       
        # create global settings
        self.tst("settings --type global -a gset01","Global setting added")
        self.tst("settings --type global -a gset02","Global setting added") 
        self.tst('settings -t streamsubgroup -g bogus -l', 'Global setting "bogus" does not exist')
        # list settings -> empty
        self.tst('settings -t streamsubgroup -g gset01 -l', 'There are no streamsubgroup settings')
        # add setting
        self.tst("settings --type streamsubgroup -g gset01 -a streamsub01","added")
        # list settings for another global setting: should be empty
        self.tst('settings -t streamsubgroup -g gset02 -l', 'There are no streamsubgroup settings')
        # list again, check that the new setting is there
        self.tst("settings --type streamsubgroup -g gset01 -l", "streamsub01")
        # select bogus setting
        self.tst("settings --type streamsubgroup -g gset01 -s twit", '"twit" does not exist')
        # delete bogus setting
        self.tst("settings --type streamsubgroup -g gset01 -e twit", '"twit" does not exist')
        # delete setting
        self.tst("settings --type streamsubgroup -g gset01 -e streamsub01", 'Stream subgroup removed') 
        # check that its gone       
        self.tst("settings --type streamsubgroup -g gset01 -l", 'There are no streamsubgroup settings')
        
        # clean up
        self.tst("s -t global -e gset01", 'setting removed')
        self.tst("s -t global -e gset02", 'setting removed')

    def test_06_modify_streamsubgroup_setting(self):
        self.tst("settings --type global -a gset01","Global setting added")
        self.tst("settings --type streamsubgroup -g gset01 -a streamsub01","subgroup added")
        # check that enabled is False
        self.tst("settings --type streamsubgroup -g gset01 -l", "enabled   : False")
        # change enabled from False to True
        self.tst("settings --type streamsubgroup -g gset01 -p streamsub01 --modify enabled=True", "modified")
        # check result
        self.tst("settings --type streamsubgroup -g gset01 -l", "enabled   : True")   
        self.tst("s -t global -e gset01", 'setting removed')  
        
    def test_07_add_list_delete_media_stream(self): 
        # add global setting and streamgroup
        self.tst("settings --type global -a gset07","Global setting added")
        self.tst("settings --type streamsubgroup -g gset07 -a sub01","subgroup added")        
        # list 
        self.tst("settings --type stream -g gset07 -p sub01 -l","no media streams")
        
        # add media stream
        self.tst("settings --type stream -g gset07 -p sub01 -a audio","Media stream added")
        self.tst("settings --type stream -g gset07 -p sub01 -l", "audio01")
        
        # delete media stream
        self.tst("settings --type stream -g gset07 -p sub01 -e audio01", "Media stream removed")
        self.tst("settings --type stream -g gset07 -p sub01 -l","no media streams")    
        self.tst("settings --type stream -g gset07 -p sub01 -e audio01", "\"audio01\" does not exist")
        
        # clean up
        self.tst("s -t global -e gset07", 'setting removed')
        
        
    def test_08_modify_media_stream(self):
        self.tst("settings --type global -a gset08","Global setting added")
        self.tst("settings --type streamsubgroup -g gset08 -a sub","subgroup added")
        self.tst("settings --type stream -g gset08 -p sub -a video","Media stream added")
        self.tst("settings --type stream -g gset08 -p sub -l", "video01")
        # change port number to 666
        self.tst("settings --type stream -g gset08 -p sub -i video01 --modify port=666",  "modified")
        # check that the modification worked
        self.tst("settings --type stream -g gset08 -p sub -l", "666")
        # clean up
        self.tst("s -t global -e gset08", 'setting removed')        
        
    def test_09_save_settings(self):
        """
        test that pickling of settings works
        """
        self.tst('settings --type global --list', 'no global settings')
        self.tst('settings -t media --list', 'no media settings')
        
        self.tst("settings --type global -a gset09","Global setting added")
        self.tst("settings --type streamsubgroup -g gset09 -a sub","subgroup added")
        self.tst("settings --type stream -g gset09 -p sub -a data","Media stream added")
        self.tst("settings --type stream -g gset09 -p sub --list", "data01")
        # add media setting
        self.tst("settings --type media -a media09","Media setting added")
        # list again, check that the new setting is there
        self.tst("settings --type media --list", "media09")
        self.tst("settings --save", "saved")
        
        # clean up
        self.tst("settings --type global -e gset09", 'setting removed')
        self.tst("s -t media -e media09", 'removed') 
         
        self.tst('settings --type global --list', 'no global settings')
        self.tst("settings --type media --list", "no media settings")
        
        self.tst("settings --load","Settings loaded")
        self.tst("settings --type global --list", "gset09")
        self.tst("settings --type media --list", "media09")

        
          
   