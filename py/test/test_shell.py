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
        for i in range(len(results)):
            result = results[i]
            success, results_infos = result
            if isinstance(results_infos,failure.Failure):
                pass # if there is an error, the programmer should fix it.
            else:
                command = commands[i]
                stdout, stderr, signal_or_code = results_infos
                if success:
                    #print stdout
                    if command[0] == 'echo':
                        if stdout != 'toto\n':
                            callback.fail("result not matching what is expected. Should be toto but is %s" % (stdout))
                    if command[0] == 'ls':
                        if stdout.find('test.log') == -1:
                            callback.fail("result not matching what is expected. Should contain this file name but is %s" % (stdout))
                else:
                    callback.fail('command output is not successful')
    def on_commands_error(self,commands,callback=None):
        # overriden
        pass
        print commands
        print callback
        
class Test_0_Shell(unittest.TestCase):
    """
    tests for utils.shell
    """
    def test_1_ls_and_echo(self):
        d = TestShellCommander()
        commands = [
                ['echo','toto'],
                ['ls','-a']
                # TODO: create error returned
                # TODO: ['not_existing'] not found command triggers a very bad error so far
            ]
        d.commands_start(commands, self) # HACK : for the test, we pass the unittest instance instead of a callback
        time.sleep(0.1)
        
