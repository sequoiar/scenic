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
System test tools for network tests.

Usage: trial test/<scriptname>.py
""" 
import unittest
import pexpect
import os
import time
import sys

import miville.utils.telnet_testing as testing

testing.VERBOSE_CLIENT = False
testing.VERBOSE_SERVER = False
testing.START_SERVER = False # You must start miville manually on both local and remote host.
testing.start()

class Test_0_Cli(testing.TelnetBaseTest):
    """
    General tests for the CLI
    """
    def test_01_unknown_command(self):
        self.client.sendline('qwetyqrwetyqwertye')
        self.sleep()
        self.expectTest('command not found', 'Unknown command didn\'t give error.')

class Test_1_AddressBook(testing.TelnetBaseTest):
    """
    Systems tests for the Address Book
    """
    
    def test_01_add_contact(self):
        name = testing.REMOTE_CONTACT_NAME
        ip = testing.REMOTE_HOST 
        self.client.sendline("c -a %s %s" % (name, ip))
        self.sleep()
        #self.expectTest('Contact added', 'Could not add contact.')
    
    def test_02_select_contact(self):
        name = testing.REMOTE_CONTACT_NAME
        self.client.sendline("c -s %s" % name)
        self.sleep(0.4)
        # TODO : test if contact is selected
        self.expectTest('selected', 'Could not select contact.')
        
class Test_2_Join(testing.TelnetBaseTest):
    """
    Joins the selected contact
    """
    def test_01_join(self):
        self.client.sendline("j -s")
        self.sleep(0.4)
        # TODO : test if successful
        # self.expectTest('accepted', 'Could not join contact.')

class Test_3_Network(testing.TelnetBaseTest):
    """
    System Tests for network testing.
    
    network -s -t 1 -k remotetolocal
    network -s -t 1 -k localtoremote
    network -s -t 1 -k dualtest
    network -s -t 1 -k tradeoff
    """
    def test_01_remotetolocal(self):
        self.client.sendline("network -s -t 1")
        self.sleep(1.9)
        self.expectTest('speed', 'Could not do network test. (default)')

    def test_02_localtoremote(self):
        self.client.sendline("network -s -t 1 -k localtoremote")
        self.sleep(1.9)
        self.expectTest('speed', 'Could not do network test. (localtoremote)')

    def test_03_dualtest(self):
        self.client.sendline("network -s -t 1 -k dualtest")
        self.sleep(1.9)
        self.expectTest('speed', 'Could not do network test. (dualtest)')

    def test_04_tradeoff(self):
        self.client.sendline("network -s -t 1 -k tradeoff")
        self.sleep(1.9)
        self.expectTest('speed', 'Could not do network test. (tradeoff)')
   
