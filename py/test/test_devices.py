#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technologiques (SAT)
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

import time

from twisted.trial import unittest

import devices

import utils.log
del devices.log
devices.log = utils.log.start('error', 1, 0, 'devices')



class TestAudioDriver(devices.Driver):
    def __init__(self,case):
        self.case = case
    def start(self):
        pass
    def list(self):
        return []
    def get(self):
        return None
    def shell_command_result(self,command,results):
        self.case.failUnlessSubstring("toto", results, 'Shell command not giving what is expected.')

class TestAudioDev(devices.Device):
    pass

class Test_1_Driver(unittest.TestCase):
    """
    Tests for the Driver base class.
    """
    def test_1_driver(self):
        # any exception will make the test fail.
        d = TestAudioDriver(self)
        d.start()
    
    def test_2_simple_shell_command(self):
        d = TestAudioDriver(self)
        d.shell_command_start(['echo','toto'])
        time.sleep(0.1)

class Test_2_Device(unittest.TestCase):
    def test_1_device_attributes(self):
        d = TestAudioDev()
        d.addAttribute(devices.IntAttribute('sampling rate',44100,48000, 8000,192000))
        d.addAttribute(devices.IntAttribute('bit depth',16,16, 8,24))
        
        tmp = d.getAttribute('sampling rate').getValue()
        self.assertEqual(tmp, 44100,'Attribute not matching what we gave it.')
        
        tmp = d.getAttribute('bit depth').getValue()
        self.assertEqual(tmp, 16,'Attribute not matching what we gave it.')
        
        tmp = d.getAttribute('bit depth').getDefault()
        self.assertEqual(tmp, 16,'Default not matching what we gave it.')
        
        tmp = d.getAttribute('sampling rate').getRange() # returns a two int tuple
        self.assertEqual(tmp[0], 8000,  'Minimum value not matching what we gave it.')
        self.assertEqual(tmp[1], 192000,'Maximum value not matching what we gave it.')

class Test_3_v4l_Driver(unittest.TestCase):
    def test_1_list(self):
        d = devices.Video4LinuxDriver()
        l = d.list()
        self.assertEqual(type(l), list,'Driver doesn\'t return a list of devices.')
    
    def test_2_computer_has_a_dev_video0(self):
        d = devices.Video4LinuxDriver()
        l = d.list()
        name = None
        try:
            name = l[0]
        except IndexError:
            pass
        self.assertEqual(name, '/dev/video0','Computer doesn\'t have a /dev/video0 v4l device. (it is probably correct)')
    
