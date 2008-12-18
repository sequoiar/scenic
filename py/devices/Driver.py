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

if __name__ == '__main__':
    # simple test
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
    
