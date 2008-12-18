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

from twisted.trial import unittest

import devices
import utils.log

del devices.log
devices.log = utils.log.start('error', 1, 0, 'devices')

class Test_1_Driver(unittest.TestCase):
    """
    Tests for the Driver base class.
    """
    def test_1_driver(self):
         # simple test
        class TestAudioDriver(devices.Driver):
            def start(self):
                self.shell_command_start(['ls','-la'])
            def list(self):
                return []
            def get(self):
                return None
            def shell_command_result(self,command,results):
                print "results are :\n",results
        
        # any exception will make the test fail.
        d = TestAudioDriver()
        
    def test_2_devices(self):
        class TestAudioDev(devices.Device):
            pass
            
        d = TestAudioDev()
        d.addAttribute('sampling rate', devices.IntAttribute(44100,48000, 8000,192000))
        d.addAttribute('bit depth', devices.IntAttribute(16,16, 8,24))
        
        tmp = d.getAttribute('sampling rate').getValue()
        self.assertEqual(tmp, 44100,'Attribute not matching what we gave it.')
        
        tmp = d.getAttribute('bit depth').getValue()
        self.assertEqual(tmp, 16,'Attribute not matching what we gave it.')
        
        tmp = d.getAttribute('bit depth').getDefault()
        self.assertEqual(tmp, 16,'Default not matching what we gave it.')
        
        tmp = d.getAttribute('sampling rate').getRange() # returns two-tuple
        self.assertEqual(tmp[0], 8000,  'Minimum value not matching what we gave it.')
        self.assertEqual(tmp[1], 192000,'Maximum value not matching what we gave it.')
        
