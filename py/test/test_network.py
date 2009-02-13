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
# along with Sropulpof.  If not, see <http://www.gnu.org/licenses/>.

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
app.main()
app.me.verbose = False
app.view.verbose = False #True 
app.go()
remote = ("tzing", "10.10.10.66")
previous = None 

class Test_Network_Test(unittest.TestCase):
    """
    Integration tests for v4l2 devices.
    """
    def setUp(self):
        app.api.delete_contact(app.me, remote[0])
        app.api.add_contact(app.me, remote[0], remote[1], 2222)
        app.api.select_contact(app.me, remote[0])

    def _run_miville(self):
        global previous
        print "please wait..."
        if app.last is not previous:
            print "Miville notification: ", app.last.value 
            previous = app.last
        app.go(0.1)
        

    def test_01_unidirectional(self):
        # We need a remote host with iperf set in the contacts.
        # and it must be selected.
        app.api.start_connection(app.me)
        for i in range(5):
            self._run_miville()
        app.api.network_test_start(app.me)
        for i in range(5):
            self._run_miville()
        # cleanup
        app.api.delete_contact(app.me, remote[0]) 

