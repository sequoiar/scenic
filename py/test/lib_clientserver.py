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

Miville executable directory must be in your PATH and PYTHONPATH.
""" 
import unittest
import pexpect
import os
import time
import sys
import tempfile
import commands
import string

# module variables
VERBOSE = False # you can override this from your test

def echo(s, verbose=None):
    """
    Prints a line to standard output with a prefix.
    """
    global VERBOSE
    if verbose is True:
        print s
    elif verbose is False:
        pass
    else:
        if VERBOSE:
            print s

def die():
    """
    Ends the programs with error flag.
    """
    echo("EXITING")
    sys.exit(1)

def create_tmp_dir():
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

def open_logfile(path, filename):
    """
    Opens a file name to write and returns its handle. 
    If the directory doesn't exist, creates it.
    """
    try:
        os.makedirs(path)
    except OSError, e:
        pass
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
    #TODO: test and use
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
            if line.find("python ../miville.py") != -1:
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
        self.logfile = None
        self.verbose = False
        self.logfile_path = "." # overriden by the tester
        self.logfile_name = "default.log"
        self.delayafterclose = 0.2 # important to override this in child classes
        self.timeout_expect = 0.9
#         self.maxread = 2000
        self.expected_when_started = ""  #TODO
        self.test_case = None # important : must be a valid TestCase instance. Should be overriden by kwargs
        self.__dict__.update(kwargs)
        self.child = None # pexpect.spawn object

    def kill(self):
        """
        Kills a pexpect.spawn object
        
        See kill -l for flags
        """
        echo("KILLING process", self.verbose)
        if self.child is not None:
            self.child.delayafterclose = self.delayafterclose
            # TODO: delayafterterminate
            try:
                self.child.close()
                if self.logfile is not sys.stdout:
                    pass # self.logfile.close()
            except IOError, e:
                echo(e.value, self.verbose)
            except ValueError, e:
                echo(e.value, self.verbose)
    
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
        if self.logfile is None: 
            if self.verbose:
                self.logfile = sys.stdout
            else:
                self.logfile = open_logfile(self.logfile_path, self.logfile_name)
        command = self.make_command()
        try:
            directory = os.getcwd()
            if self.verbose:
                echo('Current working dir: ' + directory, self.verbose)
                echo('Starting \"%s\"' % command, self.verbose)
            kwargs = {
                'timeout':self.timeout_expect,
#                 'maxread':self.maxread
                }
            if self.logfile is not None:
                kwargs['logfile'] = self.logfile
            self.child = pexpect.spawn(command, **kwargs) 
            if self.expected_when_started != "":
                index = self.child.expect(self.expected_when_started, timeout=self.timeout_expect)
                self.test_case.assertEqual(index, 0, "Process is not ready")
            else:
                self.sleep(self.timeout_expect)
        except pexpect.ExceptionPexpect, e:
            echo("Error starting process %s." % (command), self.verbose)
            raise
        if not self.is_running():
            raise Exception("Child process is not running. Is an other similar process already running ? Command used : %s" % (command))
    
    def sendline(self, line):
        self.child.sendline(line)
    
    def expect_test(self, expected, message=None, timeout=-1):
        """
        Fails a test if the expected regex value is not read from the client output.
        
        The expected value can be a string that is compiled to a regular expression (re)
        or the name of a Exception class.
        
        Succeeds otherwise.
        """
        if message is None:
            message = "Expected \"%s\" but did not get it." % (expected)
        if timeout == -1:
            timeout = self.timeout_expect
        self.flush_output()
        # other listed expectations are child classes of Exception
        index = self.child.expect([expected, pexpect.EOF, pexpect.TIMEOUT], timeout=timeout) # 2 seconds max
        self.test_case.assertEqual(index, 0, message)
        self.test_case.failIfEqual(index, 1, 'Problem : Unexpected EOF')
        self.test_case.failIfEqual(index, 2, 'Problem : Time out.')

    def send_expect(self, command, expected, timeout=-1, message=None):
        """
        Sends a text string and tests for an expected reply.
        Wraps self.child.sendline and self.expect_test.
        """
        self.child.sendline(command)
        self.expect_test(expected, message, timeout)
        # self.flush_output()
    
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
        raise NotImplementedError("You must override this method")
# ---------------------------------------------------

class ClientServerTester(object):
    """
    Testing manager for a client and a server process.
    """
    SERVER_CLASS = Process
    CLIENT_CLASS = Process

    def __init__(self, **kwargs):
        """
        Update default options here

        :param name: Used as a prefix for log files.
        """
        self.verbose = False
        self.test_case = None
        self.name = 'default' # the only thing the user must set...
        self.__dict__.update(kwargs)
        self.log_to_file = not self.verbose
        self.client = None
        self.server = None
        self.logfile_path = "../_trial_log"
        self.logfile_name = "default.log" # overriden !
    
    def setup(self, test_case):
        """
        Should be called in the setUp method of unittest.TestCase classes.
        :param test_case: unittest.TestCase instance. Typically self. 
        """
        if self.log_to_file:
            self.logfile_name = "%s.%s.%s.%s" % (
                test_case.__class__.__module__, 
                test_case.__class__.__name__, 
                test_case._testMethodName, 
                self.name 
                )
        self.test_case = test_case
        if self.server is None:
            self.start_server()
        if self.client is None:
            self.start_client()

    def start_server(self, **kwargs):
        """
        Factory for server process
        :param kwargs: **kwargs for child
        """
        self._start_child('server', self.SERVER_CLASS, **kwargs)

    def start_client(self, **kwargs):
        """
        Factory for client process
        :param kwargs: **kwargs for child
        """
        self._start_child('client', self.CLIENT_CLASS, **kwargs)
    
    def _start_child(self, which, klass, **kwargs):
        """
        Called by self.start_server and self.start_client
        :param which: "client" or "server"
        :param klass: self.SERVER_CLASS or self.CLIENT_CLASS
        :param kwargs: **kwargs for child
        """
        kwargs['test_case'] = self.test_case  #IMPORTANT
        kwargs['logfile_path'] = self.logfile_path
        kwargs['verbose'] = self.verbose
        kwargs['logfile_name'] = "%s.%s.log" % (self.logfile_name, which) # this is correct. do not change this
        self.__dict__[which] = klass(**kwargs)
        self.__dict__[which].start()

    def kill_children(self):
        """
        Stops all children processes
        """
        for child in [self.client, self.server]:
            if child is not None:
                try:
                    child.kill()
                except Exception, e:
                    echo(e.message, self.verbose)

    # one-to-one mapping of methods: ----------------
    def send_expect(self, command, expected, timeout=-1, message=None):
        self.client.send_expect(command, expected, timeout, message)
    def sendline(self, line):
        self.client.sendline(line)
    def expect_test(self, expected, message=None, timeout=-1):
        self.client.expect_test(expected, message, timeout)
    def sleep(self, duration=0.1):
        """
        Non-blocking sleep that flushes the output.
        """
        end = time.time() + duration
        while time.time() < end:
            if self.client is not None:
                self.client.sleep(0.01)
            if self.server is not None:
                self.server.sleep(0.01)

class TelnetProcess(Process):
    """
    telnet client process
    """
    def __init__(self, **kwargs):
        self.port_offset = 0
        self.host = "localhost"
        self.port = 14444
        Process.__init__(self, **kwargs)
        echo("starting %s(%s)" % (self.__class__.__name__, kwargs), self.verbose)

    def make_command(self):
        return "telnet %s %d" % (self.host, self.port + self.port_offset)

class TelnetForMivilleProcess(TelnetProcess):
    """
    telnet client process specific to Miville's use.
    """
    def __init__(self, **kwargs):
        kwargs['expected_when_started'] = "pof"
        TelnetProcess.__init__(self, **kwargs)

class TelnetForMilhouseProcess(TelnetProcess):
    """
    telnet client process specific to Milhouse's use.
    """
    def __init__(self, **kwargs):
        kwargs['expected_when_started'] = "log:"
        TelnetProcess.__init__(self, **kwargs)
    def stop(self):
        self.send_expect('stop:', 'stop: ack="ok"')

class MivilleProcess(Process):
    """
    miville.py server process
    """
    def __init__(self, **kwargs):
        self.port_offset = 0
        self.miville_home = "~/.miville"
        self.use_tmp_home = True
        kwargs['expected_when_started'] = "Miville is ready"
        Process.__init__(self, **kwargs)
        if self.use_tmp_home:
            self.miville_home = create_tmp_dir()
            echo("MIVILLE HOME : %s" % (self.miville_home), self.verbose)
        echo("starting %s(%s)" % (self.__class__.__name__, kwargs), self.verbose)

    def make_command(self):
        return "../miville.py -o %s -m %s -C" % (self.port_offset, os.path.expanduser(self.miville_home))

class TelnetMivilleTester(ClientServerTester):
    """
    Tests miville with telnet
    """
    SERVER_CLASS = MivilleProcess
    CLIENT_CLASS = TelnetForMivilleProcess

    def __init__(self, **kwargs):
        """
        Custom attributes for this tester.
        """
        self.port_offset = 0
        self.miville_home = "~/.miville"
        self.use_tmp_home = True
        ClientServerTester.__init__(self, **kwargs)

    def _start_child(self, which, klass, **kwargs):
        """
        We specify attributes of self to pass to children
        
        Called by self.start_server and self.start_client
        :param which: "client" or "server"
        :param klass: self.SERVER_CLASS or self.CLIENT_CLASS
        :param kwargs: **kwargs for child
        """
        if which == 'client':
            attrs_to_pass = ['port_offset']
        elif which == 'server':
            attrs_to_pass = ['port_offset', 'miville_home', 'use_tmp_home']
        for attr_name in attrs_to_pass:
            kwargs[attr_name] = self.__dict__[attr_name]
        ClientServerTester._start_child(self, which, klass, **kwargs)

class MilhouseProcess(Process):
    """
    milhouse tcp server process
    """
    def __init__(self, **kwargs): # mode=[r|s], serverport=9000
        self.mode = 't'
        self.serverport = '8000'
        kwargs['expected_when_started'] = "READY"
        Process.__init__(self, **kwargs)

    def make_command(self):
        return "milhouse -%s --serverport %s" % (self.mode, self.serverport)

class TelnetMilhouseTester(ClientServerTester):
    """
    Tests milhouse with telnet
    """
    SERVER_CLASS = MilhouseProcess
    CLIENT_CLASS = TelnetForMilhouseProcess
    
    def __init__(self, **kwargs):
        self.mode ='t'
        self.serverport = '0'
        ClientServerTester.__init__(self, **kwargs)
    
    def _start_child(self, which, klass, **kwargs):
        """
        We specify attributes of self to pass to children
        
        Called by self.start_server and self.start_client
        :param which: "client" or "server"
        :param klass: self.SERVER_CLASS or self.CLIENT_CLASS
        :param kwargs: **kwargs for child
        """
        if which == 'client':
            kwargs['port'] = self.serverport
        elif which == 'server':
            kwargs['serverport'] = self.serverport
            kwargs['mode'] = self.mode
        ClientServerTester._start_child(self, which, klass, **kwargs)

    def stop(self):
        self.send_expect('stop:', 'stop: ack="ok"')

    def start(self):
        self.send_expect('start:', 'start: ack="ok"')

    def quit(self):
        self.send_expect('quit:', '')
