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
import pexpect
import os
import time
import sys
# not used yet
from unittest import TextTestRunner
# see  /usr/lib/python2.5/unittest.py

class SysTest:
    """
    Pretty print of tests.
    
    TODO
    """
    pass
#    def assertTest(cmd,exp):
#        pass

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
    
if __name__ == '__main__':
    # config 
    server_exec = os.path.expanduser("~/src/miville/trunk/py/miville.py")
    client_exec = "telnet localhost 14444"
    
    # start server
    try:
        println("Starting server")
        server = pexpect.spawn(server_exec)
        server.logfile = sys.stdout
        println("Waiting 500 ms...")
        time.sleep(0.5) # seconds
    except pexpect.ExceptionPexpect,e:
        println("Error starting server:"+e)
        die()
    
    #start client
    try:
        println("Starting client")
        child = pexpect.spawn(client_exec)
    except pexpect.ExceptionPexpect,e:
        println("Error starting client:"+e)
        die()
    
    child.logfile = sys.stdout
    s = child.sendline
    
    try:
        #child.expect("Trying 127.0.0.1...")
        #child.expect("Connected to localhost.")
        #child.expect("Escape character is '^]'.")
        #child.expect("Welcome to Sropulpof!")
        #child.expect("*")
        #child.expect("pof: ")
        index = child.expect(['pof: ', pexpect.EOF, pexpect.TIMEOUT])
        if index == 0:
            println("got the prompt as expected")
            pass
        elif index == 1:
            println("Unexpected EOF")
        elif index == 2:
            println("TIMEOUT")
            die()
    except Exception,e:
        println("Error:"+str(e))
        #p("debug information:")
        #p(str(child))
    
    # get list of contacts from the address book
    s("c -l")
    
