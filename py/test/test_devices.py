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
from twisted.internet import reactor

import devices
import utils.log

from utils import commands

del devices.log
devices.log = utils.log.start('error', 1, 0, 'devices')

#del devices..log
#devices.shell.log = utils.log.start('error', 1, 0, 'commands')


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
        devices.Driver.prepare(self)
    
    def _on_devices_polling(self):
        self.devices = {}
        dev = devices.Device('M-Audio Delta 1010 LT')
        self._add_new_device(dev)
        for name in ['egg','spam']:
            dev.add_attribute(devices.StringAttribute(name, 'bacon', 'bacon'))
        self.devices = self._new_devices 

    def on_commands_error(self, commands):
        raise NotImplementedError('This method must be implemented in child classes. (if you need it)')
        
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
        d.stop_polling()
   
class Test_2_Device_Attributes(unittest.TestCase):
    """
    Test attributes for devices 
    """
    def test_1_device_int_attributes(self):
        d = devices.Device('MOTU')
        d.add_attribute(devices.IntAttribute('sampling rate', 44100, 48000, 8000, 192000))
        d.add_attribute(devices.IntAttribute('bit depth', 16, 16, 8, 24))
        
        tmp = d.attributes['sampling rate'].get_value()
        self.assertEqual(tmp, 44100, 'Attribute not matching what we gave it.')
        
        tmp = d.attributes['bit depth'].get_value()
        self.assertEqual(tmp, 16, 'Attribute not matching what we gave it.')
        
        tmp = d.attributes['bit depth'].default
        self.assertEqual(tmp, 16, 'Default not matching what we gave it.')
        
        tmp = d.attributes['sampling rate'].range # a two int tuple
        self.assertEqual(tmp[0], 8000,  'Minimum value not matching what we gave it.')
        self.assertEqual(tmp[1], 192000, 'Maximum value not matching what we gave it.')
    
    def test_2_device_string_attribute(self):
        d = devices.Device('MOTU')
        d.add_attribute(devices.StringAttribute('meal', 'egg', 'spam'))
        
        tmp = d.attributes['meal'].get_value()
        self.assertEqual(tmp, 'egg', 'Attribute not matching what we gave it.')
        
        tmp = d.attributes['meal'].default
        self.assertEqual(tmp, 'spam', 'Attribute default not matching what we gave it.')
    
    def test_3_device_options_attribute(self):
        d = devices.Device('MOTU')
        d.add_attribute(devices.OptionsAttribute('meal', 'egg', 'spam', ['egg', 'spam', 'ham', 'bacon']))
        
        attr = d.attributes['meal']
        #self.assertEqual(tmp, 'egg','Attribute not matching what we gave it: '+attr)
        
        # value
        value = attr.get_value()
        self.assertEqual(value, 'egg', 'Attribute value not matching what we gave it: '+str(value))
        
        i = attr.options.index(attr.get_value())
        self.assertEqual(i, 0, 'Attribute value index incorrect.')
        
        # default 
        tmp = attr.default
        self.assertEqual(tmp, 'spam','Attribute default not matching what we gave it.'+tmp)
        
        tmp = attr.options.index(attr.default)
        self.assertEqual(tmp, 1, 'Attribute default index incorrect.')
        
    def test_4_list_attributes(self):
        d = devices.Device('MOTU')
        d.add_attribute(devices.StringAttribute('meal', 'egg', 'spam'))
        d.add_attribute(devices.IntAttribute('ham', 0, 0))
        
        tmp = d.attributes.keys()
        self.assertEqual(tmp, ['meal', 'ham'], 'Attribute names not matching what we gave it.')

# --------------------------------- no good ---------------------------
# no good.
class Test_3_Drivers_Managers(unittest.TestCase):
    """
    Warning: This test suite adds dummy drivers and devices to the 
    "devices" module "managers" attribute.
    """ 
    def test_1_get_manager(self):
        # test for the get_manager() function.
        for kind in ('video','audio','data'):
            tmp = devices.managers[kind].drivers
            self.assertEqual(type(tmp), dict, 'Should be a dict.')
        try:
            devices.managers['spam']
            self.fail('there should be no drivers manager with that name.')
        except KeyError:
            pass
    
    def test_2_list_drivers(self):
        for kind in ('video','audio','data'):
            tmp = devices.managers[kind].drivers
            self.assertEqual(type(tmp), dict, 'Should be a dict.')
        
class DummyListener(object):
    """
    Similar to the way the Driver objects are used in the core API.
    """
    def __init__(self, test):
        self.test = test
        self.has_been_called = False
    
    def on_devices_added(self, devices):
        pass
        
    def on_devices_removed(self, devices):
        pass
        
    def on_attributes_changed(self, attributes):
        pass
    
    def on_devices_list(self, devices):
        VERBOSE = False
        if VERBOSE:    
            print "\nv4l: REFRESHED DEVICES"
        if len(devices) == 0:
            print "\non_devices_list(): NO V4L2 DEVICE."
            pass
        else:
            for name in devices:
                if VERBOSE:
                    print "V4L2 device : ", name
                device = devices[name]
                #print "List %s : device %s" % (driver.get_name(), device.get_name())
        self.has_been_called = True
        #print "SET has_been_called to ", self.has_been_called
    	#print "\nOK DONE "
    	
    def on_devices_list_check(self, devices):
    	VERBOSE = False
    	#VERBOSE = False
    	if VERBOSE:
    	    pass
            #print "\non_devices_list_check(): v4l2 devices and their attributes : "
    	for device in devices.values():
    		driver_name = device.driver.name
    		device_name = device.name
    		#print ">> device %s:%s" % (driver_name, device_name) 
    		for attr in device.attributes.values():
    			name = attr.name
    			value = attr.get_value()
    			default = attr.default
                #if VERBOSE:
                #    print "%s:%s: %s = %s (%s)" % (driver_name, device_name, name, value, default)
                #    if name in ['width','height']:
                #        if not isinstance(value, int):
                #            print "Attribute %s should be %s but is %s" % (name, 'int', value)
                #    if name in ['driver', 'norm', 'input']:
                #        if not isinstance(value, str):
                #            print "Attribute %s should be %s but is %s" % (name, 'str', value)
        
class DummyWrapperAPI(object):
    """
    fakely implements the MVC pattern
    mimics the Subject class
    """
    def __init__(self, test):
        self.listener = DummyListener(test)
        self.callbacks = {
            'devices_added':self.listener.on_devices_added,
            'devices_removed':self.listener.on_devices_removed,
            'device_attributes_changed':self.listener.on_attributes_changed,
            'devices_list':self.listener.on_devices_list
        }
    def notify(self, caller, value, key=None):
        #print "notify(%s,%s,%s)" % (str(caller), str(value), str(key))
        self.callbacks[key](value)
    
    def use_tester_check(self):
        """Enables using on_devices_list_check instead of on_devices_list"""
        self.callbacks['devices_list'] = self.listener.on_devices_list_check


def v4l_change_norm_1(results, test):
    driver = test.driver
    
    try:
        video0 = test.driver.devices['/dev/video0']
    except KeyError, e:
            test.fail('no device /dev/video0: '+str(e.message))
    else:
        
        attr = test.driver.devices['/dev/video0'].attributes['norm']
        
        #attr.set_value('Composite0')
        #for expected in ['Composite0', 'Composite1', 'S-Video']:
        #    attr.set_value(expected)
        #    time.sleep(0.1)
        #    self.driver.poll_now()
        #    time.sleep(0.1)
        #    value = attr.get_value()
        #    if value != expected:
        #        self.fail('norm attributes should be changed to %s but is %s.' % (expected, value))
    


class Test_4_v4l_Driver(unittest.TestCase):
    """
    test Video4LinuxDriver
    
    The golden rule is: If your tests call a function which returns a Deferred, your test should return a Deferred.
    """
    def setUp(self):
        self.api = DummyWrapperAPI(self)
        devices.v4l.start(self.api)
        
        self.driver = devices.managers['video'].drivers['v4l2']
        if not isinstance(self.driver, devices.VideoDriver):
            self.fail('%s should be a VideoDriver instance.' % (self.driver))
        
    def test_1_get_driver(self):
        video_drivers = devices.managers['video'].drivers
        #pprint.pprint(video_drivers)
        if 'v4l2' not in video_drivers:
            self.fail('v4l2 should be in video drivers.')
        
    def test_2_list_devices(self):
        return self.driver.prepare() # _poll_devices()  
        #time.sleep(0.1)        
        
    def test_3_devices_attributes(self):
        # override one callback
        self.api.use_tester_check()
        self.driver.poll_now() # which calls Driver._poll_devices()
        time.sleep(0.1)
        
    def test_4_set_norm(self):
        return self.driver.poll_now().addCallback(v4l_change_norm_1, self)
    
    def xxtest_5_width_height(self):
        self.driver.poll_now() # which calls Driver._poll_devices()
        
        time.sleep(0.1)
        try:
            video0 = self.driver.devices['/dev/video0']
        except KeyError, e:
            #self.driver.stop_polling()
            print "ERROR",e
            self.fail('no device /dev/video0')
        else:
            attr_w = self.driver.devices['/dev/video0'].attributes['width']
            attr_h = self.driver.devices['/dev/video0'].attributes['height']
            
            for w, h in ((320, 240), (1024, 768), (640, 480)):
                
                attr_w.set_value(w)
                attr_h.set_value(h)
                time.sleep(0.1)
                self.driver.poll_now()
                val_w, val_h = attr_w.get_value(), attr_h.get_value()
                if val_w != w:
                    self.fail('Width should has been changed to %d but is %d.' % (w, val_w))
                elif val_h != h:
                    self.fail('Height should has been changed to %d but is %d.' % (h, val_h))
                else:
                    pass
                    #print 'Success ! Width is %d and height is %d.' % (w, h)
        
    def tearDown(self):
        self.driver.stop_polling()

