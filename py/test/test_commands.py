#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Miville
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Miville is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Miville.  If not, see <http://www.gnu.org/licenses/>.

import time
import pprint

from twisted.trial import unittest
from twisted.python import failure

from miville.utils import commands
import miville.utils.log

del commands.log
commands.log = miville.utils.log.start('error', 1, 0, 'commands')

global test

def on_commands_results(results, commands, extra_arg=None, caller=None):
    global test
    
    VERBOSE = False
    if VERBOSE:
        print 'results', results
    for i in range(len(results)):
        result = results[i]
        success, results_infos = result
        if isinstance(results_infos, failure.Failure):
            print 'FIXME', str(results_infos) #TODO handle failures
            pass # if there is an error, the programmer should fix it.
        else:
            command = commands[i]
            executable = command[0]
            stdout, stderr, signal_or_code = results_infos
            if success:
                if VERBOSE:
                    print '\n------------'
                    print '\ncommand:', command
                    print '\nstdout:', stdout
                    print '\nstderr', stderr
                if executable == 'echo':
                    expected = extra_arg['echo']
                    if stdout.find(expected) == -1:
                        test.fail("echo result not matching what is expected. Should be %s but is %s" % (expected, stdout))
                elif executable == 'ls':
                    expected = extra_arg['ls'] 
                    if stdout.find(expected) == -1:
                        test.fail("ls result not matching what is expected. Should contain file %s name but is: '%s'" % (expected, stdout))
                elif executable == 'aconnect':
                    test.fail('aconnect %s should have generated a failure.' % (command[1]))
            else:
                test.fail('command output is not successful')
                
def on_commands_error(command_failure, commands, extra_arg=None):
    global test
    
    if extra_arg != 'should generate an error' and not isinstance(command_failure.value, unittest.FailTest):
        #print "@@@@@@@@@"
        #print ">>>> ERROR:"
        #pprint.pprint({'failure':command_failure, 'commands':commands, 'extra_arg':extra_arg})
        #print "@@@@@@@@@"
        test.fail('on_commands_error() with error: %s' % (command_failure.value.message))
    else:
        pass # has generated an error as expected.
    
class Test_1_Commands(unittest.TestCase):
    """
    tests for utils.commands
    """
    def setUp(self):
        global test
        test = self
    
    def test_1_find_commands(self):
        """
        tries to find a commond that exists, and one that doesn't
        """
        aconnect = commands.find_command('aconnect') # should work
        try:
            commands.find_command('spam_egg_bacon_ham')
        except commands.CommandNotFoundError:
            pass # successfully found error
        else:
            self.fail('Did not realize that we are trying to find an unexisting command.')
    
    def test_2_ls_and_echo(self):
        """
        tests the ls and echo commands 
        """
        all_commands = [
                ['echo', 'spam'],
                ['ls', '-l']
            ]
        extra_arg = {
            'echo': 'spam', 
            'ls': 'test.log'
            }
        deferred = commands.commands_start(all_commands, on_commands_results, extra_arg) # sets the callback
        return deferred
    
    def XXtest_3_handle_errors(self):
        # TODO should fail ?
        all_commands = ['aconnect', 'egg']
        extra_arg = 'should generate an error'
        try:
            deferred = commands.commands_start(all_commands, on_commands_results, extra_arg).addErrback(on_commands_error)
            return deferred
        except commands.CommandNotFoundError:
            # pass 
            raise
