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

TODO: Improve errors, exceptions and failures handling. 
"""

import os
import pprint
import sys
import traceback

from twisted.internet import reactor, defer # protocol,
from twisted.python import procutils, failure
from twisted.internet import utils

# App imports
from utils import log

log = log.start('debug', 1, 0, 'shell')

class CommandNotFoundException(Exception):
    """
    raised when a shell command is not found
    """
    pass

class ShellCommander(object):
    """
    Handles one or many shell processes asynchronously.
    
    Classes that need to start many shell commands need to extend this one.
    """
    def find_command(self, command_name, error_msg=None):
        """
        used to check if command is available on this system.
        
        Throws a CommandNotFoundException if not.
        """
        try:
            executable = procutils.which(command_name)[0] # gets the executable
        except IndexError:
            if error_msg is None:
                error_msg = 'Could not find command %s' % (command_name)
            raise CommandNotFoundException, error_msg
        return executable

    def _command_start(self, executable, command):
        """
        Actually starts only one command.
        Returns a Deferred object
        """
        #deferred = defer.fail(failure.Failure(failure.DefaultException('Could not find command %s'% (command[0]))))
        # if try was successful
        try:
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
            # TODO: handle better
            log.critical('Cannot start the command %s. Reason: %s' % (executable,str(e.message)))
        return deferred
        
    def commands_start(self, commands, extra_arg=None):
        """
        Starts a shell command.
        
        commands is a tuple of tuple of strings.
        callback is a callback to call once done.
        calls on_commands_results which calls the callback when done.
        """
        deferreds = []
        ok = True
        for command in commands:
            try:
                executable = self.find_command(command[0]) # gets the executable
            except CommandNotFoundException, e:
                raise CommandNotFoundException,'Cannot find the shell command: %s. Reason: %s' % (command[0], e.message)
                break
        if ok:
            for command in commands:
                deferreds.append(self._command_start(executable, command))
            defer.DeferredList(deferreds).addCallback(self.on_commands_results, commands, extra_arg).addErrback(self.on_commands_error, commands, extra_arg)
        
    def on_commands_error(self, command_failure, commands, extra_arg=None):
        print "@@@@@@@@@"
        print ">>>> ERROR:"
        pprint.pprint({'failure':command_failure, 'exception':sys.exc_info(), 'commands':commands, 'extra_arg':extra_arg})
        traceback.print_tb(sys.exc_info()[2])
        print '>>>> Exception message : %s' % (command_failure.value.message)
        print "@@@@@@@@@"
        #raise NotImplementedError, 'This method must be implemented in child classes. (if you need it)'
        
    def on_commands_results(self, results, commands, extra_arg=None): 
        """
        Called once a bunch of child processes are done.
        
        @param success_results a tuple of (boolean,str) tuples
        @param commands is a list of the provided commands
        @param callback is a callback to call once done.
        
        Args are: the command that it the results if from, text data resulting from it, callback to call once done.
        callback should be called with the same arguments. command, results, callback (but results can be something else than text)
        
        You must extend this class and override this method. Copy-paste this one to make your life easier.
        """
        #pprint.pprint(results) # (out, err, code)
        for i in range(len(results)):
            result = results[i]
            success, results_infos = result
            command = commands[i]
            print ">>>>>>>>>>>>>>>>  command : %s   <<<<<<<<<<<<<<<" % (command)
            
            if isinstance(results_infos,failure.Failure):
                print ">>>> FAILURE : ",results_infos.getErrorMessage()  # if there is an error, the programmer should fix it.
                # TODO : handle failure better
            else:
                #pprint.pprint(result)
                stdout, stderr, signal_or_code = results_infos
                print ">>>> stdout: %s" % (stdout)
                print ">>>> stderr: %s" % (stderr)
                
                if success:
                    print ">>>> Success !"
                    print ">>>> code is ", signal_or_code
                else:
                    print ">>>> Failure !"
                    print ">>>> signal is ", signal_or_code
        #raise NotImplementedError, 'This method must be implemented in child classes. (if you need it)'
        
    def single_command_start(self, command, extra_arg=None):
        """
        Starts a single shell command.
        
        command is a list of strings.
        callback is a callback to call with the command and the result once done.
        """
        try:
            executable = self.find_command(command[0]) # gets the executable
        except Exception:
            raise Exception,'Cannot find the shell command: %s' % (command[0])
        deferreds = [self._command_start(executable, command)] # list with a single deferred
        defer.DeferredList(deferreds).addCallback(self.on_commands_results, [command])
        
if __name__ == '__main__':
    print "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
    print "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
    d = ShellCommander()
    try:
        d.find_command('asdasd')
    except CommandNotFoundException:
        print "1) Successfully found that command does not exist."
    commands = [
            ['ls','-l'],
            ['echo','egg'],
            ['ls','-a']
        ]
    try:
        d.commands_start(commands)
    except CommandNotFoundException,e:
        print 'Raised error while it should not : %s', e.message
    reactor.callLater(0.1, reactor.stop)
    reactor.run()
    
