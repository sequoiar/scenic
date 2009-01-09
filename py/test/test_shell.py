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

from utils import shell
import utils.log

del shell.log
shell.log = utils.log.start('error', 1, 0, 'shell')

class TestShellCommander(shell.ShellCommander):
    def __init__(self, unit_test):
        self.test = unit_test
        
    def on_commands_results(self, results, commands, extra_arg=None):
        # overriden
        for i in range(len(results)):
            result = results[i]
            success, results_infos = result
            if isinstance(results_infos,failure.Failure):
                print 'FIXME',str(results_infos) #TODO handle failures
                pass # if there is an error, the programmer should fix it.
            else:
                command = commands[i]
                stdout, stderr, signal_or_code = results_infos
                if success:
                    #print stdout
                    if command[0] == 'echo':
                        expected = extra_arg['echo']
                        if stdout.find(expected) == -1:
                            self.test.fail("echo result not matching what is expected. Should be %s but is %s" % (expected, stdout))
                    elif command[0] == 'ls':
                        expected = extra_arg['echo'] 
                        if stdout.find(expected) == -1:
                            self.test.fail("result not matching what is expected. Should contain file %s name but is: '%s'" % (expected, stdout))
                    elif command[0] == 'aconnect':
                        self.test.fail('aconnect %s should have generated a failure.' % (command[1]))
                else:
                    test.fail('command output is not successful')
                    
    def on_commands_error(self, command_failure, commands, extra_arg=None):
        # overriden
        if extra_arg != 'should generate an error' and not isinstance(command_failure.value,unittest.FailTest):
            #print "@@@@@@@@@"
            #print ">>>> ERROR:"
            #pprint.pprint({'failure':command_failure, 'commands':commands, 'extra_arg':extra_arg})
            #print "@@@@@@@@@"
            self.test.fail('on_commands_error() with error: %s' % (command_failure.value.message))
        
class Test_0_Shell(unittest.TestCase):
    """
    tests for utils.shell
    """
    def test_1_find_commands(self):
        x = TestShellCommander(self)
        aconnect = x.find_command('aconnect')
        try:
            x.find_command('spam_egg_bacon_ham')
            self.fail('Did not realize that we are trying to find an unexisting command.')
        except:
            pass
    
    def test_2_ls_and_echo(self):
        x = TestShellCommander(self)
        commands = [
                ['echo','spam'],
                ['ls','-l']
                # TODO: create error returned
                # TODO: ['not_existing'] not found command triggers a very bad error so far
            ]
        extra_arg = {
            'echo':'spam', 
            'ls':'test.log'
            }
        x.commands_start(commands, extra_arg)
        time.sleep(0.1)
    
    def test_3_handle_errors(self):
        x = TestShellCommander(self)
        
        commands = ['aconnect', 'egg']
        extra_arg = 'should generate an error'
        try:
            x.commands_start(commands, extra_arg)
        except shell.CommandNotFoundException:
            pass
        time.sleep(0.1)
