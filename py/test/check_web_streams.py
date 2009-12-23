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
Tests the Web interface streaming between 2 miville on the local host.

 * This scripts starts 2 miville.
 * selenium-server needs to be running.
 * The path to selenium.py in is PYTHONPATH : %s

Expects::

 * The invitation has been accepted
 * Streaming (Medium-Bandwidth 2-Channel raw)
 * Streaming (Medium-Bandwidth 2-Channel raw)
 * Stopped streaming
"""
from selenium import selenium
import time
import unittest
import re
import os
import commands
from test import lib_miville_telnet as libmi

# ----------------- setup
libmi.kill_all_running_miville()
VERBOSE = True
logdir = os.path.join(os.getcwd(), "_trial_log")
try:
    os.makedirs(logdir) # make sure we have the dir
except:
    pass # too bad  
alice = libmi.MivilleTester(
    port_offset=2, 
    verbose=VERBOSE, use_tmp_home=True, 
    miville_logfile=open(os.path.join(logdir, "log_web_alice_miville.txt"), 'w'),
    telnet_logfile=open(os.path.join(logdir, "log_web_alice_telnet.txt"), 'w'),
    )
bob = libmi.MivilleTester(
    port_offset=3, 
    verbose=VERBOSE, use_tmp_home=True, 
    miville_logfile=open(os.path.join(logdir, "log_web_bob_miville.txt"), 'w'),
    telnet_logfile=open(os.path.join(logdir, "log_web_bob_telnet.txt"), 'w'),
    )
alice.start_miville_process()
alice.start_telnet_process()
bob.start_miville_process()
bob.start_telnet_process()


def print_usage():
    print("""
    Running distributed web tests.
    This command must be ran from the same dir.
    We assume that :
     - Miville is running on local host (alice) and remote host. (bob)
     - The remote host (bob) has a contact for alice with auto-answer enabled.
     - There are no contact named bob in the local addressbook.
     - Selenium server is running
     - The local miville is neither connected nor streaming to any remote agent.
    """ % (os.getenv("PYTHONPATH")))

def delete_unwanted_contacts():
    print("Deleting contact bob using the CLI.")
    actions = [
        "c -l", # list contacts
        "c -s bob", # select contact bob
        "j -i", # stop streams with bob
        "c -e", # erase contact bob
        ]
    for action in actions:
        comm = """echo %s | nc localhost 14446 -q 1 """ % (action)
        ret = commands.getoutput(comm)
        print(ret)
        time.sleep(0.1)
    
    print("Creating contact alice in bob's addressbook using the CLI. Enabling auto answer.")
    actions2 = [
        "c -l", # list contacts
        "c -s alice", # select contact alice
        "j -i", # stop streams with alice
        "c -e", # erase contact alice
        "c -a alice 127.0.0.1 2224 yes", # add contact alice with auto_answer
        ]
    for action in actions2:
        comm = """echo %s | nc localhost 14447 -q 1 """ % (action)
        ret = commands.getoutput(comm)
        print(ret)
        time.sleep(0.1)

class Test_Web_Streams(unittest.TestCase):
    """
    Tests test streams between local miville.
    """
    def setUp(self):
        # libmiville stuff
        global alice
        alice.unittest = self
        self.alice = alice
        global bob
        bob.unittest = self
        self.bob = bob
        # selenium stuff.
        delete_unwanted_contacts() # FIXME: this is to be called only once. 
        #Make sure to remove this if creating more than one test method
        self.verificationErrors = []
        self.selenium = selenium("localhost", 4444, "*chrome", "http://localhost:8082/")
        self.selenium.start()
    
    def _expect(self, txt):
        """
        Checks in the addressbook status HTML element for a string.
        """
        client = self.selenium
        got = client.get_text("adb_status")
        if got.find(txt) == -1:
            self.fail("Expected '%s' but got '%s'." % (txt, got))
        else:
            pass #print("Got '%s' in '%s' as expected." % (txt, got))

    def test_selenium_streams(self):
        client = self.selenium
        print("Opening %s with selenium." % ("http://localhost:8082"))
        client.open("http://localhost:8082") # alice
        time.sleep(0.1)
        client.click("adb_add")
        time.sleep(0.1)
        client.type("adb_name", "bob")
        time.sleep(0.1)
        client.type("adb_address", "127.0.0.1")
        time.sleep(0.1)
        client.type("adb_port", "2225")
        time.sleep(0.1)
        client.click("adb_edit")
        time.sleep(0.1)
        client.click("adb_join") # connect
        time.sleep(5.0)
        self._expect("The invitation has been accepted")
        client.select("strm_global_setts", "label=Medium-Bandwidth 2-Channel raw")
        time.sleep(0.1)
        client.click("//option[@value='18']")
        time.sleep(0.1)
        client.click("strm_start") # start streams
        time.sleep(10.0)
        self._expect("Streaming")
        client.click("strm_start") # stop streams
        time.sleep(5.0)
        self._expect("Stopped streaming")
        client.click("adb_join") # disconnect
        time.sleep(5.0)
        self._expect("Connection stopped")
        client.click("adb_remove")
        time.sleep(0.5)
    
    def tearDown(self):
        self.selenium.stop()
        self.assertEqual([], self.verificationErrors)

#if __name__ == "__main__":
#    unittest.main()
