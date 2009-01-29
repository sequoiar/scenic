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

------------

Usage for test purposes::

    $ cd py/
    $ export PYTHONPATH=$PWD
    $ python devices/devices.py

To run unit tests:: 

    $ trial test/test_devices.py
    
"""


#TODO: retrieve former implementation of the shellprocessprotocol and put it in utils/shell

#TODO: you want to test that a failing condition occurs? use self.assertFailure

# System imports
import os, sys 

# Twisted imports
from twisted.internet import reactor #, protocol
from twisted.internet.error import AlreadyCalled # for when we cancel twice an event.
# App imports
from utils import log, commands
from errors import DeviceError

log = log.start('debug', 1, 0, 'devices')

class Driver(object): #shell.ShellCommander):
    """
    Base class for a driver for a type of Device. (ALSA, JACK, v4l, dv1394, etc.)
    
    Drivers must inherit from this class and implement each method.
    Singleton : There should be only one instance of each Driver child classes.
    
    Methods and attributes that must be overriden in child classes are:
        - name : A class attribute string for the name of the driver (example: 'v4l2')
        - on_attribute_change(...) : method called when the programmer changes the value of a device attribute
        - on_devices_polling(...) : method called when the Driver wants to poll its devices.
        - on_commands_results(...) : method called once commands started from on_devices_polling or on_attribute_change are done.
        - on_commands_error(...) : method similar to the method above.
    
    The programmer can also override:
        - prepare(...) : method called once at startup
    
    Also, the few direct subclasses (such as VideoDriver) of Driver must override:
        - kind : A class attribute string for the type of the driver (example: 'video')
        
    TODO: implement the MVC pattern. 
    
    The driver objects will eventually call the notify method if the ControllerApi instance.
    This will be used instead of the current callbacks.
    That means there should be a start function in each Driver module. 
        argument : the api object.
        
    This would return the Driver instance for each of those Driver modules.
        
    TODO: separate Driver and ShellCommander classes
        
    
    If a getter/setter is necessary, the corresponding attribute should be private.
    i.e. begin with an underscore.
    """
    name = 'default_name'
    
    def __init__(self, polling_interval=15.0, polling_enabled=True):
        """
        child classes __init__ methods must call this one if defined.
        
        Example : Driver.__init__(self)
        TODO: remove 2 attributes.
        """
        self.devices = {} # dict
        self._new_devices = {} # used to populate the list of devices.
        self.polling_interval = polling_interval
        self.state_poll_enabled = polling_enabled
        self._delayed_id = None
        self.kind = 'default_kind'
        self.api = None
       
    def __del__(self):
        self.stop_polling()
    
    def __str__(self):
        return self.name
        
    def _add_new_device(self, device):#, in_new_devices=False):
        """
        Add a Device to this _new_devices.
        Also sets the Device Driver to this one.
        """
        self._new_devices[device.name] = device
        device.driver = self
     
    def _on_done_devices_polling(self, caller=None, event_key=None):
        """
        Compares former dict of devices with the new one
        and notifies the listener if changes occurred.
        Also sends it the whole list of devices for this driver.
        
        old_devices is the former dict of self.devices. 
        
        calls the API/Core callbacks:
           'on_devices_removed'     arg: dict of Device instances
           'on_devices_added'       arg: dict of Device instances
           'on_attributes_changed'  arg: dict of Attribute instances
           'on_devices_list'        arg: dict of Device instances
        """
        # Copying self._new_devices self.to devices
        old_devices = self.devices
        self.devices = self._new_devices
        #print "self.devices: %s" % (str(self.devices))
        
        # check for deleted
        removed = {}
        for name in old_devices:
            if name not in self.devices:
                removed[name] = old_devices[name]
                
        # check for added devices
        added = {}
        for name in self.devices:
            if name not in old_devices:
                added[name] = self.devices[name]
        
        # check for attribute changes
        attr_changed = {}
        for dev_name in self.devices:
            # if devices is in both new and former dict:
            if dev_name not in added and dev_name not in removed:
                dev = self.devices[dev_name]
                for attr_name in dev.attributes:
                    attr = dev.attributes[attr_name]
                    try:
                        old_attr = old_devices[attr_name]
                    except KeyError:
                        pass
                    else:
                        if attr.get_value() != old_attr.get_value():
                            attr_changed[attr.name] = attr
        # let us call the callbacks
        if len(added) > 0:
            self._call_event_listener('devices_added', added, caller) #dict
        if len(removed) > 0:
            self._call_event_listener('devices_removed', removed, caller) # dict
        if len(attr_changed) > 0:
            self._call_event_listener('device_attributes_changed', attr_changed.values(), caller) # dict
        # print "calling on_devices_list"
        #print "calling", self._call_event_listener, 'on_devices_list'
        #print "DONE"
        if event_key == 'devices_list':
            self._call_event_listener('devices_list', self.devices, caller) # dict !!!!
        #TODO: maybe not call it every time.:wq
        
      
    def _call_event_listener(self, event_key, argument, caller=None):
        """
        Calls an event listener (with one argument)
        
        See register_event_listener
        Might throw KeyError if event name doesn't exist.
        TODO: change for the api
        
        events names are:
            'devices_removed'     arg: list of Device instances
            'devices_added'       arg: list of Device instances
            'device_attributes_changed'  arg: list of Attribute instances
            'devices_list'        arg: list of Device instances
        
        Will call observer.<Subject>.notify(caller, value, key=None) with args caller=self, value=a tuple, key='some string'
        """
        self.api.notify(caller, argument, event_key) # driver instance, dict, event key name
        
    def prepare(self):
        """
        Starts the use of the Driver. 
        
        Called only once on system startup.
        Should be implemented in child classes. The prepare mothod in 
        child classes should then call this one. Like this:
        Device.prepare(self)
        """
        if self.state_poll_enabled:
            #return self._poll_devices()
            # changed to avoid calling a twisted start_single_command before reactor is started.
            self._delayed_id = reactor.callLater(self.polling_interval, self._poll_devices)
    
    def on_attribute_change(self, attribute, caller=None, event_key=None):
        """
        Called by a device when one of its attribute is changed. (by the core, usually)
        
        Must be overriden if necessary to do shell scripts or so
        """
        return self.poll_now(caller, event_key) # TODO
    
    def set_polling_interval(self, ms): # TODO: remove
        """
        Interval between polling devices.
        
        :param ms: milliseconds.
        """
        self.polling_interval = ms
        #if self.state_poll_enabled:
        #    self.poll_now()
    
    def get_polling_interval(self): # TODO: remove
        return self.polling_interval
    
    def start_polling(self, caller=None, event_key=None):
        """
        Returns a Deferred
        """
        self.state_poll_enabled = True
        return self.poll_now(caller, event_key)
        
    def stop_polling(self):
        self._cancel_delayed_polling()
        self.state_poll_enabled = False
        
    #def get_polling_is_enabled(self):
    #    return self.state_poll_enabled
    
    def _cancel_delayed_polling(self):
        if self._delayed_id is not None: 
            # TODO : seems to cause an AttributeError
            # we should fix this instead of silently catching it
            try:
                self._delayed_id.cancel()
            except AttributeError:
                pass
            except AlreadyCalled:
                pass
            self._delayed_id = None
            
    def poll_now(self, caller=None, event_key=None): 
        """
        Called when the programmer wants to refresh 
        the attributes for a device right now.
        
        Will call all the callbacks with the correct values.
        """
        return self._poll_devices(caller, event_key)
        
    def _poll_devices(self, caller=None, event_key=None): 
        """
        Devices polling routine.
        """
        if self.state_poll_enabled:
            self._cancel_delayed_polling()
            self._delayed_id = reactor.callLater(self.polling_interval, self._poll_devices) # no caller, no event_key
        self._new_devices = {}
        return self._on_devices_polling(caller, event_key)
    
    def _on_devices_polling(self, caller=None, event_key=None): 
        """
        This is where the Driver actually calls many commands 
        in order to populate its Device list and their attributes.
        
        Must be overriden in child classes.
        Once it is done, should call self._on_done_devices_polling(old_devices) with the 
        former list of devices the new dict is populated.
        
        IMPORTANT: Should return a Deffered instance.
        """
        raise NotImplementedError('this method must be overriden in child classes.')
        #pass
        # default behaviour :
        #self._on_done_devices_polling({})
    
# Drivers should extend one of these classes: 
class VideoDriver(Driver):
    """
    """
    pass

class AudioDriver(Driver):
    """
    """
    pass

class DataDriver(Driver):
    """
    """
    pass
    

class Attribute(object):
    """
    Base class for an attribute of a device.
    """
    kind = 'default kind of attribute'
    
    def __init__(self, name, value=None, default=None):
        self.device = None
        self.name = name
        self._value = value # TODO: make private. 
        self.default = default
    
    def __str__(self):
        return "%s (%s)" % (str(self._value), str(type(self)))

    def get_value(self):
        """
        TODO: asynchronous. 
        arg: callback, called with the whole 
        attribute as argument.
        """
        return self._value

    def set_value(self, value, caller=None, event_key=None):#do_notify_driver=True):
        """ 
        Changes the value of the Attribute for a device.
        
        Notifies the Driver if do_notify_driver is not False.
        TODO: Should return a Deferred object
        """
        # TODO: return a Deferred object.
        self._value = value
        #if do_notify_driver:
        return self._on_change(caller, event_key)
        #else:
        #    return None
    
    def _on_change(self, caller=None, event_key=None):
        """
        Called when the value changed.

        Tells the Device that the value changed.
        """ 
        return self.device.driver.on_attribute_change(self, caller, event_key)
        
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
    # TODO: range should be a tuple
    def __init__(self, name, value=0, default=0, minimum=0, maximum=1023):
        Attribute.__init__(self, name, value, default)
        self.range = (minimum, maximum)
       
class OptionsAttribute(Attribute):
    """
    The options is a list (not a tuple) of possible options. 
    The value and default are indices. (int)
    
    A tuple is better since it is immutable. Indices are integers.
    
    If you want to get the index of a value, use attr.options.index(value)
    """
    kind = 'options'
    
    def __init__(self, name, value=None, default=None, options=[]):
        """
        In this case, value is stored as a string.
        
        Options is a list of strings. Indices are integers, of course.
        """
        Attribute.__init__(self, name, value, default)
        self.options = options
        
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
        self.human_readable_name = name  # can be overriden by the driver.
        
    def add_attribute(self, attribute):
        """
        Adds an attribute to a device.
        
        Calls Attribute.set_device()
        
        TODO: should we use a direct acces to the dict instead?
        """
        self.attributes[attribute.name] = attribute
        attribute.device = self
    
    def __str__(self):
        s = ""
        s += "name:"+self.name+"\n"
        if len(self.attributes) == 0:
            s += 'no attributes'
        else:
            for k in self.attributes:
                s += '%s:%s' % (k, self.attributes[k].get_value())
        return s
      
class DriversManager(object):
    """
    Manages all drivers of its kind.
    
    There should be only one instance of this class
    """
    kind = 'default_kind'
    
    def __init__(self):
        self.drivers = dict()
    
    def add_driver(self, driver):
        """
        The key will be the driver's name
        """
        driver.kind = self.kind
        self.drivers[driver.name] = driver

class VideoDriversManager(DriversManager):
    kind = 'video'
    
class AudioDriversManager(DriversManager):
    kind = 'audio'
    
class DataDriversManager(DriversManager):
    kind = 'data'

# drivers managers
managers = {}
managers['video'] = VideoDriversManager()
managers['audio'] = AudioDriversManager()
managers['data']  = DataDriversManager()

def get_all_drivers(driver_kind=None):
    """
    Returns a list of drivers.

    Might throw a KeyError. (if driver_kind does nbot exist)

    If driver_kind is None, will return all drivers.
    """
    global managers
    if driver_kind is not None:
        return managers[driver_kind].drivers.values()
    else:
        ret = []
        for manager in managers.values():
            for driver in manager.drivers.values():
                ret.append(driver)
        return ret

def get_all_devices(driver_kind=None, driver_name=None):
    """
    Returns a list.
    
    """
    drivers = get_all_drivers(driver_kind)
    ret = []
    for driver in drivers:
        if driver_name is not None:
            if driver.name == driver_name:
                ret.append(driver)
        else:
            ret.append(driver)
    return ret

def get_manager(manager_kind):
    """
    Returns a manager for a kind of driver.
    
    Might throws a KeyError
    """
    global managers
    return managers[manager_kind]

# each Driver module should do something like this: 
# devices.get_manager('video').add_driver(Video4LinuxDriver())

def list_attributes(caller, driver_kind, driver_name, device_name):
    """
    Lists a device's attributes
    
    Called from the API.device_list_attributes.
    Might raise a DeviceError
    
    :param driver_kind: 'video', 'audio' or 'data'
    :param driver_name: 'alsa', 'v4l2'
    :param device_name: '/dev/video0', 'hw:1'
    """
    global managers
    try: 
        manager = managers[driver_kind]
    except KeyError:
        raise DeviceError('No such kind of driver: %s' % (driver_kind))
    try:
        driver = manager.drivers[driver_name]
    except KeyError:
        raise DeviceError('No such driver name: %s' % (driver_name))
    try:
        device = driver.devices[device_name]
    except KeyError:
        raise DeviceError('No such device: %s' % (device_name))
    return device.attributes


def modify_attribute(caller, driver_kind, driver_name, device_name, attribute_name, value):
    """
    Modifies a device's attribute
    
    
    Might raise a DeviceError
    :param driver_kind: 'video', 'audio' or 'data'
    :param driver_name: 'alsa', 'v4l2'
    :param device_name: '/dev/video0', 'hw:1'
    :param attribute_name:
    """
    global managers
    
    try: 
        manager = managers[driver_kind]
    except KeyError:
        raise DeviceError('No such kind of driver: %s' % (driver_kind))
    try:
        driver = manager.drivers[driver_name]
    except KeyError:
        raise DeviceError('No such driver name: %s' % (driver_name))
    try:
        device = driver.devices[device_name]
    except KeyError:
        raise DeviceError('No such device: %s' % (device_name))
    try:
        attribute = device.attributes[attribute_name]
    except KeyError:
        raise DeviceError('No such attribute: %s' % (attribute_name))
    attribute.set_value(value, caller, 'device_modify_attribute') #'device_attributes_changed') # TODO : I think that key is not correct.
    #self.notify(caller, attribute, 'device_attributes_changed') # TODO: make asynchronous
    driver.poll_now(caller, 'device_modify_attribute')
    # TODO: attribute_changed should be triggered
    
    #self.notify(caller, 'No such attributes for driver/device: %s %s %s:%s' % (driver_name, device_name, attribute_name, str(value)), 'info')
    #self.notify(caller, 'you called devices_modify_attributes', 'devices_modify_attribute')
    #self.notify(caller, 
    # devices.get_driver(driver_name).devices[device_name].attributes[attribute_name].set_value(value)

