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
Library for starting miville and telnet in a pexpect test.
Usage : 

import unittest
from test import lib_miville_telnet as libmi

tester = libmi.MivilleTester(self)
tester.start_miville_process()
tester.start_telnet_process()

class Test_ExampleOneMivilleTest(unittest.TestCase):
    def setUp(self):
        global tester
        self.tester = tester
    def test_01_simple(self):
        self.tester.tst('c -l', 'pof:')

""" 
import unittest
import pexpect
import os
import time
import sys
import tempfile
import commands

def get_color(color=None):
    """
    Returns ANSI escaped color code.
    Colors can be either 'BLUE' or 'MAGENTA' or None
    colors = {'BLACK':30, 'RED':31, 'GREEN':32, 'YELLOW':33, 'BLUE':34, 'MAGENTA':35, 'CYAN':36, 'WHITE':37}
    """
    if os.environ['TERM'] not in ['xterm', 'xterm-color', 'rxvt', 'rxvt-unicode']:
        return ''
    colors = {'BLACK':30, 'RED':31, 'GREEN':32, 'YELLOW':33, 'BLUE':34, 'MAGENTA':35, 'CYAN':36, 'WHITE':37}
    try:
        s = str(colors[color]) + 'm'
    except KeyError:
        s = '0m' # default (usually white)
    return "\x1b[" + s

def println(s, endl=True):
    """
    Prints a line to standard output with a prefix.
    """
    if endl:
        print "\n", get_color('MAGENTA'), ">>>>", s, get_color()
    else:
        print "\n", ">>>>", s, # note the comma (",") at end of line

class MivilleTester(object):
    def __init__(self, unittest, **kwargs):
        """
        Update default options here
        """
        self.unittest = unittest
        self.port_offset = 0
        self.miville_command = "../mivilled"
        self.use_tmp_home = False
        self.verbose = True
        self.log_prefix = ''
        self.color = 'CYAN'
        self.telnet_logfile = sys.stdout #child.logfile 
        self.miville_logfile = sys.stdout #child.logfile 
        #override attributes
        self.__dict__.update(kwargs)
        # non-overridable attributes
        self.miville_process = None
        self.telnet_process = None
        # operations
        if self.use_tmp_home:
            self.miville_home = self._make_tmp_dir()
        else:
            self.miville_home = os.path.expanduser("~/.miville")

    def _make_tmp_dir(self):
        """
        Creates a temporary dir and return its path.
        """
        tmp = tempfile.mktemp()
        os.mkdir(tmp)
        if self.verbose:
            print "using %s/ as a miville_home)" % (tmp)
        return tmp

    def start_miville_process(self):
        """
        Starts the miville server 
        """
        command = "%s -o %s -m %s" % (self.miville_command, self.port_offset, self.miville_home)
        try:
            directory = os.getcwd()
            if self.verbose:
                println('Current working dir: ' + directory)
                println('Starting \"%s\"' % command)
                self.miville_process = pexpect.spawn(command, logfile=self.miville_logfile, timeout=0.01) # ProcessOutputLogger(logPrefix, color)
            else:
                self.miville_process = pexpect.spawn(command)
            self.sleep(0.9) # seconds
            if self.is_running(self.miville_process) == False:
                raise Exception("Process %s could not be started. Not running. Is an other miville running at the same time?" % (command))
        except pexpect.ExceptionPexpect, e:
            println("Error starting process %s: %s" % (command, str(e)))
            raise
        self.sleep(0.5) # seconds

    def start_telnet_process(self):
        """
        Starts the telnet process
        """
        command = "telnet %s %s" % ("localhost", self.port_offset + 14444)
        try:
            self.telnet_process = pexpect.spawn(command, logfile=self.telnet_logfile, timeout=0.01) # ProcessOutputLogger(logPrefix, color)
            sleep(0.5) # seconds
            if self.is_running(self.telnet_process) == False:
                raise Exception("Telnet could not be started. Not running. %s" % (command))
        except pexpect.ExceptionPexpect, e:
            println("Error starting process %s: %s" % (command, str(e)))
            raise
        self.sleep(0.3) # seconds

    
    def make_sure_miville_runs(self):
        ok = self.is_running(self.miville_process)
        
    def is_running(self, process):
        """
        Process is a pexpect.spawn object
        
        Returns boolean
        """
        if process is None:
            print "is_running: process is None"
            return False
        if process.isalive() == False:
            status = "no status available" 
            if process:
                status = process.status
            print "is_running: process.isalive() is False"
            println("Error starting process: %s" % status)
            return False
        else:
            return process

    def kill_process(self, process):
        """
        Kills a pexpect.spawn object
        
        See kill -l for flags
        """
        print "KILLING process" # + str(process)
        try:
            if self.is_running(process) == True:
                process.kill(15)
                self.sleep(0.3)
                if is_running(process) == True:
                    process.kill(9)
        except Exception, e:
            print "Error killing process", e
            raise

    def expectTest(self, expected, message, timeout=2):
        """
        Fails a test if the expected regex value is not read from the client output.
        
        The expected value can be a string that is compiled to a regular expression (re)
        or the name of a Exception class.
        
        Succeeds otherwise.
        """
        # other listed expectations are child classes of Exception
        index = self.telnet_process.expect([expected, pexpect.EOF, pexpect.TIMEOUT], timeout=timeout) # 2 seconds max
        #self.evalTest(index, message)
        self.unittest.assertEqual(index, 0, message)
        self.unittest.failIfEqual(index, 1, 'Problem : Unexpected EOF')
        self.unittest.failIfEqual(index, 2, 'Problem : Time out.')

    def tst(self, command, expected, timeout=2, errorMsg=None):
        """
        Convenient little gem of a tst method that
        sends a telnet request and tests for an expected reply.
        """
        self.telnet_process.sendline(command)
        err = errorMsg or 'The command did not return: "%s" as expected' % expected
        # self.sleep(0.1)
        self.expectTest(expected, err)
        #self.telnet_process.readlines()
        #self.miville_process.readlines()
        # self.sleep(0.1)
    
    def flush(self):
        """
        Flushes the output buffer of the process to stdout.
        """
        for child in (self.telnet_process, self.miville_process):
            if child is not None:
                try:
                    child.readline()
                except pexpect.TIMEOUT:
                    pass

    def sleep(self, duration=0.3):
        """
        Non-blocking sleep
        """
        end = time.time() + duration
        while time.time() < end:
            time.sleep(0.01)
            self.flush()

def die():
    """
    Ends the programs with error flag.
    """
    println("EXITING")
    sys.exit(1)

def execute_bash_command_string(cmd):
    """
    Executes a bash string 

    You can use pipes and other shell goodies.
    """
    status = commands.getstatusoutput(cmd)
    output = status[1]
    print cmd
    lines = output.split("\n")
    for line in lines:
        if line.find(cmd) == -1:
            print line
    print

# def kill_all_running_miville(self):
#     """
#     Not implemented yet
# 
#     Kills all instances of running miville
# 
#     """
#     pass

class Test_OneMivilleTest(unittest.TestCase):
    """
    Base class for tests that start one miville and one telnet client 
    """
    def test_01_simple(self):
        self.tester = MivilleTester(self)
        self.tester.start_miville_process()
        self.tester.start_telnet_process()
        # def tst(self, command, expected, timeout=2, errorMsg = None):
        self.tester.tst('c -l', 'toto')

