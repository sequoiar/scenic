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
System test tools for a com_chan ping.

Tests with prefic dualtest_* expects that you first start a miville on both local 
and remote host.

Usage: trial test/<scriptname>.py
""" 
import unittest
import pexpect
import os
import time
import sys

import miville.utils.telnet_testing as testing

testing.VERBOSE_CLIENT = False #True
testing.VERBOSE_SERVER = False
testing.START_SERVER = False # You must start miville manually on both local and remote host.
testing.start()

class Test_1_Connection(testing.TelnetBaseTest):
    def test_01_add_contact(self):
        """
        Makes sure the contact is in our addressbook and selects it.
        """
        name = testing.REMOTE_CONTACT_NAME
        ip = testing.REMOTE_HOST 
        self.client.sendline("c -a %s %s" % (name, ip))
        self.sleep()
        #self.expectTest('Contact added', 'Could not add contact.')
    
    def test_02_select_contact(self):
        name = testing.REMOTE_CONTACT_NAME
        self.client.sendline("c -s %s" % name)
        self.sleep(0.4)
        # tests if contact is selected
        # self.expectTest('selected', 'Could not select contact.')
        
    def test_03_join(self):
        """
        Joins the selected contact
        """
        self.client.sendline("j -s")
        self.sleep(0.4)
        # TODO : test if successful
        #self.expectTest('accepted', 'Could not join contact.')

class Test_2_Ping(testing.TelnetBaseTest):
    """
    System Tests for ping.
   
    pof> ping
    """
    def test_01_ping(self):
        self.client.sendline("ping")
        self.sleep(0.3)
        self.expectTest('pong', 'Did not receive pong answer.')

