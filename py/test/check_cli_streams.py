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
Tests if miville can stream between two alice mivilles.
"""

import unittest
import pexpect
import sys
from test import lib_miville_telnet as libmi
import os

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
    miville_logfile=open(os.path.join(logdir, "log_streams_alice_miville.txt"), 'w'),
    telnet_logfile=open(os.path.join(logdir, "log_streams_alice_telnet.txt"), 'w'),
    )
bob = libmi.MivilleTester(
    port_offset=3, 
    verbose=VERBOSE, use_tmp_home=True, 
    miville_logfile=open(os.path.join(logdir, "log_streams_bob_miville.txt"), 'w'),
    telnet_logfile=open(os.path.join(logdir, "log_streams_bob_telnet.txt"), 'w'),
    )
alice.start_miville_process()
alice.start_telnet_process()
bob.start_miville_process()
bob.start_telnet_process()

import time

class Test_Miville_Streams(unittest.TestCase):
    def setUp(self):
        global alice
        alice.unittest = self
        self.alice = alice
        global bob
        bob.unittest = self
        self.bob = bob

    def _send_expect_non_blocking(self, which_peer, command, expected, timeout=2.0):
        """
        Sends a string to telnet client
        Expects some output
        Meanwhile, flushes the output of the other agent's client and server processes.
        """
        which_peer.telnet_process.sendline(command)
        got_it = False
        start_time = time.time()
        end_time = start_time + timeout
        while not got_it:
            now = time.time()
            if now > end_time:
                break
            # check if got expected
            index = which_peer.telnet_process.expect([expected, libmi.pexpect.EOF, libmi.pexpect.TIMEOUT], timeout=0.1)
            if index == 0: #what we expect
                got_it = True
            #self._flush_both()
            #flush this agent's miville output
            child = which_peer.miville_process
            if child.isalive():
                try:
                    child.readline()
                except pexpect.TIMEOUT, e:
                    pass
            # flush the other agent output
            for agent in [self.alice, self.bob]:
                if agent is not which_peer:
                    agent.flush()
            time.sleep(0.1)
        if not got_it:
            self.fail("Expected \"%s\" but did not get it. Refer to the logfile %s" \
                    % (expected, which_peer.miville_process.logfile.name))

    def _flush_both(self):
        # stdout, not telnet output
        self.alice.flush()
        self.bob.flush()
        
    def test_01_cli_works(self):
        self.alice.tst('c -l', 'pof:')
        self._flush_both()
        self.bob.tst('c -l', 'pof:')
        self._flush_both()

    def test_02_add_contacts(self):
        self.alice.tst("c -a Bob 127.0.0.1 2225", "added")
        self._flush_both()
        self.bob.tst("c -a Alice 127.0.0.1 2224", "added")
        self._flush_both()
        self.alice.tst("c -s Bob", "selected")
        self._flush_both()
        self.bob.tst("c -s Alice", "selected") #useless
        self._flush_both()

    def test_03_join(self):
        self.alice.telnet_process.sendline("j -s")
        self.bob.expectTest("Do you accept")
        self.bob.telnet_process.sendline("Y")
        self.alice.expectTest('accepted', 'Connection not successful.')
        self._flush_both()

    def lets_sleep(self, duration):
        timeout = duration # let's stream for 10 seconds.
        start_time = time.time()
        end_time = start_time + timeout
        while True:
            now = time.time()
            if now > end_time:
                break
            self.alice.sleep(0.1)
            self.bob.sleep(0.1)
        
    def test_04_start_stop_streams(self):
        self._send_expect_non_blocking(alice, "z -s Bob", "Successfully started to stream", timeout=15.0)
    
        self.lets_sleep(15.0) # let's sleep for 15 seconds.
        self._send_expect_non_blocking(alice, "z -i Bob", "Successfully stopped to stream", timeout=15.0)

#    def test_04_start_streams(self):
#        # FIXME: doesnt actually test anything
#        self.alice.telnet_process.sendline("z -s Bob")
#        for t in range(3):
#            duration = float(t + 1)
#            self.alice.sleep(duration)
#            self.bob.sleep(duration)
#
#    def test_05_stop_streams(self):
#        # FIXME: doesnt actually test anything
#        self.alice.telnet_process.sendline("z -i Bob")
#        duration = 1.0
#        self.alice.sleep(duration)
#        self.bob.sleep(duration)

    def test_06_disconnect(self):
        # mandatory prior to delete contacts
        self.alice.telnet_process.sendline("j -i")
        self.alice.expectTest("Connection stopped")
        #TODO: self.bob.expectTest("Connection has been closed")
        self._flush_both()
    
    def test_98_quit(self):
        self.alice.telnet_process.sendline("c -e Bob")
        self.bob.telnet_process.sendline("c -e Alice")
        self._flush_both()
        self.alice.telnet_process.sendline("quit")
        self.bob.telnet_process.sendline("quit")
        self._flush_both()

    def test_99_kill_miville(self):
        self._flush_both()
        self.alice.kill_miville_and_telnet()
        self.bob.kill_miville_and_telnet()
        # close files
        try:
            self.alice.miville_logfile.close()
            self.alice.telnet_logfile.close()
            self.bob.miville_logfile.close()
            self.bob.telnet_logfile.close()
        except ValueError:
            pass
        

