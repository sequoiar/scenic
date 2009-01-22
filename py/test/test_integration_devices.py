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
# system imports
import unittest
import sys

# app imports
import devices
import imiville as app

# startup poutine
app.main()
app.me.verbose = False
app.view.verbose = False


class Test_devices_v4l2(unittest.TestCase):
    """
    Integration tests for v4l2 devices.
    """
    def test_01_video0(self):
        app.api.devices_list(app.me, 'video')
        value = app.last.value
        if not isinstance(value, list):
            self.fail('Last notify should give a list.')
        if not isinstance(value[0], devices.Device):
            self.fail('Last notify should give a Device instance.')
    
    def test_02_attributes(self):
        app.api.device_modify_attribute(app.me, 'video', 'v4l2', '/dev/video0', 'norm', 'pal')
        app.go()
        
        app.api.device_list_attributes(app.me, 'video', 'v4l2', '/dev/video0')
        value = app.last.value
        has_norm = False
        # TODO: will change for dict !!
        for attribute in value: # list of Attribute instances
            if attribute.name == 'norm':
                has_norm = True
                if attribute.get_value().index("PAL") == -1: # if "PAL" not found
                    self.fail("Attr value has not been changed.")
        if not has_norm:
            self.fail("Attr not found.")
        app.api.device_modify_attribute(app.me, 'video', 'v4l2', '/dev/video0', 'norm', 'ntsc')
        app.go()
        


