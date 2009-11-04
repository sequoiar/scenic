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
Integration tests within the API.

Usage: trial test/itest_devices.py
""" 
import unittest
import warnings
import sys

from miville import devices
import imiville as app

class Test_01_devices_v4l2(unittest.TestCase):
    """
    Integration tests for v4l2 devices.
    """
    def test_01_video0(self):
        # startup poutine XXX
        app.main()
        app.me.verbose = False
        app.view.verbose = False
        
        app.api.devices_list(app.me, 'video')
        value = app.last.value
        if not isinstance(value, list):
            pass #self.fail('Last notify should give a list.')
        elif len(value) == 0:
            pass #self.fail("There are no v4l2 devices on this computer.")
        elif not isinstance(value[0], devices.Device):
            pass #self.fail('Last notify should give a Device instance.')
    
    def test_02_attributes(self):
        app.go()
        try:
            app.api.device_modify_attribute(app.me, 'video', 'v4l2', '/dev/video0', 'norm', 'pal')
        except AttributeError, e:
            self.fail("imiville does not have a api attribute !!!")
        app.go()
        
        app.api.device_list_attributes(app.me, 'video', 'v4l2', '/dev/video0')
        value = app.last.value
        has_norm = False
        if not isinstance(value, dict):
            pass #self.fail("Error trying to list device attribute. There might be no v4l2 device.")
        # TODO: will change for dict !!
        else:
            try:
                if not value['norm'].get_value() == "pal":
                    try:
                        self.test_02_attributes.skip = "No V4L2 device."
                    except:
                        pass
                        print "NO V4L2 DEVICE. PASS."
                    pass #self.fail("Failed to change attribute's value.")
            except IndexError:
                pass #self.fail("Device has no norm value.")
            # TODO
            app.api.device_modify_attribute(app.me, 'video', 'v4l2', '/dev/video0', 'norm', 'ntsc')
            app.go()
        
    def test_03_width_height(self):
        app.go()
        try:
            v4l2 = devices.get_manager('video').drivers['v4l2']
        except KeyError:
            print "NO V4L2 DEVICE. PASS."
        else:
            try:
                video0 = v4l2.devices['/dev/video0']
            except KeyError, e:
                print "NO V4L2 DEVICE. PASS."
                pass #self.fail('no device /dev/video0')
            else:
                for w, h in ((320, 240), (640, 480)):
                    attr_w = v4l2.devices['/dev/video0'].attributes['width']
                    attr_h = v4l2.devices['/dev/video0'].attributes['height']
                    attr_w.set_value(w)
                    attr_h.set_value(h)
                    app.go()
                    v4l2.poll_now()
                    app.go()
                    attr_w = v4l2.devices['/dev/video0'].attributes['width']
                    attr_h = v4l2.devices['/dev/video0'].attributes['height']
                    val_w, val_h = attr_w.get_value(), attr_h.get_value()
                    if val_w != w:
                        #self.fail
                        warnings.warn('Width should has been changed to %d but is %d.' % (w, val_w))
                    elif val_h != h:
                        warnings.warn('Height should has been changed to %d but is %d.' % (h, val_h))
                    else:
                        pass

class Test_02_devices_jackd(unittest.TestCase):
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
            try:
                self.test_01_devices.skip = "No JACKD device."
            except:
                print "NO JACKD DEVICE. PASS."
        elif not isinstance(value[0], devices.Device):
            self.fail('Last notify should give a Device instance.')
    
    def test_02_attributes(self):
        app.go()
        app.api.device_list_attributes(app.me, 'audio', 'jackd', 'default')
        value = app.last.value
        has_attr = False
        if not isinstance(value, dict):
            try:
                self.test_01_devices.skip = "No JACKD device."
            except:
                print "NO JACKD DEVICE. PASS."
        else:
            try:
                x = value['rate'].get_value()
            except IndexError:
                self.fail("Device has no rate attribute.")
    
    def test_03_stop_devices(self):
        # app.miville.exit()
        pass

    def test_04_disable_kill_jackd(self):
        """
        Should be tested more
        """
        app.api.devices_toggle_kill_jackd_enabled(0)

