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
Classes for handling shell process.
"""

import os
import pprint
import sys

from twisted.internet import reactor, protocol, defer
from twisted.python import procutils, failure
from twisted.internet import utils

# App imports
from utils import log

log = log.start('debug', 1, 0, 'shell')

class ShellCommander(object):
    """
    Handles one or many shell processes asynchronously.
    """
    def _command_start(self,command):
        """
        Starts only one command.
        Returns a Deferred object
        """
        deferred = None
        executable = False
        try:
            executable = procutils.which(command[0])[0] # gets the executable
        except IndexError:
            pass
        if executable is False:
            log.critical('Cannot find the shell command: %s' % (command[0])) # log.critical
        
        if True: #executable is not False:
            try:    
                if True: # not verbose
                    log.info('Starting command: %s' % (command[0]))
                #getProcessOutputAndValue(executable, args=(), env={}, path='.', reactor=None)
                #
                #Spawn a process and returns a Deferred that will be called back with
                #its output (from stdout and stderr) and it's exit code as (out, err, code)
                #If a signal is raised, the Deferred will errback with the stdout and
                #stderr up to that point, along with the signal, as (out, err, signalNum)
                args = []
                if len(command) > 1:
                    args = command[1:]
                deferred = utils.getProcessOutputAndValue(executable, args, os.environ)
            except Exception,e:
                #print "ERROR:",sys.exc_info()
                log.critical('Cannot start the command: %s' % (executable))
        else:
            print 'Cannot find the shell command: %s' % (command[0])
        if deferred == None:
            deferred = defer.fail(failure.Failure(failure.DefaultException('Could not find command %s'% (command[0]))))
        return deferred
        
    def commands_start(self, commands, callback=None):
        """
        Starts a shell command.
        
        commands is a tuple of tuple of strings.
        callback is a callback to call once done.
        calls on_commands_results which calls the callback when done.
        """
        deferreds = [self._command_start(command) for command in commands]
        defer.DeferredList(deferreds).addCallback(self.on_commands_results, commands,callback).addErrback(self.on_commands_error,commands,callback)
        
    def on_commands_error(self,commands,callback=None):
        raise NotImplementedError, 'This method must be implemented in child classes. (if you need it)'
        
    def on_commands_results(self, results, commands,callback=None): 
        """
        Called once a bunch of child processes are done.
        
        @param success_results a tuple of (boolean,str) tuples
        @param commands is a list of the provided commands
        @param callback is a callback to call once done.
        
        Args are: the command that it the results if from, text data resulting from it, callback to call once done.
        callback should be called with the same arguments. command, results, callback (but results can be something else than text)
        
        You must extend this class and override this method.
        """
        #pprint.pprint(results) # (out, err, code)
        for i in range(len(results)):
            result = results[i]
            success, results_infos = result
            print "----------------------------------"
            
            if isinstance(results_infos,failure.Failure):
                print "failure ::: ",results_infos.getErrorMessage()  # if there is an error, the programmer should fix it.
            else:
                pprint.pprint(result)
                command = commands[i]
                
                stdout, stderr, signal_or_code = results_infos
                if success:
                    print "success for command %s" % (command[0])
                    print "stdout: %s" % (stdout)
                    print "stderr: %s" % (stdout)
                    print "code is ", signal_or_code
                else:
                    print "failure for command %s" % (command[0])
                    print "stderr: %s" % (stdout)
                    print "signal is ", signal_or_code
        if callback is not None:
            callback() # the programmer can add arguments. 
        raise NotImplementedError, 'This method must be implemented in child classes. (if you need it)'
        
    def single_command_start(self, command, callback=None):
        """
        Starts a single shell command.
        
        command is a list of strings.
        callback is a callback to call with the command and the result once done.
        """
        deferreds = [self._command_start(command)]
        defer.DeferredList(deferreds).addCallback(self.on_commands_results, commands,callback)
        
if __name__ == '__main__':
    def test_callback():
        print "DONE"
    d = ShellCommander()
    commands = [
            ['ls','-l'],
            ['echo','toto'],
            ['ls','-a'],
            ['asdasd'],
            ['v4l2-ctl','qweqweqwe']
        ]
    d.commands_start(commands, test_callback)
    reactor.run()
    
