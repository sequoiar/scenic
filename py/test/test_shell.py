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
    def on_commands_results(self, results, commands, callback=None):                
        # overriden
        test = callback
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
                        if stdout != 'toto\n':
                            test.fail("result not matching what is expected. Should be toto but is %s" % (stdout))
                    elif command[0] == 'ls':
                        fname = 'test.log'    
                        if stdout.find('test.log') == -1:
                            test.fail("result not matching what is expected. Should contain file %s name but is: '%s'" % (fname,stdout))
                    elif command[0] == 'aconnect':
                        pass    
                        #print 'aconnect error : ',stderr
                        #test.fail('aconnect succeded')
                else:
                    test.fail('command output is not successful')
    def on_commands_error(self,results,commands,callback=None):
        # overriden
        test = callback
        test.fail('this never seems to be called')
        print "SHELL ERROR:"
        print results
        print commands
        print callback
        
class Test_0_Shell(unittest.TestCase):
    """
    tests for utils.shell
    """
    def test_1_find_commands(self):
        x = TestShellCommander()
        aconnect = x.find_command('aconnect')
        try:
            x.find_command('spam_egg_bacon_ham')
            self.fail('Found successfully an unexisting command.')
        except:
            pass
    
    def test_2_ls_and_echo(self):
        x = TestShellCommander()
        commands = [
                ['echo','toto'],
                ['ls','-a'],
                ['aconnect', 'asd'] # should generate an error
                # TODO: create error returned
                # TODO: ['not_existing'] not found command triggers a very bad error so far
            ]
        x.commands_start(commands, self) # HACK : for the test, we pass the unittest instance instead of a callback
        time.sleep(0.1)
    
    def test_3_handle_errors(self):
        #TODO: do we handle errors properly? (if a process returns an error code. Not sure.
        self.fail('we should handle processes errors properly')
        # TODO: example : ['aconnect','asd'] should generate an error

