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
Library for testing a client and a server.

Use cases:
 * telnet and miville
 * telnet and milhouse
""" 
import unittest
import pexpect
import os
import time
import sys
import tempfile
import commands
import string
# from miville.utils.common import get_def_name

# module variables
VERBOSE = False

def echo(s, endl=True):
    """
    Prints a line to standard output with a prefix.
    """
    global VERBOSE
    if VERBOSE:
        if endl:
            print s
        else:
            print s, 
            # note the comma (",") at end of line

def die():
    """
    Ends the programs with error flag.
    """
    echo("EXITING")
    sys.exit(1)

def use_tmp_dir():
    """
    Creates a temporary dir in /tmp/ and return its path.
    """
    # TODO : add prefix
    global VERBOSE
    tmp = tempfile.mktemp()
    os.mkdir(tmp)
    if VERBOSE:
        print "Created %s/ directory." % (tmp)
    return tmp

def open_logfile(self, path, filename):
    """
    Opens a file name to write and returns its handle. 
    If the directory doesn't exist, creates it.
    """
    os.makedirs(path)
    fullpath = os.path.join(path, filename)
    return open(fullpath, 'w')


def shell_command(txt):
    """
    Executes a bash string 

    You can use pipes and other shell goodies.
    """
    global VERBOSE
    status = commands.getstatusoutput(cmd)
    output = status[1]
    lines = output.split("\n")
    if VERBOSE:
        print txt
        for line in lines:
            if line.find(txt) == -1:
                print line
        print 

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

def kill_all_zombie_processes():
    """
    Kills all instances of running zombie process some other tests might have started.

    (if there are some)
    Right now, only killing miville is implemented.
    """
    echo("Checking for running processes...")
    command = "ps aux | grep miville | grep -v grep | grep python"
    echo(command)
    output = commands.getstatusoutput(command)[1]
    echo(output)
    lines = output.splitlines()
    for line in lines:
        try:
            if line.find("python ./miville.py") != -1:
                tokens = line.split()
                proc = tokens[1]
                pid = int(proc)
                print "WARNING! WE NEED TO KILL A ZOMBIE MIVILLE INSTANCE. PID: %s"  % (proc)
                out = commands.getstatusoutput("kill %s" % (proc))[1]
                echo(out)
                time.sleep(0.1)
                out = commands.getstatusoutput("kill -KILL %s" % (proc))[1]
                echo(out)
                time.sleep(0.1)
                os.kill(pid, 9)
                time.sleep(0.1)
        except IndexError, e:
            raise
        except TypeError, e:
            raise
        except OSError,e :
            pass

class Process(object):
    """
    Manages a process using pexpect.
    """
    def __init__(self, **kwargs):
        global VERBOSE
        self.logfile = sys.stdout
        self.verbose = VERBOSE
        self.logfile_suffix = ""
        self.delayafterclose = 0.1 # important to override this in child classes
        self.timeout_expect = 0.1
        self.expected_when_started = ""
        self.test_case = None # important : must be a valid TestCase instance.
        self.LOGFILE_EXTENSION = "txt"
        self.__dict__.update(kwargs)
        self.child = None # pexpect.spawn object
    
    def open_logfile(self, filename):
        """
        Opens the log file in write mode
        """
        self.logfile = open("%s%s.%s" % (filename, self.logfile_suffix, self.LOGFILE_EXTENSION), 'w')

    def kill(self):
        """
        Kills a pexpect.spawn object
        
        See kill -l for flags
        """
        echo("KILLING process")
        if self.child is not None:
            self.child.delayafterclose = self.delayafterclose
            # TODO: delayafterterminate
            try:
                self.child.close()
                if self.logfile is not sys.stdout:
                    self.logfile.close()
            except IOError, e:
                echo(e.value)
            except ValueError, e:
                echo(e.value)
    
    def is_running(self):
        """
        Returns boolean
        """
        if self.child is None:
            return False
        return self.child.isalive()

    def start(self):
        """
        Starts the process
        """
        command = self.make_command()
        try:
            directory = os.getcwd()
            if self.verbose:
                echo('Current working dir: ' + directory)
                echo('Starting \"%s\"' % command)
            self.child = pexpect.spawn(command, logfile=self.logfile, timeout=self.timeout_expect) 
            # TODO : add expectation here.
            try:
                self.expect_test(self.expected_when_started, "process did not output: %s" % self.expected_when_started, timeout=0.5)
            except:
                self.sleep(0.4) # seconds
        except pexpect.ExceptionPexpect, e:
            echo("Error starting process %s." % (command))
            raise
        if not self.is_running():
            raise Exception("Process could not be started. Not running. Is an other similar process already running ? %s" % (command))
    
    def expect_test(self, expected, message=None, timeout=-1):
        """
        Fails a test if the expected regex value is not read from the client output.
        
        The expected value can be a string that is compiled to a regular expression (re)
        or the name of a Exception class.
        
        Succeeds otherwise.
        """
        if message is None:
            message = "Expected %s but did not get it." % (expected)
        if timeout == -1:
            timeout = self.timeout_expect
        # other listed expectations are child classes of Exception
        index = self.child.expect([expected, pexpect.EOF, pexpect.TIMEOUT], timeout=timeout) # 2 seconds max
        self.test_case.assertEqual(index, 0, message)
        self.test_case.failIfEqual(index, 1, 'Problem : Unexpected EOF')
        self.test_case.failIfEqual(index, 2, 'Problem : Time out.')

    def send_and_expect(self, command, expected, timeout=-1, message=None):
        """
        Sends a text string and tests for an expected reply.
        Wraps self.child.sendline and self.expect_test.
        """
        self.child.sendline(command)
        self.expect_test(expected, message, timeout)
    
    def flush_output(self):
        """
        Flushes the output buffer of the process to stdout. (or a file)
        """
        if self.child is not None:
            if self.child.isalive():
                try:
                    self.child.readline()
                except pexpect.TIMEOUT:
                    pass

    def sleep(self, duration=0.1):
        """
        Non-blocking sleep that flushes the output.
        """
        end = time.time() + duration
        while time.time() < end:
            time.sleep(0.01)
            self.flush_output()

    def make_command(self):
        """
        :return string: Shell command to execute in order to start this process.
        """
        raise NotImplementedError("You should override this method")
# ---------------------------------------------------

class ClientServerTester(object):
    """
    Testing manager for a client and a server process.
    """
    SERVER_CLASS = Process
    CLIENT_CLASS = Process
    def __init__(self, name, **kwargs):
        """
        Update default options here

        :param name: Used as a prefix for log files.
        """
        global VERBOSE
        self.name = name
        self.verbose = VERBOSE
        self.logfile_prefix = "default_" # TODO: use caller file name
        self.client_logfile = sys.stdout
        self.server_logfile = sys.stdout
        self.test_case = None
        self.__dict__.update(kwargs)
        self.server_kwargs = {'logfile':self.server_logfile}
        self.client_kwargs = {'logfile':self.client_logfile}
        self.client = None
        self.server = None
    
    def setup(self,test_case):
        """
        Should be called in the setUp method of unittest.TestCase classes.
        :param test_case: unittest.TestCase instance. Typically self. 
        """
        self.test_case = test_case
        for child in (self.client, self.server):
            child.test_case = test_case
    def prepare(self):
        raise NotImplementedError()

    def start(self):
        """
        Starts server and client processes.
        """
        self.prepare()
        self.server = self.SERVER_CLASS(**self.server_kwargs)
        self.client = self.CLIENT_CLASS(**self.client_kwargs)
        self.server.start()
        self.client.start()

    def kill_client_and_server(self):
        for child in [self.client, self.server]:
            if child is not None:
                try:
                    child.kill()
                except Exception, e:
                    echo(e.message)
            

class TelnetProcess(Process):
    """
    telnet client process
    """
    def __init__(self, **kwargs):
        self.host = "localhost"
        self.port = 14444
        Process.__init__(self, **kwargs)

    def make_command(self):
        return "telnet %s %d" % (self.host, self.port)

class MivilleProcess(Process):
    """
    miville.py server process
    """
    def __init__(self, **kwargs):
        self.port_offset = 0
        self.miville_home = "~/.miville"
        self.use_tmp_home = False
        Process.__init__(self, **kwargs)

    def make_command(self):
        command = "./miville.py -o %s -m %s -C" % (self.port_offset, os.path.expanduser(self.miville_home))
        return command

class TelnetMivilleTester(ClientServerTester):
    """
    Tests miville with telnet
    """
    SERVER_CLASS = MivilleProcess
    CLIENT_CLASS = TelnetProcess


    def __init__(self, **kwargs): # name, port_offset=0, use_tmp_home=False):
        # TODO: manage log files.
        self.port_offset = 0
        self.miville_home = "~/.miville"
        self.use_tmp_home = False
        ClientServerTester.__init__(self, **kwargs)

    def prepare(self):
        if self.use_tmp_home:
            self.miville_home = use_tmp_dir()
        self.server_kwargs = {
            'logfile':self.server_logfile, 
            'port_offset':self.port_offset, 
            'miville_home':self.miville_home
        }
        self.client_kwargs = {
            'logfile':self.client_logfile, 
            'port':14444 + self.port_offset
        }

class MilhouseProcess(Process):
    """
    milhouse tcp server process
    """
    def __init__(self, **kwargs): # mode=[r|s], serverport=9000
        self.mode = 't'
        self.serverport = '8000'
        Process.__init__(self, **kwargs)

    def make_command(self):
        command = "milhouse -%s --serverport %s" % (self.mode, self.serverport)
        return command

class TelnetMilhouseTester(ClientServerTester):
    """
    Tests milhouse with telnet
    """
    SERVER_CLASS = MilhouseProcess
    CLIENT_CLASS = TelnetProcess
    
    def __init__(self, **kwargs):
        self.mode ='t'
        self.serverport = '0'
        ClientServerTester.__init__(self, **kwargs)
    
    def prepare(self):
        self.server_kwargs = {
            'logfile':self.server_logfile, 
            'mode':self.mode, 
            'serverport':self.serverport
        }
        self.client_kwargs = {
            'logfile':self.client_logfile, 
            'port':self.serverport 
        }
