#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technoligiques (SAT)
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

"""
System test for the telnet UI.
"""
import unittest
import pexpect
import os
import time
import sys
# ---------------------------------------------------------------------
# config 
server_exec = os.path.expanduser("~/src/miville/trunk/py/miville.py")
client_command = "telnet localhost 14444"
waiting_delay = 1.0 # seconds
# ---------------------------------------------------------------------
# functions
def println(s,endl=True):
    """
    Prints a line to standard output
    """
    if endl:
        print ">>>>",s
    else:
        print ">>>>",s, # note the comma (",") at end of line

def die():
    """
    Ends the programs with error flag.
    """
    println("EXITING")
    sys.exit(1)

# ---------------------------------------------------------------------
# startup poutine
# starting the server
try:
    println("Starting server")
    server = pexpect.spawn(server_exec)
    #server.logfile = sys.stdout
    println("Waiting %f seconds..." % (waiting_delay))
    time.sleep(waiting_delay) # seconds
except pexpect.ExceptionPexpect,e:
    println("Error starting server:"+e)
    die()

# global variable for the telnet client child process
client = None

# starting the client
try:
    println("Starting client")
    client = pexpect.spawn(client_command)
except pexpect.ExceptionPexpect,e:
    println("Error starting client:"+e)
    die()
    
#client.logfile = sys.stdout
#s = client.sendline
# ---------------------------------------------------------------------
# classes
class TelnetBaseTest(unittest.TestCase):
    """
    Telnet test case parent class
    """
    messages = {
        'prompt':"pof: ",
        'greeting':"Welcome to Sropulpof!",
        'not found':"command not found"
    }
    def setUp(self):
        """
        Starts a Telnet client for tests.
        """
        global client
        self.client = client
        #self.timeout = 1000
        
    def tearDown(self):
        """
        Destructor for each test. Nothing to do.
        """
        pass
# ---------------------------------------------------------------------
# test classes
class Test_1_AddressBook(TelnetBaseTest):
    def test_1_default_prompt(self):
        index = self.client.expect(['pof: ', pexpect.EOF, pexpect.TIMEOUT])
        self.assertEqual(index, 0, 'The default prompt is not appearing.')
        self.failIfEqual(index, 1, 'Problem : Unexpected EOF')
        self.failIfEqual(index, 2, 'Problem : Time out.')
        
    def test_2_add_contact(self):
        # adding a contact
        # c -a name ip [port]
        self.client.sendline("c -a Juliette 154.123.2.3")
        index = self.client.expect(['pof: ', pexpect.EOF, pexpect.TIMEOUT])
        self.assertEqual(index, 0, 'The default prompt is not appearing.')
        self.client.sendline("c -l") 
        index = self.client.expect(['Juliette'])
        self.assertEqual(index, 0, 'The contact that has just been added is not appearing.')
        
        
        
