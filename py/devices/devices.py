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
"""


# System imports
import os,sys #, resource, signal, time, sys, select

#print sys.path

# Twisted imports
from twisted.internet import reactor, protocol
from twisted.python import procutils # Utilities for dealing with processes

# App imports
from utils import log

log = log.start('debug', 1, 0, 'devices')

class Driver:
    """
    Base class for a driver for a type of Device.
    
    Drivers must inherit from this class and implement each method.
    """
        
    def start(self):
        """
        Starts the use of the Driver. 
        
        Called only once on system startup.
        There should be a "started" class variable for each driver in 
        order to make sure it is started only once.
        """
        raise NotImplementedError, 'This method must be implemented in child classes.'
    
    def list(self): #,callback):
        """
        Lists name of devices of the type that a driver supports on this machine right now.
        """
        # TODO: Will call the provided callback with list of Device objects ?
        raise NotImplementedError, 'This method must be implemented in child classes.'
    
    def get(self,device_name=None):
        """
        Returns a device object.
        
        device_name must be a ASCII string.
        Returns None in case of device doesn't exist ?
        """
        raise NotImplementedError, 'This method must be implemented in child classes.'
    
    def shell_command_start(self, command):
        """
        Command is a list of strings.
        """
        executable = procutils.which(command[0])[0]
        #args = command[1:]
        if executable:
            try:
                log.info('Starting command: %s' % command[0])
                self.process = reactor.spawnProcess(ShellProcessProtocol(self,command), executable, command, os.environ, usePTY=True)
            except:
                log.critical('Cannot start the device polling/control command: %s' % executable)
        else:
            log.critical('Cannot find the shell command: %s' % command[0])
            
    def shell_command_result(self, command, text_results):
        """
        Called from child process.
        
        Args are: the command that it the results if from, text data resulting from it.
        """
        raise NotImplementedError, 'This method must be implemented in child classes. (if you need it)'
        
class ShellProcessProtocol(protocol.ProcessProtocol):
    """
    Protocol for doing asynchronous call to a shell command.
    
    Calls its caller back with the result.
    """
    def __init__(self, server,command):
        self.server = server
        self.command = command
        
    def connectionMade(self):
        log.info('Device polling/configuring command started: %s' % (self.command[0]))
    
    def outReceived(self, data):
        log.info('Result from command %s : %s' % (self.command[0],data))
        self.server.shell_command_result(self.command,data)
           
    def processEnded(self, status):
        log.info('Device poll/config command ended. Message: %s' % status)

class Attribute:
    """
    Base class for an attribute of a device.
    
    TODO: Should we store the name of the attribute as well?
    """
    def __init__(self,value=None,default=None):
        self.value = value
        self.default = default
        
    def getValue(self):
        return self.value

    def setValue(self,val):
        self.value = val
        
    def getDefault(self):
        return self.default

    def setDefault(self,val):
        self.default = val
        
    def getType():
        """
        Returns the type() of self.
        """
        return type(self)
        
class BooleanAttribute(Attribute):
    def __init__(self,value=False,default=False):
        Attribute.__init__(self,value,default)

class StringAttribute(Attribute):
    def __init__(self,value='',default=''):
        Attribute.__init__(self,value,default)

class IntAttribute(Attribute):
    """
    The range is a two-numbers tuple defining minimum and maximum possible value.
    """
    def __init__(self,value=0,default=0,minVal=0,maxVal=1023):
        Attribute.__init__(self,value,default)
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
    The options is a list of possible options. 
    The value is the index.
    """
    def __init__(self,value=None,default=None,options=[]):
        Attribute.__init__(self,value,default)
        self.setOptions(options)
    
    def getValueByIndex(self,k):
        """
        Returns the actual value by its index in the list.
        """
        ret = None
        try:
            self.options[i]
        except IndexError:
            pass
        return ret
    
    def getIndexForValue(self,val):
        """
        Returns an index for that value, or None if not found.
        """
        ret = None
        try:
            ret = self.option.index(val)
        except ValueError:
            pass
        return ret
    
    def getIndex(self):
        """
        Returns current value's index.
        """
        return self.value
        
    def setIndex(self,i):
        """
        Sets current value's index.
        """
        self.value = i
    
    def getValue(self):
        """
        Overrides the parent's getValue method.
        """
        return self.getValueByIndex(self.value)
        
    def setValue(self,val):
        """
        Overrides the parent's setValue method.
        """
        self.value = self.getIndexForValue(val)
    
    def getOptions(self):
        return self.options
    
    def setOptions(self,optsList):
        """
        Argument must be a list
        """
        self.options = optsList #copy.deepcopy(optsList)

class Device:
    """
    Base class for any Device.
    """
    def getAttribute(self,k):
        """
        Gets one Attribute object.
        
        Returns None if attribute doesn't exist.
        """
        ret = None
        try:
            ret = self.__dict__[k]
        except KeyError:
            pass
            raise KeyError, 'That Device doesn\'t have that attribute !'
        return ret
    
    def getAttributeNames(self):
        """
        Gets list of all attributes names
        """
        return self.__dict__.keys()
    
    def getAllAttributes(self):
        """
        Returns a dict in the form name = Attribute
        """ 
        return self.__dict__
    
    def addAttribute(self,k,val):
        self.__dict__[k] = val #copy.deepcopy(val)
    
    def printAllAttributes(self):
        """
        for debugging purposes
        """
        for k,attr in self.__dict__.items():
            print "%40s = %30s" % (k, str(attr.getValue()))

if __name__ == '__main__':
    # tests
    class TestAudioDev(Device):
        pass
            
    print "----------------------------"
    print "starting test"
    d = TestAudioDev()
    d.addAttribute('sampling rate', IntAttribute(44100,48000, 8000,192000))
    d.addAttribute('bit depth', IntAttribute(16,16, 8,24))
    print "DEVICE INFO:"
    d.printAllAttributes()
    print "DONE testing the Device class."
    
    print "----------------------------"
    print "NOW TESTING the Driver class:"
    class TestAudioDriver(Driver):
        def start(self):
            self.shell_command_start(['ls','-la'])
        def list(self):
            return []
        def get(self):
            return None
        def shell_command_result(self,command,results):
            print "results from command %s are :%s" % (command[0], results)
    
    print "TEST main()"
    d = TestAudioDriver().start()
    
    reactor.run()
