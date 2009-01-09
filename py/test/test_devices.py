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
from twisted.python import failure

import devices
import utils.log

del devices.log
devices.log = utils.log.start('error', 1, 0, 'devices')
del devices.shell.log
devices.shell.log = utils.log.start('error', 1, 0, 'shell')

# TODO: was simpler
    #def shell_command_result(self,command,results,callback=None):
    #    """
    #    Our simple test prints "ham"
    #    """
    #    self.test_case.failUnlessSubstring("ham", results, 'Shell command not giving what is expected.')

# ---------------------------- Dummy implementation classes -----------
class DummyAudioDriver(devices.AudioDriver):
    """
    We need to subclass the Driver class in order to test it.
    
    Methods and attributes that must be overriden in child classes are:
    - name : A class attribute string for the name of the driver (example: 'v4l2')
    - on_attribute_change : method called when the programmer changes the value of a device attribute
    - on_devices_polling : method called when the Driver wants to poll its devices.
    - on_commands_results : method called once commands started from on_devices_polling or on_attribute_change are done.
    - on_commands_error : method similar to the method above.
    """
    name = 'dummy audio driver'
    
    def __init__(self, polling_interval, test_case):
        self.test_case = test_case
        devices.Driver.__init__(self, polling_interval)
        
    def on_attribute_change(self, attribute):
        pass
    
    def prepare(self):
        pass
    
    def on_devices_polling(self):
        pass # TODO: test this
        
    def on_commands_error(self, commands):
        raise NotImplementedError, 'This method must be implemented in child classes. (if you need it)'
        
    def on_commands_results(self, results, commands):
        """
        Our simple test prints "ham"
        """
        for i in range(len(results)):
            result = results[i]
            success, results_infos = result
            if isinstance(results_infos, failure.Failure):
                print "failure ::: ", results_infos.getErrorMessage()
            else:
                command = commands[i]
                stdout, stderr, signal_or_code = results_infos
                self.test_case.failUnlessSubstring("ham", stdout, 'Shell command not giving what is expected.')
    
# -------------------------- tests ------------------------------------
class Test_1_Driver(unittest.TestCase):
    """
    Tests for the Driver base class.
    """
    def test_1_driver_instanciation(self):
        # any exception will make the test fail.
        d = DummyAudioDriver(0.1, self)
        d.prepare()
        time.sleep(0.2)
        d.stop_polling()
    
    def test_2_stop(self):
        d = DummyAudioDriver(0.1, self)
        d.prepare()
        d.stop_polling()
    
    def test_3_simple_shell_command(self):
        d = DummyAudioDriver(0.1, self)
        d.single_command_start(['echo','ham'])
        time.sleep(0.2) # 100 ms
        d.stop_polling()
    # TODO : test if polling works.
    
class Test_2_Device_Attributes(unittest.TestCase):
    """
    Test attributes for devices 
    """
    def test_1_device_int_attributes(self):
        d = devices.Device('MOTU')
        d.add_attribute(devices.IntAttribute('sampling rate', 44100, 48000, 8000, 192000))
        d.add_attribute(devices.IntAttribute('bit depth', 16, 16, 8, 24))
        
        tmp = d.get_attribute('sampling rate').get_value()
        self.assertEqual(tmp, 44100, 'Attribute not matching what we gave it.')
        
        tmp = d.get_attribute('bit depth').get_value()
        self.assertEqual(tmp, 16, 'Attribute not matching what we gave it.')
        
        tmp = d.get_attribute('bit depth').get_default()
        self.assertEqual(tmp, 16,'Default not matching what we gave it.')
        
        tmp = d.get_attribute('sampling rate').get_range() # returns a two int tuple
        self.assertEqual(tmp[0], 8000,  'Minimum value not matching what we gave it.')
        self.assertEqual(tmp[1], 192000,'Maximum value not matching what we gave it.')
    
    def test_2_device_string_attribute(self):
        d = devices.Device('MOTU')
        d.add_attribute(devices.StringAttribute('meal', 'egg', 'spam'))
        
        tmp = d.get_attribute('meal').get_value()
        self.assertEqual(tmp, 'egg','Attribute not matching what we gave it.')
        
        tmp = d.get_attribute('meal').get_default()
        self.assertEqual(tmp, 'spam','Attribute default not matching what we gave it.')
    
    def test_3_device_options_attribute(self):
        d = devices.Device('MOTU')
        d.add_attribute(devices.OptionsAttribute('meal','egg', 'spam', ['egg','spam','ham','bacon']))
        
        attr = d.get_attribute('meal')
        #self.assertEqual(tmp, 'egg','Attribute not matching what we gave it: '+attr)
        
        # value
        value = attr.get_value()
        self.assertEqual(value, 'egg','Attribute value not matching what we gave it: '+str(value))
        
        i = attr.index_of(attr.get_value())
        self.assertEqual(i, 0,'Attribute value index incorrect.')
        
        # default 
        tmp = attr.get_default()
        self.assertEqual(tmp, 'spam','Attribute default not matching what we gave it.'+tmp)
        
        tmp = attr.index_of(attr.get_default())
        self.assertEqual(tmp, 1,'Attribute default index incorrect.')
        
    def test_4_list_attributes(self):
        d = devices.Device('MOTU')
        d.add_attribute(devices.StringAttribute('meal','egg', 'spam'))
        d.add_attribute(devices.IntAttribute('ham', 0, 0))
        
        tmp = d.get_attributes().keys()
        self.assertEqual(tmp, ['meal','ham'],'Attribute names not matching what we gave it.')

# --------------------------------- no good ---------------------------
# no good.
class disabled_Test_3_Drivers_Managers(unittest.TestCase):
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
        
    def xxtest_1_list_drivers(self):
        for kind in ('video','audio','data'):
            tmp = devices.managers[kind].getDrivers()
            self.assertEqual(type(tmp), dict,'Should be a dict.')
    
    def xxonListDevices(self, devices):
        self.assertEqual(type(devices), dict,'Should be a dict.')
        self.assertEqual(devices['MOTU'].get_name(), 'MOTU','Wrong device name.')
        self.assertEqual(devices['SPAM'].get_name(), 'SPAM','Wrong device name.')
        
    def xxtest_2_list_devices(self):
        dummyDriver = devices.managers['audio'].getDriver('dummy')
        dummyDriver.listDevices(self.onListDevices) # passing a callback
        time.sleep(0.1) # 100 ms
    
class disabled_Test_4_v4l_Driver(unittest.TestCase):
    """
    test Video4LinuxDriver
    """
    def xxtest_0_list_drivers(self):
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
    
    def xxtest_1_list_devices(self):
        v4l = devices.managers['video'].getDriver('v4l')
        v4l.listDevices(self.onListDevices_1)
        time.sleep(0.1) # 100 ms
    
    def onListDevices_2(self, devices):
        try:
            tmp = devices['/dev/video0']
        except KeyError,e:
            self.fail('There is no /dev/video0. (might be OK) '+str(e.message))
        
    def xxtest_2_computer_has_a_dev_video0(self):
        driver = devices.managers['video'].getDriver('v4l')
        driver.listDevices(self.onListDevices_2)
        time.sleep(0.1) # 100 ms
    
    def onListDevices_3(self, devices):
        try:
            device = devices['/dev/video0']
        except KeyError,e:
            self.fail('There is no /dev/video0. (might be OK) '+str(e.message))
            return
        #print '\n/dev/video0 attributes:'
        #device.printAllAttributes()
        #print "\nINPUT:"
        tmp = device.get_attribute('video standard').getValue()
        
    def xxtest_3_video0_attributes(self):
        driver = devices.managers['video'].getDriver('v4l')
        driver.listDevices(self.onListDevices_3)
        time.sleep(0.1) # 100 ms
        
    def xxtest_4_prepare_driver(self):
        driver = devices.managers['video'].get_driver('v4l')
        driver.prepareDriver()
        
