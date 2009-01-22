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
Integration tests within the API.

Usage: trial test/itest_devices.py
""" 
import unittest
import sys
import devices

sys.path.append('..')
import imiville

imiville.main()

class Test_devices_v4l2(unittest.TestCase):
    """
    Integration tests for v4l2 devices.
    """
    def test_01_attributes(self):
        imiville.api.devices_list(imiville.me, 'video')
        value = imiville.last.value
        if not isinstance(value, list):
            self.fail('Last notify should give a list.')
        if not isinstance(value[0], devices.Device):
            self.fail('Last notify should give a Device instance.')
        
        #api.device_list_attributes(me, 'video', 'v4l2', '/dev/video0')
        #api.device_modify_attribute(me, 'video', 'v4l2', '/dev/video0', 'norm', 'ntsc')
        #go()


