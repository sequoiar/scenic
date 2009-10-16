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

tester = libmi.MivilleTester()
tester.start_miville_process()
tester.start_telnet_process()

class Test_ExampleOneMivilleTest(unittest.TestCase):
    def setUp(self):
        global tester
        tester.unittest = self
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
import string

def get_color(color=None):
    """
    Returns ANSI escaped color code.
    Colors can be either 'BLUE' or 'MAGENTA' or None
    colors = {'BLACK':30, 'RED':31, 'GREEN':32, 'YELLOW':33, 'BLUE':34, 'MAGENTA':35, 'CYAN':36, 'WHITE':37}
    """
    try:
        if os.environ['TERM'] not in ['xterm', 'xterm-color', 'rxvt', 'rxvt-unicode']:
            return ''
    except KeyError:
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
    def __init__(self, **kwargs):
        """
        Update default options here
        """
        self.unittest = None
        self.port_offset = 0
        self.miville_command = "./mivilled"
        self.use_tmp_home = False
        self.verbose = True
        self.color = 'CYAN'
        self.logfile_prefix = "default"
        #override attributes
        self.__dict__.update(kwargs)
        self.telnet_logfile = sys.stdout #child.logfile 
        self.miville_logfile = sys.stdout #open("./miville%d.log" % (self.port_offset), 'w') #child.logfile 
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
        command = "%s -o %s -m %s -C" % (self.miville_command, self.port_offset, self.miville_home)
        try:
            directory = os.getcwd()
            if self.verbose:
                println('Current working dir: ' + directory)
                println('Starting \"%s\"' % command)
                self.miville_process = pexpect.spawn(command, logfile=self.miville_logfile, timeout=0.01) # ProcessOutputLogger(logPrefix, color)
            else:
                self.miville_process = pexpect.spawn(command)
            self.miville_process.expect(["Miville is ready"], 2.0) # self.sleep(0.9) # seconds
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
            self.sleep(0.5) # seconds
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
            # print "is_running: process is None"
            return False
        if process.isalive() == False:
            status = "no status available" 
            if process:
                status = process.status
            #print "is_running: process.isalive() is False"
            #println("Error starting process: %s" % status)
            return False
        else:
            return process

    def kill_miville_and_telnet(self):
        # todo quit
        self.sleep(0.3)
        for process in [self.telnet_process, self.miville_process]:
            if process is not None:
                self.kill_process(process)

    def kill_process(self, process):
        """
        Kills a pexpect.spawn object
        
        See kill -l for flags
        """
        print "KILLING process" # + str(process)
        if process is not None:
            process.close()
            now = time.time()
            while process.isalive() == True:
                self.sleep(0.01)
                # dont' wait for isalive for more than 2 seconds
                if time.time() > (now + 2):
                    break
            try:
                if self.is_running(process) == True:
                    process.kill(15)
                    self.sleep(0.3)
                    if is_running(process) == True:
                        process.kill(9)
            except Exception, e:
                print "Error killing process", e
                raise

    def expectTest(self, expected, message=None, timeout=2):
        """
        Fails a test if the expected regex value is not read from the client output.
        
        The expected value can be a string that is compiled to a regular expression (re)
        or the name of a Exception class.
        
        Succeeds otherwise.
        """
        if message is None:
            message = "Expected \"%s\" but did not get it." % (expected)
        # other listed expectations are child classes of Exception
        index = self.telnet_process.expect([expected, pexpect.EOF, pexpect.TIMEOUT], timeout=timeout) # 2 seconds max
        #self.evalTest(index, message)
        self.unittest.assertEqual(index, 0, message)
        self.unittest.failIfEqual(index, 1, 'Problem : Unexpected EOF')
        self.unittest.failIfEqual(index, 2, 'Problem : Time out.')
        self.sleep(0.01)

    def tst(self, command, expected, timeout=2, errorMsg=None):
        """
        Convenient little gem of a tst method that
        sends a telnet request and tests for an expected reply.
        """
        self.telnet_process.sendline(command)
        err = errorMsg or 'The command did not return: "%s" as expected' % expected
        # self.sleep(0.1)
        self.expectTest(expected, err, timeout=timeout)
        #self.telnet_process.readlines()
        #self.miville_process.readlines()
        # self.sleep(0.1)
    
    def flush(self):
        """
        Flushes the output buffer of the process to stdout. (or a file)
        """
        for child in (self.telnet_process, self.miville_process):
            if child is not None:
                if child.isalive():
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


def kill_all_running_miville():
    """
    Kills all instances of running miville.

    (if there are some)
    """
    print "Checking for running miville instances......"
    output = commands.getstatusoutput("ps aux | grep miville | grep -v grep | grep python")[1]
    print output
    lines = output.splitlines()
    for line in lines:
        try:
            if line.find("python ./mivilled") != -1:
                tokens = line.split()
                proc = tokens[1]
                pid = int(proc)
                print "WARNING! WE NEED TO KILL A MIVILLE INSTANCE NOW. PID: %s"  % (proc)
                print commands.getstatusoutput("kill %s" % (proc))[1]
                time.sleep(0.1)
                print commands.getstatusoutput("kill -KILL %s" % (proc))[1]
                time.sleep(0.1)
                os.kill(pid, 9)
                time.sleep(0.1)
        except IndexError, e:
            raise
        except TypeError, e:
            raise
        except OSError,e :
            pass

def get_env_variable(key):
    """
    Returns the value of an environment variable.
    
    Catches the errors in case there are some.
    """
    ret = None
    try:
        ret = os.environ[key]
    except KeyError:
        pass
    return ret

def get_test_remote_host():
    """
    Returns the configured remote miville host IP for a test.

    Used for testing with two computers.
    Looks for environment variables.
    Example in bash:
    export MIVILLE_TEST_REMOTE_HOST="10.10.10.65" 
    """
    ret = get_env_variable('MIVILLE_TEST_REMOTE_HOST')
    if ret is None:
        ret = '127.0.0.1' # default
    return ret

