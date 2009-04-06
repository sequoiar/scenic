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
System test for the telnet UI.

Usage: trial test/systest_telnet.py

pexpect expected strings are regular expression. See the re module.
""" 
import unittest
import shutil
from test import lib_miville_telnet as libmi

local = libmi.MivilleTester(use_tmp_home=True)
local.start_miville_process()
local.start_telnet_process()
# import unittest
# import pexpect
# import os
# import time
# import sys
# 
# import miville.utils.telnet_testing as testing

# starts the miville server internally !
# testing.VERBOSE_CLIENT = False
# testing.VERBOSE_SERVER = False
# testing.START_SERVER = True  
# 
# testing.start()

# test classes
class Test_0_cli(unittest.TestCase):
    """
    General tests for the CLI
    """
    ######################################
    def setUp(self):
        global local
        local.unittest = self
        self.local = local
        self.client = local.telnet_process
    #######################################
    def test_01_default_prompt(self):
        self.local.expectTest('pof: ', 'The default prompt is not appearing.')
    def test_02_unknown_command(self):
        self.client.sendline('qwetyqrwetyqwertye')
        self.local.expectTest('command not found', 'Unknown command didn\'t give error.')
        
class Test_1_AddressBook(unittest.TestCase):
    """
    Systems tests for the Address Book
    """
    ######################################
    def setUp(self):
        global local
        local.unittest = self
        self.local = local
        self.client = local.telnet_process
    #######################################
    def test_02_add_contact(self):
        # adding a contact
        # c -a name ip [port]
        self.client.sendline("c -a Juliette 154.123.2.3")
        self.local.expectTest('Contact added', 'Contact not added')
        
    def test_03_list(self):
        self.client.sendline("c -l") 
        self.local.expectTest('Juliette:', 'The contact that has just been added is not appearing.')

    def test_04_add_duplicate(self):
        self.client.sendline("c -a Juliette 192.168.20.20")
        self.local.expectTest('Could not add contact.', 'Double entry shouldn\'t have been made.')

    def test_05_select(self):
        self.client.sendline("c -s Juliette")
        self.local.expectTest('Contact selected', 'Contact couln\'t be selected')

    def test_06_duplicate_selected(self):
        self.client.sendline("c -d Henriette")
        self.local.expectTest('Contact duplicated', 'Selected contact cannot be duplicated')

    def test_07_duplicate_named(self):
        self.client.sendline("c -d Mariette Juliette")
        self.local.expectTest('Contact duplicated', 'Specified contact cannot be duplicated')

    def test_08_modify_selected_name(self):
        self.client.sendline("c -s Mariette")
        self.client.sendline("c -m Luciette")
        self.local.expectTest('Contact modified', 'Selected contact name cannot be modified')

    def test_08_modify_selected_ip(self):
        self.client.sendline("c -m address=172.16.20.30")
        self.local.expectTest('Contact modified', 'Selected contact name cannot be modified')

    def test_09_modify_selected_name_and_ip(self):
        self.client.sendline("c -m Annette 192.168.30.30")
        self.local.expectTest('modified', 'Selected contact name cannot be modified')

    def test_98_erase_contact(self):
        self.client.sendline("c -e Juliette")
        self.local.expectTest('Contact deleted', 'Error while trying to erase contact')

    def test_99_delete_invalid_contact(self):
        self.client.sendline("c -e some_invalid_name")
        self.local.expectTest('Could not delete', 'There should be no contact with that name.')

# class DISABLED_Test_2_Audiostream(testing.TelnetBaseTest):
#     """
#     System Tests for Audiostream
#     """
#     def test_00_default_prompt(self):
#         self.local.expectTest('pof: ', 'The default prompt is not appearing.')
# 
#     def test_01_add_audiostream(self):
#         self.sleep()
#         self.client.sendline("a -a audiostream")
#         self.sleep()
#         self.local.expectTest('Audio stream audiostream created.', 'Audio stream audio cannot be created.')
# 
#     def test_02_rename_audiostream(self):
#         self.client.sendline("a -m audio audiostream")
#         self.sleep()
#         self.local.expectTest('Audio stream audiostream rename to audio.', 'Audio stream audiostream cannot be renamed to audio.')
# 
#     def test_03_change_container(self):
#         self.client.sendline("a -t mpegts audio")
#         self.sleep()
#         self.local.expectTest('container of audio stream audio is set to mpegts.', 'Container of audio stream audio was not set to mpegts.')
# 
#     def test_04_change_codec(self):
#         self.client.sendline("a -c vorbis audio")
#         self.sleep()
#         self.local.expectTest('codec of audio stream audio is set to vorbis.', 'Codec of audio stream audio was not set to vorbis.')
# 
#     def test_05_change_settings(self):
#         self.client.sendline("a -s value1:key1,value2:key2 audio")
#         self.sleep()
#         self.local.expectTest('codec_settings of audio stream audio is set to value1:key1,value2:key2.', 'Codec settings of audio stream audio was not set to new values.')
# 
#     def test_06_change_bitdepth(self):
#         self.client.sendline("a -d 24 audio")
#         self.sleep()
#         self.local.expectTest('bitdepth of audio stream audio is set to 24.', 'Bitdepth of audio stream audio was not set to 24.')
# 
#     def test_07_change_samplerate(self):
#         self.client.sendline("a -r 44100 audio")
#         self.sleep()
#         self.local.expectTest('sample_rate of audio stream audio is set to 44100.', 'Sample rate of audio stream audio was not set to 44100.')
# 
#     def test_08_change_channels(self):
#         self.client.sendline("a -v 8 audio")
#         self.sleep()
#         self.local.expectTest('channels of audio stream audio is set to 8.', 'Number of channels of audio stream audio was not set.')
# 
#     def test_09_change_port(self):
#         self.client.sendline("a -p 5050 audio")
#         self.sleep()
#         self.local.expectTest('port of audio stream audio is set to 5050.', 'IP Port of audio stream audio was not set.')
# 
#     def test_10_change_buffer(self):
#         self.client.sendline("a -b 100 audio")
#         self.sleep()
#         self.local.expectTest('buffer of audio stream audio is set to 100.', 'Buffer time of audio stream audio was not set to 44100.')
# 
# #    def test_11_change_input(self):
# #        self.client.sendline("a -i jackaudiosrc audio")
# #        self.sleep()
# #        self.local.expectTest('source of audio stream audio is set to jackaudiosrc.', 'Unable to specify audio input.')
# #    
# #    def test_12_change_codec_of_invalid_stream(self):
# #        self.client.sendline("a -c vorbis audiostream")
# #        self.sleep()
# #        self.local.expectTest('There\'s no audio stream with the name audiostream', 'IP Port of audio stream audio was not set.')
#     
#     def test_13_set_samplerate_to_invalid_value(self):
#         self.client.sendline("a -r Juliette audio")
#         self.sleep()
#         self.local.expectTest('option -r: invalid integer value: u\'Juliette\'', 'Invalid value accepted as sample rate')
#         #self.local.expectTest(AssertionError,"OK")
#         
#     def test_14_invalid_audio_stream_and_codec_order(self):
#         """
#         pof: a -c toto qweqweqwe (set codec to toto, but qweqweqwe is not a valid audio stream)
#         There's no audio stream with the name qweqweqwe
#         """
#         self.client.sendline("a -c audio qweqweqwe")
#         self.sleep()
#         self.local.expectTest('There\'s no audio stream with the name qweqweqwe', 'There should be no audio stream with that name.')
#         
#     def test_50_list_audiostreams(self):
#         self.client.sendline("a -l")
#         self.sleep()
#         self.local.expectTest('audio', 'The list of audio streams cannot be obained.')
#     
#     def test_98_delete_audiostream(self):
#         self.client.sendline("a -e audio")
#         self.sleep()
#         self.local.expectTest('Audio stream audio deleted.', 'Audio stream audio cannot be deleted.')
# 
#     def test_99_delete_invalid_audiostream(self):
#         self.client.sendline("a -e audiostream")
#         self.sleep()
#         self.local.expectTest('There\'s no audio stream with the name audiostream', 'There should be no audio stream with that name.')
#         
# class DISABLED_Test_3_Videostream(testing.TelnetBaseTest):
#     """
#     System Tests for Videostream
#     """
#     def test_00_default_prompt(self):
#         self.local.expectTest('pof: ', 'The default prompt is not appearing.')
# 
#     def test_01_add_videostream(self):
#         self.sleep()
#         self.client.sendline("v -a videostream")
#         self.sleep()
#         self.local.expectTest('Video stream videostream created.', 'Video stream video cannot be created.')
# 
#     def test_02_rename_videostream(self):
#         self.client.sendline("v -m video videostream")
#         self.sleep()
#         self.local.expectTest('Video stream videostream rename to video.', 'Video stream videostream cannot be renamed to video.')
# 
#     def test_03_change_container(self):
#         self.client.sendline("v -t mpegts video")
#         self.sleep()
#         self.local.expectTest('container of video stream video is set to mpegts.', 'Container of video stream video was not set to mpegts.')
# 
#     def test_04_change_codec(self):
#         self.client.sendline("v -c vorbis video")
#         self.sleep()
#         self.local.expectTest('codec of video stream video is set to vorbis.', 'Codec of video stream video was not set to vorbis.')
# 
#     def test_05_change_settings(self):
#         self.client.sendline("v -s value1:key1,value2:key2 video")
#         self.sleep()
#         self.local.expectTest('codec_settings of video stream video is set to value1:key1,value2:key2.', 'Codec settings of video stream video was not set to new values.')
# 
#     def test_06_change_width(self):
#         self.client.sendline("v -w 640 video")
#         self.sleep()
#         self.local.expectTest('width of video stream video is set to 640.', 'Bitdepth of video stream video was not set to 24.')
# 
#     def test_07_change_height(self):
#         self.client.sendline("v -r 480 video")
#         self.sleep()
#         self.local.expectTest('height of video stream video is set to 480.', 'Sample rate of video stream video was not set to 44100.')
# 
#     def test_09_change_port(self):
#         self.client.sendline("v -p 5050 video")
#         self.sleep()
#         self.local.expectTest('port of video stream video is set to 5050.', 'IP Port of video stream video was not set.')
# 
#     def test_10_change_buffer(self):
#         self.client.sendline("v -b 100 video")
#         self.sleep()
#         self.local.expectTest('buffer of video stream video is set to 100.', 'Buffer time of video stream video was not set to 44100.')
# 
#     def test_11_change_input(self):
#         self.client.sendline("v -i v4l2src:location=/dev/video0 video")
#         self.sleep()
#         self.local.expectTest('source of video stream video is set to v4l2src:location="/dev/video0".', 'Unable to specify video input.')
#     
# #    def test_12_change_codec_of_invalid_stream(self):
# #        self.client.sendline("v -c vorbis videostream")
# #        self.sleep()
# #        self.local.expectTest('There\'s no video stream with the name videostream', 'IP Port of video stream video was not set.')
# 
#     # NOTE: -z flag is not for changing description !
#     #def test_13_change_description(self):
#     #    self.client.sendline("v -z 'A Description' video")
#     #    self.sleep()
#     #    self.local.expectTest('source of video stream video is set to jackvideosrc.', 'Unable to specify video input.')
#     
#     def test_14_set_samplerate_to_invalid_value(self):
#         self.client.sendline("v -r Juliette video")
#         self.sleep()
#         self.local.expectTest('option -r: invalid integer value: u\'Juliette\'', 'Invalid value accepted as sample rate')
#         
#     def test_50_list_videostreams(self):
#         self.client.sendline("v -l")
#         self.sleep()
#         self.local.expectTest('video', 'The list of video streams cannot be obained.')
#     
#     def test_98_delete_videostream(self):
#         self.client.sendline("v -e video")
#         self.sleep()
#         self.local.expectTest('Video stream video deleted.', 'Video stream video cannot be deleted.')
# 
#     def test_99_delete_invalid_videostream(self):
#         self.client.sendline("v -e videostream")
#         self.sleep()
#         self.local.expectTest('There\'s no video stream with the name videostream', 'There should be no video stream with that name.')
#         
# class DISABLED_Test_004_Settings(testing.TelnetBaseTest):
#     """
#     System Tests for presets
#     """
#     def tst(self, command, expected, errorMsg = None):
#         self.client.sendline(command)
#         self.sleep()
#         err = errorMsg or 'The command did not return: "%s" as expected' % expected
#         self.local.expectTest(expected, err)
#         
#     def test_00_yes(self):
#         self.local.expectTest('pof: ', 'The default prompt is not appearing.')
#     
#     def test_01_GlobalPresets(self):
#         """
#         test that preset file exist.
#         look for a known preset
#         check that you can't delete a preset
#         
#         Assign a preset to a contact
#         """
#         
#         # os.path.expanduser("~")+"/.miville/sropulpof.preset"
#         path = "/usr/share/sropulpof/globalSettings.presets"
#         self.assertTrue(os.path.exists(path), 'Global Preset file "%s" is missing' % path)
# 
#     def test_02_MediaPresets(self):    
#         """
#         test that preset file exist.
#         look for a known preset
#         check that you can't delete a preset
#         """
#         path = "/usr/share/sropulpof/mediaSettings.presets"
#         self.assertTrue(os.path.exists(path), 'Global Preset file "%s" is missing' % path)
#         
#         
#     def test_02_addSettings(self):
#         
#         self.tst('list settings', 'There are 5 preset settings', 'The preset files are missing.')
 
class Test_5_Devices(unittest.TestCase):
    """
    System Tests for devices.

    devices -k video -l
    devices -k video -t v4l2 -d /dev/video0 -a
    devices -k video -t v4l2 -d /dev/video0 -m norm pal
    devices -k video -t v4l2 -d /dev/video0 -a
    devices -k video -t v4l2 -d /dev/video0 -m norm ntsc
    devices -k video -t v4l2 -d /dev/video0 -a
    """
    ######################################
    def setUp(self):
        global local
        local.unittest = self
        self.local = local
        self.client = local.telnet_process
    def test_99_kill_miville(self):
        self.local.kill_miville_and_telnet()
    #######################################
    def test_01_list(self):
        self.client.sendline("devices -k video -l")

    def DISABLED_test_01_list_v4l2_devices(self):
        self.client.sendline("devices -k video -l")
        #self.local.expectTest('/dev/video0.', 'Warning: no v4l2 device appearing in the CLI.')

    def DISABLED_test_02_modify_v4l2_attribute(self):
        self.client.sendline("devices -k video -t v4l2 -d /dev/video0 -a")
        #self.local.expectTest('norm', 'v4l2 device doesn\'t have a norm attribute.')
       
        self.client.sendline("devices -k video -t v4l2 -d /dev/video0 -m norm pal")
        self.client.sendline("devices -k video -t v4l2 -d /dev/video0 -a")
        #self.local.expectTest('PAL', 'Could not modify device attribute.')
       
        self.client.sendline("devices -k video -t v4l2 -d /dev/video0 -m norm ntsc")
        self.client.sendline("devices -k video -t v4l2 -d /dev/video0 -a")
        #self.local.expectTest('NTSC', 'Could not modify device attribute.')

