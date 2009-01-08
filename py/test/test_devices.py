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
import pprint

from twisted.trial import unittest

import devices
#from utils.observer import Subject,Observer
import utils.log
del devices.log
devices.log = utils.log.start('error', 1, 0, 'devices')

# ---------------------------- Dummy implementation classes -----------
class DummyAudioDriver(devices.Driver):
    """
    We need to subclass the Driver class in order to test it.
    """
    def __init__(self,test_case):
        self.test_case = test_case
        devices.Driver.__init__(self,'dummy')
        
    def shell_command_result(self,command,results,callback=None):
        """
        Our simple test prints "ham"
        """
        self.test_case.failUnlessSubstring("ham", results, 'Shell command not giving what is expected.')

# -------------------------- tests ------------------------------------
class Test_1_Driver(unittest.TestCase):
    """
    Tests for the Driver base class.
    """
    def test_1_driver_instanciation(self):
        # any exception will make the test fail.
        d = DummyAudioDriver(self)
        d.prepareDriver()
    
    def test_2_simple_shell_command(self):
        d = DummyAudioDriver(self)
        d.shell_command_start(['echo','ham'])
        time.sleep(0.1) # 100 ms

class Test_2_Device_Attributes(unittest.TestCase):
    """
    Test attributes for devices 
    """
    def test_1_device_int_attributes(self):
        d = devices.Device('MOTU')
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
    
    def test_2_device_string_attribute(self):
        d = devices.Device('MOTU')
        d.addAttribute(devices.StringAttribute('meal','egg','spam'))
        
        tmp = d.getAttribute('meal').getValue()
        self.assertEqual(tmp, 'egg','Attribute not matching what we gave it.')
        
        tmp = d.getAttribute('meal').getDefault()
        self.assertEqual(tmp, 'spam','Attribute default not matching what we gave it.')
    
    def test_3_device_options_attribute(self):
        d = devices.Device('MOTU')
        d.addAttribute(devices.OptionsAttribute('meal',0, 1,['egg','spam','ham','bacon']))
        
        attr = d.getAttribute('meal')
        
        # value
        tmp = attr.getValue()
        self.assertEqual(tmp, 0,'Attribute not matching what we gave it: '+str(tmp))
        
        tmp = attr.getValueForIndex(attr.getValue())
        self.assertEqual(tmp, 'egg','Attribute not matching what we gave it.')
        
        # default 
        tmp = attr.getDefault()
        self.assertEqual(tmp, 1,'Attribute default not matching what we gave it.')
        
        tmp = attr.getValueForIndex(attr.getDefault())
        self.assertEqual(tmp, 'spam','Attribute not matching what we gave it.')
        
    def test_4_list_attributes(self):
        d = devices.Device('MOTU')
        d.addAttribute(devices.StringAttribute('meal','egg', 'spam'))
        
        tmp = d.getAttributes().keys()
        self.assertEqual(tmp, ['meal'],'Attribute names not matching what we gave it.')
        
        

# --------------------------------- no good ---------------------------
# no good.
class Test_3_Drivers_Managers(unittest.TestCase):
    """
    Warning: This test suite adds dummy drivers and devices to the 
    "devices" module "managers" attribute.
    """ 
    def setUp(self):
        # drivers
        dummyDriver = DummyAudioDriver(self) # passing it the TestCase
        devices.managers['audio'].addDriver(dummyDriver)
        
        # devices
        dummyDevice = devices.Device('MOTU')
        spamDevice = devices.Device('SPAM')
        dummyDriver.addDevice(dummyDevice)
        dummyDriver.addDevice(spamDevice)
        
    def test_1_list_drivers(self):
        for kind in ('video','audio','data'):
            tmp = devices.managers[kind].getDrivers()
            self.assertEqual(type(tmp), dict,'Should be a dict.')
    
    def onListDevices(self, devices):
        self.assertEqual(type(devices), dict,'Should be a dict.')
        self.assertEqual(devices['MOTU'].getName(), 'MOTU','Wrong device name.')
        self.assertEqual(devices['SPAM'].getName(), 'SPAM','Wrong device name.')
        
    def test_2_list_devices(self):
        dummyDriver = devices.managers['audio'].getDriver('dummy')
        dummyDriver.listDevices(self.onListDevices) # passing a callback
        time.sleep(0.1) # 100 ms
    
class Test_4_v4l_Driver(unittest.TestCase):
    """
    test Video4LinuxDriver
    """
    def test_0_list_drivers(self):
        drivers = devices.managers['video'].getDrivers()
        
        # DEBUG INFO:
        #print "\nDRIVERS MANAGERS:"
        #pprint.pprint(devices.managers)
        #print "VIDEO DRIVERS:"
        #pprint.pprint(drivers)
        
        self.assertEqual(type(drivers), dict, 'Should be a dict.')
        
        SHOULD_HAVE = 1 # number of video drivers there should be. (only v4l implemented for now)
        self.assertEqual(len(drivers), SHOULD_HAVE, 'Dict should have %d driver but has %d.' % (SHOULD_HAVE,len(drivers)))
    
    def onListDevices_1(self, devices):
        self.assertEqual(type(devices), dict,'Should be a dict.')
    
    def test_1_list_devices(self):
        v4l = devices.managers['video'].getDriver('v4l')
        v4l.listDevices(self.onListDevices_1)
        time.sleep(0.1) # 100 ms
    
    def onListDevices_2(self, devices):
        try:
            tmp = devices['/dev/video0']
        except KeyError:
            self.fail('There is no /dev/video0. (might be OK)')
        
    def test_2_computer_has_a_dev_video0(self):
        driver = devices.managers['video'].getDriver('v4l')
        driver.listDevices(self.onListDevices_2)
        time.sleep(0.1) # 100 ms
    
    def onListDevices_3(self, devices):
        try:
            device = devices['/dev/video0']
        except KeyError:
            self.fail('There is no /dev/video0. (might be OK)')
            return
        
        #print '\n/dev/video0 attributes:'
        #device.printAllAttributes()
        #print "\nINPUT:"
        tmp = device.getAttribute('video standard').getValue()
        
    
    def test_3_video0_attributes(self):
        driver = devices.managers['video'].getDriver('v4l')
        driver.listDevices(self.onListDevices_3)
        time.sleep(0.1) # 100 ms
        
    
