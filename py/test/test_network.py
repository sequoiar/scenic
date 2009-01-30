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

class Test_Network_Test(unittest.TestCase):
    """
    Integration tests for v4l2 devices.
    """
    def test_01_unidirectional_local_to_remote(self):
        # We need a remote host with iperf set in the contacts.
        # and it must be selected.
        app.api.delete_contact(app.me, 'brrr')
        app.api.add_contact(app.me, 'brrr', '10.10.10.65')
        app.api.select_contact(app.me, 'brrr')
        
        app.api.network_test_start(app.me)
        for i in range(3):
            print "please wait..."
            app.go(1)
        
        # cleanup
        app.api.delete_contact(app.me, 'brrr')

