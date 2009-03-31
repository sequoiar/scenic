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

"""
Integration tests within the API for Network performance testing.

Usage: trial test/test_network.py
""" 
# system imports

import unittest
import sys

# app imports
import imiville as app

# startup poutine
##app.main()
##app.me.verbose = False
##app.view.verbose = False #True 
##app.go()
# XXX the folowwing line represents the address of the remote host to test with
##remote = ("tzing", "10.10.10.66") # It must have a miville running with standard port numbers.
##previous = None 

class Dont_Test_Network_Test(): #unittest.TestCase):
    """
    Integration tests for network tests.
    
    We need to start miville on tzing prior to run this test.

    We need a remote host with iperf set in the contacts.
    and it must be selected.
    """
    def setUp(self):
        self._verbose = True
    
    def _initial_setup(self):
        app.api.delete_contact(app.me, remote[0])
        app.api.add_contact(app.me, remote[0], remote[1], 2222)
        app.api.select_contact(app.me, remote[0])
        app.api.start_connection(app.me)
        if self._verbose:
            print "\nwaiting for 2 seconds:"

    def tearDown(self):
        app.api.delete_contact(app.me, remote[0])
    
    def _run_miville_a_little_while(self):
        global previous
        if self._verbose:
            sys.stdout.write(".")
            sys.stdout.flush()
        app.go(0.1)
        if app.last is not previous:
            if self._verbose:
                print "Miville notification: ", app.last.value 
            previous = app.last
        
    def test_01_unidirectional(self):
        """
        network_test_start(self, caller, bandwidth=1, duration=10, kind="unidirectional")
        
        iperf -c 10.10.10.66 -t 1 -y c -u -b 30M 
        """
        self._initial_setup()
        
        app.api.network_test_start(app.me, 30, 1, "unidirectional")
        if self._verbose:
            print "\nwaiting for 3 seconds:"
        for i in range(30):
            self._run_miville_a_little_while()
        
        if not isinstance(app.last.value, dict):
            self.fail("last notification should contain a dict.")
        #  {'loss': 0.0, 'bandwidth': 30, 'jitter': 0.001, 'duration': 1, 'ip': '10.10.10.66', 'kind': 1}
        for k in ("loss", "jitter", "bandwidth"):
            if not app.last.value.has_key(k):
                self.fail("The dict in the notification from network_test should contain key %s" % k)

    def test_02_tradeoff(self):
        """
        network_test_start(self, caller, bandwidth=1, duration=10, kind="tradeoff")
        
        iperf -c 10.10.10.66 -t 1 -y c -u -b 30M -r
        """
        if self._verbose:
            print "\nwaiting for 1 seconds:"
        for i in range(10):
            self._run_miville_a_little_while()
        app.api.network_test_start(app.me, 30, 1, "tradeoff")
        if self._verbose:
            print "\nwaiting for 6 seconds:"
        for i in range(60):
            self._run_miville_a_little_while()
        
        if not isinstance(app.last.value, dict):
            self.fail("last notification should contain have a dict as value.")
        #  {'loss': 0.0, 'bandwidth': 30, 'jitter': 0.001, 'duration': 1, 'ip': '10.10.10.66', 'kind': 1}
        for k in ("loss", "jitter", "bandwidth"):
            if not app.last.value.has_key(k):
                self.fail("The dict in the notification from network_test should contain key %s" % k)

