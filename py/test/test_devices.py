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
app.go()

class Test_devices_v4l2(unittest.TestCase):
    """
    Integration tests for v4l2 devices.
    """
    def test_01_video0(self):
        app.go()
        app.api.devices_list(app.me, 'video')
        value = app.last.value
        if not isinstance(value, list):
            self.fail('Last notify should give a list.')
        elif len(value) == 0:
            self.fail("There are no v4l2 devices on this computer.")
        elif not isinstance(value[0], devices.Device):
            self.fail('Last notify should give a Device instance.')
    
    def test_02_attributes(self):
        app.go()
        app.api.device_modify_attribute(app.me, 'video', 'v4l2', '/dev/video0', 'norm', 'pal')
        app.go()
        
        app.api.device_list_attributes(app.me, 'video', 'v4l2', '/dev/video0')
        value = app.last.value
        has_norm = False
        if not isinstance(value, dict):
            self.fail("Error trying to list device attribute. There might be no v4l2 device.")
        # TODO: will change for dict !!
        try:
            if not value['norm'].get_value() == "pal":
                self.fail("Failed to change attribute's value.")
        except IndexError:
            self.fail("Device has no norm value.")
        # TODO
        app.api.device_modify_attribute(app.me, 'video', 'v4l2', '/dev/video0', 'norm', 'ntsc')
        app.go()
        
    def test_03_width_height(self):
        app.go()
        v4l2 = devices.get_manager('video').drivers['v4l2']
        
        try:
            video0 = v4l2.devices['/dev/video0']
        except KeyError, e:
            self.fail('no device /dev/video0')
        else:
            attr_w = v4l2.devices['/dev/video0'].attributes['width']
            attr_h = v4l2.devices['/dev/video0'].attributes['height']
            for w, h in ((320, 240), (1024, 768), (640, 480)):
                attr_w.set_value(w)
                attr_h.set_value(h)
                v4l2.poll_now()
                app.go()
                val_w, val_h = attr_w.get_value(), attr_h.get_value()
                if val_w != w:
                    self.fail('Width should has been changed to %d but is %d.' % (w, val_w))
                elif val_h != h:
                    self.fail('Height should has been changed to %d but is %d.' % (h, val_h))
                else:
                    pass
class Test_devices_jackd(unittest.TestCase):
    """
    Integration tests for jackd devices.
    """
    def test_01_devices(self):
        app.go()
        app.api.devices_list(app.me, 'audio')
        value = app.last.value
        if not isinstance(value, list):
            self.fail('Last notify should give a list.')
        elif len(value) == 0:
            self.fail("There are no jackd running on this computer.")
        elif not isinstance(value[0], devices.Device):
            self.fail('Last notify should give a Device instance.')
    
    def test_02_attributes(self):
        app.go()
        app.api.device_list_attributes(app.me, 'audio', 'jackd', 'default')
        value = app.last.value
        has_attr = False
        if not isinstance(value, dict):
            self.fail("Error trying to list device attribute. There might be no jackd running.")
        try:
            x = value['rate'].get_value()
        except IndexError:
            self.fail("Device has no rate attribute.")

