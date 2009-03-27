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
Starts two miville and tests the protocols/pinger module.
"""
import unittest
import pexpect
import os
import time
import sys
import tempfile

import utils.telnet_testing as testing

testing.VERBOSE_CLIENT = True
testing.VERBOSE_SERVER = True

class DualLocalBaseTest(testing.TelnetBaseTest):
    """
    Telnet test starting two miville processes.
    """
    def sleep(self, duration=0.1):
        """
        Waits a bit between each command.
        :param duration: float seconds

        REMOVED SOME PARTS OF THE METHOD COPIED FROM utils/telnet_testing.py
        """
        end = time.time() + duration
        while time.time() < end:
            time.sleep(0.001)

    def setUp(self):
        """
        Starts two miville processes and one telnet client.
        """
        self.mivilles = {
            'first':{
                'port_offset':0, 
                'contact_name':'Pierre', # Pierre Miville, 1st swiss in Nouvelle France !
                'verbose_server':False, 
                'command':os.path.expanduser('../miville.py'), # XXX parent dir since trial changes os.getcwd()
                'home':'', 
                'process':None
            }, 
            'second':{
                'port_offset':1, 
                'contact_name':'Charlotte', # Charlotte Maugis, his wife.
                'verbose_server':False, 
                'command':os.path.expanduser('../miville.py'),
                'home':'',
                'process':None
            } 
        }

        if testing.START_SERVER:
            for miville in self.mivilles.values():
                sys.stdout.write(testing.get_color('MAGENTA'))
                # if testing.CHANGE_HOME_PATH:
                TMP_NAME = tempfile.mktemp()
                os.mkdir(TMP_NAME)
                miville['home'] = TMP_NAME
                miville['command'] = "%s -o %s" % (miville['command'], miville['port_offset'])
                if testing.VERBOSE_SERVER:
                    print "pwd is ", os.environ['PWD']
                    print "current working directory", os.getcwd()
                    print "miville command:", miville['command']
                    print "using %s/ as a $HOME. (for adb and log)" % (miville['home'])
                sys.stdout.write(testing.get_color())
                # XXX : we do not want to change HOME in this test !!!!!
                #os.environ['HOME'] = miville['home']
                # spawning a pexpect
                miville['process'] = testing.start_process(miville['command'], testing.VERBOSE_SERVER, "S>", 'BLUE')
                print "sleeping for 1 second...."
                time.sleep(1.0)
        if testing.VERBOSE_CLIENT:
            print "VERBOSE_CLIENT"
            print "starting telnet client"
        # spwning a pexpect
        self.client = testing.start_process(testing.CLIENT_COMMAND, testing.VERBOSE_CLIENT, "C>", 'CYAN')
        print "sleeping for 1 second...."
        self.sleep(1)
    
    def tearDown(self):
        """
        Destructor for each test. 
        """
        pass


class Test_01_Ping(DualLocalBaseTest):
    """
    Tests for ping.
    
    pof> ping
    """
    def test_01_ping(self):
        self.client.sendline("c -s Charlotte")
        self.sleep(0.1)
        self.client.sendline("j -s")
        self.sleep(0.4)
        self.expectTest('accepted', 'Connection not successful.')
        self.sleep(0.1)
        self.client.sendline("ping")
        self.client.sendline("")
        self.sleep(0.2)
        #self.expectTest('pong', 'Did not receive pong answer.')
        # XXX disabled for now !
