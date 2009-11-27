#!/usr/bin/env python 
# -*- coding: utf-8 -*-
#
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
"""
Unit tests for miville.streams.proc
"""
import os
import tempfile
import warnings

from twisted.trial import unittest
from twisted.internet import defer
from twisted.python import failure
from twisted.internet import reactor

from miville.streams import proc
from miville.streams import milhouse 
from miville.streams import constants 
from miville.utils import log

# make log not verbose
proc.log = log.start("warning", False, False, "proc.overriden")

_globals = {}
_globals_02 = {}
_globals_05 = {}

class Test_01_Process_Manager(unittest.TestCase):
    """
    Tests the ProcessManager
    """
    def test_01_parse_process_output(self):
        """
        test parsing dummy process output using twisted.
        """
        class DummyProcessManager(proc.ProcessManager):
            def _on_out_received(self, data):
                """
                Overrides method in proc.ProcessManager so that we can
                test output
                """
                if data == self.tester.old_data:
                    self.tester.line_doubled = True
                    # make sure the correct iteration doubled/was detected
                    self.tester.correct_iteration_doubled = (data == 'iteration 20')
                else:
                    self.tester.old_data = data # record old data for subsequent comparison
            
        def _later():
            global _globals
            manager = _globals["manager"]
            #print("Calling stop()")
            manager.stop()

        def _state_changed_slot(state):
            global _globals
            #print("state of process changed to %s" % (state))
            test_case = _globals["test_case"]
            manager = _globals["manager"]
            deferred = _globals["deferred"]
            if state == proc.STATE_RUNNING:
                DURATION = 1.0
                reactor.callLater(DURATION, _later)
                #print("will call _later in 2 seconds.")
            elif state == proc.STATE_STOPPED:
                if not test_case.line_doubled:
                    msg = "doubled output was not detected"
                    #print(msg)
                    deferred.errback(failure.Failure(Exception(msg)))
                elif not self.correct_iteration_doubled:
                    msg = "wrong iteration doubled"
                    #print(msg)
                    deferred.errback(failure.Failure(Exception(msg)))
                else:
                    deferred.callback(True)
            elif state == proc.STATE_ERROR:
                _globals["test_case"].fail("state is error. Output : %s" % (manager.stdout_logger.get_text())) 
        
        # generate bash script here, this will be our process
        scriptStr = """
        #!/bin/bash
        line=0

        while true
        do 
            line=$(($line+1))
            if [ $line -eq 20 ]; then 
                echo "iteration $line"
            fi 
            echo "iteration $line"
            sleep 0.001 
        done
        """
        filename = tempfile.mktemp()
        try:
            f = open(filename, 'w')
            f.write(scriptStr)
        except IOError, e:
            raise
        finally:
            f.close()
        
        manager = DummyProcessManager(name="dummy", command=["bash", filename])
        deferred = defer.Deferred()
        global _globals
        _globals["test_case"] = self
        _globals["manager"] = manager
        _globals["deferred"] = deferred
        manager.tester = self
        manager.tester.line_doubled = False
        manager.tester.old_data = '' 
        manager.state_changed_signal = _state_changed_slot # FIXME: proper way would be to connect it to a method
        manager.start()
        return deferred
    
    def test_02_start_and_stop_mplayer(self):
        """
        Mplayer process using twisted and JACK.
        """
        def _later():
            global _globals_02
            manager = _globals_02["manager"]
            #print("Calling stop()")
            manager.stop()

        def _state_changed_slot(state):
            global _globals_02
            #print("state of process changed to %s" % (state))
            test_case = _globals_02["test_case"]
            manager = _globals_02["manager"]
            deferred = _globals_02["deferred"]
            if state == proc.STATE_RUNNING:
                DURATION = 10.0
                reactor.callLater(DURATION, _later)
                #print("will call _later in 2 seconds.")
            elif state == proc.STATE_STOPPED:
                deferred.callback(True)
            elif state == proc.STATE_ERROR:
                _globals_02["test_case"].fail("state is error. Output : %s" % (manager.stdout_logger.get_text())) 

        MOVIEFILENAME = "/var/tmp/excerpt.ogm"
        if not os.path.exists(MOVIEFILENAME):
            warnings.warn("File %s is needed for this test." % (MOVIEFILENAME))
        else:
            manager = proc.ProcessManager(name="mplayer", command=["mplayer", "-ao", "jack", MOVIEFILENAME])
            manager.state_changed_signal = _state_changed_slot # FIXME: proper way would be to connect it to a method
            deferred = defer.Deferred()
            _globals_02["test_case"] = self
            _globals_02["manager"] = manager
            _globals_02["deferred"] = deferred
            manager.tester = self
            manager.start()
            return deferred


    def test_05_parse_milhouse_output(self):
        """
        test parsing MilhouseProcessManager output using twisted.
        """
        def _later():
            global _globals_05
            manager = _globals_05["manager"]
            manager.stop()
        
        def _state_changed_slot(state):
            global _globals_05
            print("state of process changed to %s" % (state))
            test_case = _globals_05["test_case"]
            manager = _globals_05["manager"]
            deferred = _globals_05["deferred"]
            if state == proc.STATE_RUNNING:
                DURATION = 0.1
                reactor.callLater(DURATION, _later)
            elif state == proc.STATE_STOPPED or state == proc.STATE_ERROR:
                if not test_case.got_problem:
                    msg = "didn't get problem signal"
                    deferred.errback(failure.Failure(Exception(msg)))
                elif test_case.got_invalid_problem:
                    msg = "Got invalid problem signal"
                    deferred.errback(failure.Failure(Exception(msg)))
                elif test_case.problem_count != 3:
                    msg = "Expected 3 problem signals, only got %s" % (self.problem_count)
                    deferred.errback(failure.Failure(Exception(msg)))
                else:
                    print "all test conditions met, IT'S OVER"
                    deferred.callback(True)
                
            
        class DummySlot(object):
            """ This class will handle MilhouseManager's problem_signal """
            def __init__(self, tester, problem_signal):
                self.tester = tester
                problem_signal.connect(self.on_problem)

            def _is_valid_error(self, error):
                VALID_ERRORS = ("error: CRITICAL:Jack is not running", 
                        "error: ERROR:argument error: must be sender OR receiver.",
                        "error: WARNING:Buffer time 8 is too low, using 11333 instead")
                return str(error).strip() in VALID_ERRORS

            def on_problem(self, value=''):
                self.tester.got_problem = True
                if self._is_valid_error(value):
                    self.tester.problem_count += 1
                else:
                    self.tester.got_invalid_problem
        
        # generate bash script here, this will be our process
        # it sleeps at the end so that we have time to stop it aka 
        # we won't get reactor unclean stuff
        scriptStr = """
        #!/bin/bash
        echo "CRITICAL:Jack is not running"
        echo "ERROR:argument error: must be sender OR receiver."
        echo "FAKE:this is not interesting"
        echo "WARNING:Buffer time 8 is too low, using 11333 instead"
        sleep 5
        """

        filename = tempfile.mktemp()
        try:
            f = open(filename, 'w')
            f.write(scriptStr)
        except IOError, e:
            warnings.warn("IOError %s" % (e.message))
            self.fail("FILE IO error: %s" (e.message))
        finally:
            f.close()
            
        # hide milhouse output
        milhouse.log = log.start("critical", 1, 0, __name__) 
        manager = milhouse.MilhouseProcessManager(name="dummy", command=["bash", filename])
        self.got_problem = False
        self.got_invalid_problem = False
        self.problem_count = 0
        deferred = defer.Deferred()
        global _globals_05
        _globals_05["test_case"] = self
        _globals_05["manager"] = manager
        _globals_05["deferred"] = deferred
        self.problem_count = 0
        self.slot = DummySlot(self, manager.problem_signal)
        manager.tester = self
        manager.state_changed_signal = _state_changed_slot # FIXME: proper way would be to connect it to a method
        manager.start()
        print "started manager"
        return deferred
    test_05_parse_milhouse_output.skip = "Tristan, please fix me."

