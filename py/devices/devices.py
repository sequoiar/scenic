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

"""
Devices handling and Driver base classes.

Usage for test purposes:
$ cd py/
$ export PYTHONPATH=$PWD
$ python devices/devices.py

To run unit tests: 
$ trial test/test_devices.py

TODO: retrieve former implementation of the shelprocessprotocol and put it in utils/shell

TODO: you want to test that a failing condition occurs? use self.assertFailure
"""

# System imports
import os, sys 

# Twisted imports
from twisted.internet import reactor #, protocol

# App imports
from utils import log, shell

log = log.start('debug', 1, 0, 'devices')

class Driver(shell.ShellCommander):
    """
    Base class for a driver for a type of Device. (ALSA, JACK, v4l, dv1394, etc.)
    
    Drivers must inherit from this class and implement each method.
    Singleton : There should be only one instance of each Driver child classes.
    
    Methods and attributes that must be overriden in child classes are:
    - name : A class attribute string for the name of the driver (example: 'v4l2')
    - on_attribute_change : method called when the programmer changes the value of a device attribute
    - on_devices_polling : method called when the Driver wants to poll its devices.
    - on_commands_results : method called once commands started from on_devices_polling or on_attribute_change are done.
    - on_commands_error : method similar to the method above.
    
    The programmer can also override:
    - prepare : method called once at startup
    
    Also, the few direct subclasses (such as VideoDriver) of Driver must override:
    - kind : A class attribute string for the type of the driver (example: 'video')
    """
    kind = 'default_kind'
    name = 'default_name'
    
    def __init__(self, polling_interval=10.0, polling_enabled=True):
        """
        child classes __init__ methods must call this one if defined.
        
        Example : Driver.__init__(self)
        """
        self.devices = {} # dict
        self.selected_device = None
        self.polling_interval = polling_interval
        self.state_poll_enabled = polling_enabled
        self._delayed_id = None
        
        # callbacks
        self.callbacks = {
            'on_device_removed':   None,
            'on_device_added':     None,
            'on_attribute_changed': None
        }
        
        if self.state_poll_enabled:
            self._poll_devices()
        
    def __str__(self):
        return self.name
        
    def add_device(self, device):
        self.devices[device.get_name()] = device
        device.set_driver(self)
    
    def get_kind(self):
        """
        Returns the driver kind 
        Strings such as 'audio','video', 'data'
        """
        return self.kind
    
    def register_event_listener(self, event, callback):
        """
        Sets the callback for an event.
        
        events names are:
            'on_device_removed'     arg: Device instance
            'on_device_added'       arg: Device instance
            'on_attribute_changed'  arg: Attribute instance
            
        Set it to None to turn off the callback.
        """
        self.callbacks[key] = callback
        
    def _call_event_listener(self, event, argument):
        """
        Calls an event listener (with one argument)
        
        See register_event_listener
        Might throw KeyError if event name doesn't exist.
        """
        callback = self.callbacks[event]
        if callback is not None:
            callback(argument)
        
    def get_name(self):
        """
        Used by the DriverManager instances to manage Driver instances by their name.
        """
        return self.name
        
    def prepare(self):
        """
        Starts the use of the Driver. 
        
        Called only once on system startup.
        Should be implemented in child classes.
        """
        pass
        
    def get_devices(self):
        """
        Lists name of devices of the type that a driver supports on this machine right now.

        Calls the callback with a dict of devices.
        Data will be a 'key' => 'long name' dict.
        Can be overriden in child classes.
        """
        return self.devices
    
    def get_device(self, name):
        """
        Returns a device object.
        
        device_name must be a ASCII string.
        Might throw a KeyError if device doesn't exist anymore.
        """
        return  self.devices[name]
        
    def on_attribute_change(self, attribute):
        """
        Called by a device when one of its attribute is changed. (by the core, usually)
        
        Can be overriden if necessary to do shell scripts or so
        """
        self.poll_now()
    
    def set_polling_interval(self, ms):
        """
        Interval between polling devices.
        
        :param ms: milliseconds.
        """
        self.polling_interval = ms
        if self.state_poll_enabled:
            self.poll_now()
    
    def get_polling_interval(self):
        return self.polling_interval
    
    def start_polling(self):
        self.state_poll_enabled = True
        self.poll_now()
        
    def stop_polling(self):
        self._cancel_delayed_polling()
        self.state_poll_enabled = False
        
    def get_polling_is_enabled(self):
        return self.state_poll_enabled
    
    def _cancel_delayed_polling(self):
        if self._delayed_id is not None:
            self._delayed_id.cancel()
        
    def poll_now(self):
        """
        Called when the programmer wants to refresh 
        the attributes for a device right now.
        """
        if self.state_poll_enabled:
            self._cancel_delayed_polling()
        self._poll_devices()
        
    def _poll_devices(self):
        """
        Devices polling routine.
        """
        if self.state_poll_enabled:
            self._delayed_id = reactor.callLater(self.polling_interval, self._poll_devices)
        self.on_devices_polling()
    
    def on_devices_polling(self):
        """
        This is where the Driver actually polls all the devices
        in order to populate their attributes.
        
        Must be overriden in child classes.
        """
        pass
    
# Drivers should extend one of these classes: 
class VideoDriver(Driver):
    kind = 'video'
class AudioDriver(Driver):
    kind = 'audio'
class DataDriver(Driver):
    kind = 'data'
    
class Attribute(object):
    """
    Base class for an attribute of a device.
    """
    kind = 'default'
    
    def __init__(self, name, value=None, default=None):
        self.device = None
        self.name = name
        self.value = value
        self.default = default
        
    def get_value(self):
        """
        TODO: asynchronous. 
        arg: callback, called with the whole 
        attribute as argument.
        """
        return self.value

    def set_value(self, value, do_notify_driver=True):
        """
        Changes the value of the Attribute for a device.
        
        Notifies the Driver if do_notify_driver is not False.
        """
        self.value = value
        if do_notify_driver:
            self._on_change()
        
    def get_default(self):
        return self.default

    def set_default(self, default_value):
        self.default = default_value
        
    def get_kind():
        """
        Returns the kind of attribute it is of self.
        
        A string such as 'int', 'boolean', 'string' or 'options'
        """
        return self.kind
    
    def get_device(self):
        return self.device
    
    def set_device(self, device):
        """
        Called by the Device instance when the programmer adds this attribute to it.
        """
        self.device = device
    
    def _on_change(self):
        """
        Called when the value changed.

        Tells the Device that the value changed.
        """ 
        self.device.getDriver().on_attribute_change(self)
        
    def get_name(self):
        return self.name
        
class BooleanAttribute(Attribute):
    kind = 'boolean'
    def __init__(self, name, value=False, default=False):
        Attribute.__init__(self, name, value, default)

class StringAttribute(Attribute):
    kind = 'string'
    def __init__(self, name, value='', default=''):
        Attribute.__init__(self, name, value, default)

class IntAttribute(Attribute):
    """
    The range is a two-numbers tuple defining minimum and maximum possible value.
    """
    kind = 'int'
    
    def __init__(self, name, value=0, default=0, minimum=0, maximum=1023):
        Attribute.__init__(self, name, value, default)
        self.range = (minimum, maximum)
    
    def get_range(self):
        """
        Returns min val, max val
        """
        return self.range[0], self.range[1]
    
    def set_range(self, minimum, maximum):
        self.range = (minimum, maximum)
        
class OptionsAttribute(Attribute):
    """
    The options is a list (or tuple) of possible options. 
    The value and default are indices. (int)
    
    A tuple is better since it is immutable. Indices are integers.
    """
    kind = 'options'
    
    def __init__(self, name, value=None, default=None, options=[]):
        """
        In this case, value is stored as a string.
        
        Options is a list of strings. Indices are integers, of course.
        """
        Attribute.__init__(self, name, value, default)
        self.options = options
    
    def get_options(self):
        return self.options
    
    def set_options(self, options_list):
        """
        Argument must be a list or tuple.
        """
        self.options = options_list
        # TODO : check if value is in new options list.
        # self.set_value(self.value) 
    
    def index_of(self, value):
        """
        Returns an index for that value.
        Useful for: 
        {{{
        for i in  attr.get_options():
            if i == attr.index_of(attr.get_value()):
                ...
        }}}
        
        Throws ValueError if not in options list.
        """
        return self.options.index(value)
    
    def set_value(self, value):
        """
        Throws KeyError if not in options list.
        """
        if value not in self.options:
            raise KeyError, 'Option %s is not in list' % (value)
        self.value = value
    
class Device(object):
    """
    Class for any Device.

    Should not be subclassed, but rather, each object has different attributes using the methods
    defined here to add and set attributes.
    """
    def __init__(self, name):
        """
        Argument: The Driver that is used to control this device.
        """
        self.driver = None
        self.name = name
        self.attributes = dict()
        self.state_in_use = False
        
    def get_name(self):
        return self.name
        
    def set_driver(self, driver):
        self.driver = driver
    
    def get_attribute(self, name):
        """
        Gets one Attribute object.
        
        Throws a KeyError if Device doesn't have that attribute.
        """
        return self.attributes[name]
        
    def get_attributes(self):
        """
        Returns a dict in the form : string name => Attribute
        """
        return self.attributes
    
    def add_attribute(self, attribute):
        """
        Adds an attribute to a device.
        
        Calls Attribute.set_device()
        """
        self.attributes[attribute.get_name()] = attribute
        attribute.set_device(self)
    
    def __str__(self):
        s = ""
        s += "name:"+self.name+"\n"
        if len(self.attributes) == 0:
            s += 'no attributes'
        else:
            for k in self.attributes:
                s += '%s:%s' % (k, self.attributes[k].get_value())
        return s
    
    def set_in_use(self, state=True):
        self.state_in_use = state
    
    def get_in_use(self):
        return self.state_in_use
    
    def get_driver(self):
        """
        Returns its Driver object.
        """
        return self.driver

class DriversManager(object):
    """
    Manages all drivers of its kind.
    
    There should be only one instance of this class
    """
    def __init__(self):
        self.drivers = dict()
    
    def add_driver(self, driver):
        """
        The key will be the driver's name
        """
        self.drivers[driver.get_name()] = driver

    def get_drivers(self):
        """
        Returns dict of name -> Driver objects
        """
        return self.drivers
    
    def get_driver(self, name):
        """
        Might throw KeyError
        """
        return self.drivers[name]

class VideoDriversManager(DriversManager):
    pass
class AudioDriversManager(DriversManager):
    pass
class DataDriversManager(DriversManager):
    pass
