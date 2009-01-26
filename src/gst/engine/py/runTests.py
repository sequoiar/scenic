#!/usr/bin/env python
# -*- coding: utf-8 -*-

# for testing

import unittest
import pexpect
import os
import time
import sys

waiting_delay = 1.0 # seconds before starting client after server start

# ---------------------------------------------------------------------
# a class for output redirection
class ProcessOutputLogger:
    """
    Adds a prefix to each line printed by a spawn process.
    
    you must assign a reference to an instance of this class to 
    the logfile attribute of a spawn object
    """
    def __init__(self, prefixStr=''):
        self.prefix = prefixStr
        self.buffer = []

    def write(self, s):
        self.buffer.append(self.prefix + str(s).replace('\n', '\n' + self.prefix))

    def flush(self):
        pass

    def real_flush(self):
        """
        Actually flushes the buffer of this output buffer
        
        Adds some pretty colors as well.
        """
        sys.stdout.write(getColor('CYAN'))
        for s in self.buffer:
            sys.stdout.write(s)
        sys.stdout.write(getColor())
        sys.stdout.flush()
        self.buffer = []

# ---------------------------------------------------------------------
# functions
def println(s, endl=True):
    """
    Prints a line to standard output with a prefix.
    """
    if endl:
        print getColor('MAGENTA'), ">>>>", s, getColor()
    else:
        print ">>>>", s, # note the comma (",") at end of line

def start_process(command, isVerbose=False, logPrefix=''):
    """
    Command is a string to execute
    
    Returns a pexpect.spawn object
    """
    try:
        println('Starting \"%s\"' % command)
        
        if isVerbose:
            process = pexpect.spawn(command, logfile=ProcessOutputLogger(logPrefix))
        else:
            process = pexpect.spawn(command)
        #time.sleep(waiting_delay) # seconds
        if not is_running(process):
            print 'NOT RUNNING, I MUST DIE\n'
            die(process)
        else:
            return process
    except pexpect.ExceptionPexpect, e:
        println("Error starting receiver: " + str(e))
        die(process)

def is_running(process):
    """
    Process is a pexpect.spawn object
    
    Returns boolean
    """
    return process.isalive()


def getColor(c=None):
    """
    Returns ANSI escaped color code.
    
    Colors can be either 'BLUE' or 'MAGENTA' or None
    """
    if c == 'BLUE':
        s = '31m'
    elif c == 'CYAN':
        s = '36m'
    elif c == 'MAGENTA':
        s = '35m'
    else:
        s = '0m' # default (black or white)
    return "\x1b[" + s

def kill_process(process):
    """
    Kills a pexpect.spawn object
    
    See kill -l for flags
    """
    try:
        if is_running(process):
            process.kill(15)
            time.sleep(2)
            if is_running(process):
                process.kill(9)
    except Exception, e:
        print "Error killing process", e
    
def die(process):
    """
    Ends the programs with error flag.
    """
    println("EXITING")
    kill_process(process)
    sys.exit(1)


# ---------------------------------------------------------------------
# System test classes
class SropulpofBaseTest(unittest.TestCase):
    """
    Sropulpof system test case parent class
    """
    def setUp(self):
        """
        Setup for each test. 
        """
        self.receiver = start_process(os.path.expanduser("python sropulpof.py -r -o 5000"), True)
        print 'running rx'
        self.sender = start_process(os.path.expanduser("python sropulpof.py -s -o 5000"), True)
        print 'running tx'

        

    def tearDown(self):
        """
        Destructor for each test. 
        """
        pass

    def prepareTest(self):
        pass


    def sleep(self):
        """Waits a bit between each command."""
        time.sleep(0.025)

    def evalTest(self, index, message):
        """
        Fails a test, displaying a message, if the provided index resulting 
        from expect() matches some of the indices provided by expectTest()
        """
        self.assertEqual(index, 0, message)
        self.failIfEqual(index, 1, 'Problem : Unexpected EOF')
        self.failIfEqual(index, 2, 'Problem : Time out.')

    def expectTest(self, expected, message):
        """
        Fails a test if the expected regex value is not read from the receiver output.
        
        The expected value can be a string that is compiled to a regular expression (re)
        or the name of a Exception class.
        
        Succeeds otherwise.
        """
        # other listed expectations are child classes of Exception
        index = self.receiver.expect([expected, pexpect.EOF, pexpect.TIMEOUT], timeout=2) # 2 seconds max
        self.evalTest(index, message)

class Test_1(SropulpofBaseTest):
    """
    Systems tests for Sropulpof
    """
    def test_01_defaults(self):
        self.expectTest('*', 'do anything')

