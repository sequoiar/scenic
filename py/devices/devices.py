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

TODO: change attribute camel case for small caps underscores...
"""

# System imports
import os,sys #, resource, signal, time, sys, select

#print sys.path

# Twisted imports
from twisted.internet import reactor, protocol
from twisted.python import procutils # Utilities for dealing with (shell) processes

# App imports
from utils import log
#from utils import singleton
#from utils.observer import Observer, Subject

log = log.start('debug', 1, 0, 'devices')

class Driver: #, Subject):
    """
    Base class for a driver for a type of Device. (ALSA, JACK, v4l, dv1394, etc.)
    
    Drivers must inherit from this class and implement each method.
    Singleton : There should be only one instance of each Driver child classes.
    """
    def __init__(self,name=None):
        self.devices = {} # dict
        self.name = name
        
    def __str__(self):
        return self.name
        
    def addDevice(self,device):
        self.devices[device.getName()] = device
    
    def getName(self):
        """
        Used by the DriverManager instances to manage Driver instances by their name.
        """
        return self.name
        
    def prepareDriver(self):
        """
        Starts the use of the Driver. 
        
        Called only once on system startup.
        """
        pass
        
    def listDevices(self, callback):
        """
        Lists name of devices of the type that a driver supports on this machine right now.

        Calls the callback with a dict of devices.
        Data will be a 'key' => 'long name' dict.
        Can be overriden in child classes.
        """
        callback(self.devices)
    
    def getDevice(self,device_name=None):
        """
        Returns a device object.
        
        device_name must be a ASCII string.
        Throws a KeyError if device doesn't exist.
        """
        return self.devices[device_name]
        
    def onDeviceAttributeChange(self,attribute):
        """
        Called by a device when one of its attribute is changed. (by the core, usually)
        
        Can be overriden if necessaty to do shell scripts or so
        """
        pass
        
    def shell_command_start(self, command, callback=None):
        """
        Starts a shell command.
        
        command is a list of strings.
        callback is a callback to call with the command and the result once done.
        """
        executable = procutils.which(command[0])[0] # gets the executable
        #args = command[1:]
        if executable:
            try:    
                if False: # not verbose
                    log.info('Starting command: %s' % command[0])
                self.process = reactor.spawnProcess(ShellProcessProtocol(self,command,callback), executable, command, os.environ, usePTY=True)
            except:
                log.critical('Cannot start the device polling/control command: %s' % executable)
        else:
            log.critical('Cannot find the shell command: %s' % command[0])
            
    def shell_command_result(self, command, text_results, callback=None):
        """
        Called from child process.
        
        Args are: the command that it the results if from, text data resulting from it, callback to call once done.
        callback should be called with the same arguments. command, results, callback (but results can be something else than text)
        """
        raise NotImplementedError, 'This method must be implemented in child classes. (if you need it)'

# Drivers should extend one of these classes: 
class VideoDriver(Driver):
    pass
class AudioDriver(Driver):
    pass
class DataDriver(Driver):
    pass

class ShellProcessProtocol(protocol.ProcessProtocol):
    """
    Protocol for doing asynchronous call to a shell command.
    
    Calls its caller back with the result.
    """
    def __init__(self, server,command,callback=None,verbose=False):
        """
        * server is a Driver instance.
        * command is a list of strings.
        * callback is a callback to call once done.
        * verbose is a boolean
        """
        self.server = server # a Driver instance
        self.command = command
        self.verbose = verbose
        self.callback = callback
        
    def connectionMade(self):
        if self.verbose:
            log.info('Shell command called: %s' % (str(self.command)))
    
    def outReceived(self, data):
        if self.verbose:
            log.info('Result from command %s : %s' % (self.command[0],data))
        self.server.shell_command_result(self.command, data, self.callback)
        
    def processEnded(self, status):
        if self.verbose:
            log.info('Device poll/config command ended. Message: %s' % status)

class Attribute:
    """
    Base class for an attribute of a device.
    
    TODO: Should we store the name of the attribute as well?
    """
    def __init__(self,name,value=None,default=None):
        self.name = name
        self.value = value
        self.default = default
        self.device = None
        
    def getValue(self):
        return self.value

    def setValue(self,val):
        """
        Changes the value of the Attribute for device.
        
        Calls onChange() once its value is changed.
        """
        self.value = val
        self.onChange()
        
    def getDefault(self):
        return self.default

    def setDefault(self,val):
        self.default = val
        
    def getType():
        """
        Returns the type() of self.
        """
        return type(self)
    
    def setDevice(self,dev):
        """
        Sets the Driver instance it is attached to.
        
        In order to call its onAttributeChange method when the value changes.
        """
        self.device = dev
    
    def getDevice(self):
        return self.device
    
    def onChange(self):
        """
        Called when the value changed.

        Tells the Device that the value changed.
        """ 
        if self.device is not None:
            self.device.onAttributeChange(self)
            
    def getName(self):
        return self.name
        
class BooleanAttribute(Attribute):
    def __init__(self,name,value=False,default=False):
        Attribute.__init__(self,name,value,default)

class StringAttribute(Attribute):
    def __init__(self,name,value='',default=''):
        Attribute.__init__(self,name,value,default)

class IntAttribute(Attribute):
    """
    The range is a two-numbers tuple defining minimum and maximum possible value.
    """
    def __init__(self,name,value=0,default=0,minVal=0,maxVal=1023):
        Attribute.__init__(self,name,value,default)
        self.setRange(minVal,maxVal)
    
    def getRange(self):
        """
        Returns min val, max val
        """
        return self.range[0], self.range[1]
    
    def setRange(self,minimum,maximum):
        self.range = (minimum,maximum)
        
class OptionsAttribute(Attribute):
    """
    The options is a list (or tuple) of possible options. 
    The value and default are indices. (int)
    
    A tuple is better since it is immutable. Indices are integers.
    """
    def __init__(self,name,value=None,default=None,options=[]):
        Attribute.__init__(self,name,value,default)
        self.setOptions(options)
    
    def getValueForIndex(self,k):
        """
        Returns the actual value for an index in the list.
        """
        #ret = None
        #try:
        ret = self.options[k]
        #except IndexError:
        #    pass
        return ret
    
    def getIndexForValue(self,val):
        """
        Returns an index for that value, or None if not found.
        """
        #ret = None
        #try:
        ret = self.option.index(val)
        #except ValueError:
        #    pass
        return ret
    
    #def getIndex(self):
    #    """
    #    Returns current value's index.
    #    """
    #    return self.value
        
    #def setIndex(self,i):
    #    """
    #    Sets current value's index.
    #    """
    #    self.value = i
    #    self.onChange()
    
    
    
    #def getValue(self):
    #    """
    #    Overrides the parent's getValue method.
    #    """
    #    return self.value #self.getValueByIndex(self.value)
        
    #def setValue(self,val):
    #    """
    #    Overrides the parent's setValue method.
    #    """
    #    self.value = self.getIndexForValue(val)
    #    self.onChange()
    
    def getOptions(self):
        return self.options
    
    def setOptions(self,optsList):
        """
        Argument must be a list or tuple.
        """
        self.options = optsList #copy.deepcopy(optsList)

class Device:
    """
    Class for any Device.

    Should not be subclassed, but rather, each object has different attributes using the methods
    defined here to add and set attributes.
    """
    def __init__(self,name=None):
        """
        Argument: The Driver that is used to control this device.
        """
        self.driver = None
        self.attributes = dict()
        self.name = name
    
    def getName(self):
        return self.name
        
    def setDriver(self,driver):
        self.driver = driver
    
    def getAttribute(self,k):
        """
        Gets one Attribute object.
        
        Returns None if attribute doesn't exist.
        """
        ret = None
        try:
            ret = self.attributes[k]
        except KeyError:
            pass
            raise KeyError, 'That Device doesn\'t have that attribute !'
        return ret
    
    def getAttributeNames(self):
        """
        Gets list of all attributes names
        """
        return self.attributes.keys()
    
    def getAllAttributes(self):
        """
        Returns a dict in the form name = Attribute
        """ 
        return self.attributes
    
    def addAttribute(self,attr):
        """
        Adds an attribute to a device.
        TODO: Also registers that attribute to the driver.
        """
        k = attr.getName()
        self.attributes[k] = attr
        attr.setDevice(self)
    
    def printAllAttributes(self):
        """
        for debugging purposes
        """
        for k,attr in self.attributes.items():
            print "%40s = %30s" % (k, str(attr.getValue())) #TODO: if type is option, print actual value.

    def getDriver(self):
        """
        Returns its Driver object.
        """
        return self.driver

    def onAttributeChange(self,attr):
        """
        Called when an attribute has change. 
        
        Calls the Driver notifyChange method.
        """
        if self.driver is not None:
            self.driver.onDeviceAttributeChange(self,attr)

class DriversManager:
    """
    Manages all drivers of its kind.
    
    There should be only one instance of this class
    """
    def __init__(self):
        self.drivers = dict()
    
    def addDriver(self,driver):
        """
        The key will be the driver's name
        """
        self.drivers[driver.getName()] = driver

    def getDrivers(self):
        """
        Returns dict of name -> Driver objects
        """
        return self.drivers # .values()
    
    def getDriver(self,key):
        """
        Might throw KetError
        """
        return self.drivers[key]

class VideoDriversManager(DriversManager):
    pass
class AudioDriversManager(DriversManager):
    pass
class DataDriversManager(DriversManager):
    pass
