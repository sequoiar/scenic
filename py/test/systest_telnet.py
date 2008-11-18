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

Usage: trial test/systest_telnet.py
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
BE_VERBOSE = False # False
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

def kill_server():
    """
    Is this needed ?
    Is the KILL signal number correct ?
    """
    global server
    try:
        server.kill(9)
    except Exception,e:
        print "Error killing server process",e
        
def die():
    """
    Ends the programs with error flag.
    """
    println("EXITING")
    kill_server()
    sys.exit(1)

# ---------------------------------------------------------------------
# startup poutine
# global variables for the telnet client and server child process
client = None
server = None

try:
    #delete ~/.sropulpof/sropulpof.adb
    #orig_home = os.environ['HOME']
    os.environ['HOME'] = '/var/tmp'
    os.remove('/var/tmp/.sropulpof/sropulpof.adb')
except Exception,e:
    println("Error removing old sropulpof.adb or setting HOME to /var/tmp."+str(e))

# starting the server
try:
    println("Starting server")
    server = pexpect.spawn(server_exec)
    if BE_VERBOSE:
        server.logfile = sys.stdout
    println("Waiting %f seconds..." % (waiting_delay))
    time.sleep(waiting_delay) # seconds
except pexpect.ExceptionPexpect,e:
    println("Error starting server:"+e)
    die()

# starting the client
try:
    println("Starting client")
    client = pexpect.spawn(client_command)
    if BE_VERBOSE:
        client.logfile = sys.stdout
except pexpect.ExceptionPexpect,e:
    println("Error starting client:"+e)
    die()

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
        self.sleep()
        
        
    def tearDown(self):
        """
        Destructor for each test. Nothing to do.
        """
        pass
        
    def sleep(sleep):
        """Waits a bit between each command."""
        time.sleep(0.050)
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
        self.sleep()
        index = self.client.expect(['Contact added', pexpect.EOF, pexpect.TIMEOUT])
        self.assertEqual(index, 0, 'Contact not added')
        self.failIfEqual(index, 1, 'Problem : Unexpected EOF')
        self.failIfEqual(index, 2, 'Problem : Time out.')
        #index = self.client.expect(['pof: ', pexpect.EOF, pexpect.TIMEOUT])
        #self.assertEqual(index, 0, 'The default prompt is not appearing.')
    
    def test_3_list(self):

        self.client.sendline("c -l") 
        self.sleep()
        index = self.client.expect(['Juliette:', pexpect.EOF, pexpect.TIMEOUT])
        self.assertEqual(index, 0, 'The contact that has just been added is not appearing.')
        self.failIfEqual(index, 1, 'Problem : Unexpected EOF')
        self.failIfEqual(index, 2, 'Problem : Time out.')
        
    def test_4_erase_contact(self):

        self.client.sendline("c -e Juliette")
        self.sleep()
        index = self.client.expect(['Contact deleted', pexpect.EOF, pexpect.TIMEOUT])
        self.assertEqual(index, 0, 'Error while trying to erase contact')
        self.failIfEqual(index, 1, 'Problem : Unexpected EOF')
        self.failIfEqual(index, 2, 'Problem : Time out.')
        #self.client.sendline("c -l") 
        #index = self.client.expect(['Juliette'])
        #self.failIfEqual(index, 0, 'The contact has not been successfully deleted.')
        
    def test_5_delete_invalid_contact(self):
        self.sleep()
        self.client.sendline("c -e some_invalid_name")
        self.sleep()
        index = self.client.expect(['Could not delete', pexpect.EOF, pexpect.TIMEOUT])
        self.assertEqual(index, 0, 'There should be no contact with that name.')
        self.failIfEqual(index, 1, 'Problem : Unexpected EOF')
        self.failIfEqual(index, 2, 'Problem : Time out.')
