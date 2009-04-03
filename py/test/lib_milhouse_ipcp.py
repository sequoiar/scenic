#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Miville
# Copyright (C) 2008 Société des arts technoligiques (SAT)
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
System test for the GST telnet UI.

Historical note: Nelson is Milhouse's bully.
His purpose it to abuse Milhouse into submission. 

Usage: create a class that inherits from Nelson, 
       and test the hell out of Milhouse. Nelson provides:
         - one call milhouse and telnet process creation
         - tst method to send milhouse a command and verify an output
         - UML diagram creation to discuss around meetings

        run your test class with trial: trial test/my_test.py

   remember: pexpect expected strings are regular expression. 
             See the re module.
""" 
import unittest
import pexpect
import os
import time
import sys
import inspect
import repr

import commands

# global constants

EXECUTABLE = "milhouse" 
# used to be "propulseart"
#video_src = "videotestsrc"
video_src = "videotestsrc"
audio_src = 'audiotestsrc'



# ---------------------------------------------------------------------
# config 

waiting_delay = 1.0 # seconds before starting client after server start


def verb(to_print=''):
    #if VERBOSE_CLIENT:
    print to_print

def msc2png(test_name):
    cmd = "mscgen -T png -i %s.msc -o %s.png" % (test_name, test_name)
    status = commands.getstatusoutput(cmd)
    output = status[1]
    

def generate_html(test_class):    
    members = dir(test_class)
    title = test_class.__name__
    body = ''
    name = ''
    
    members= dir(test_class)
    for member_name in members:
        if member_name.startswith('test'):
            member = getattr(test_class, member_name)
            description = member.__doc__
            image = member_name + ".png"
            body += """<h2>%s</h2>
                    <p>%s</p>
                    <img src='%s' align='middle' />
                    <p><a href="%s.msc">Sequence diagram source</a>
                    <h3>Telnet logs</h3>
                    <p><a href="%s_telnet_tx.txt">sender telnet log</a>
                    <p><a href="%s_telnet_rx.txt">receiver telnet log</a>
                    <h3>Server terminal logs (YMMV)</h3>
                    <p><a href="%s_rx.txt">receiver log</a>
                    <p><a href="%s_tx.txt">sender log</a>
                    """ % (member_name, description, image,member_name,member_name,member_name,member_name,member_name)  
            msc2png(member_name)
    description = test_class.__doc__
    html = """<html>
    <head>
    <title>%s</title>
    </head>
    <body>
    <h1>%s</h1>
    <p>%s</p>
    %s
    </body>
    </html>
    """ % (title, title, description, body)
    return html  

# ---------------------------------------------------------------------
# a class for output redirection

def start_process(command, logfile=None):
    """
    Command is a string to execute
    
    Returns a pexpect.spawn object
    """
    display = os.environ['DISPLAY']
    directory = os.getcwd()
    try:

        #print 'Current working dir: ' + directory
        #if isVerbose:
        print 'starting process "%s"' % (command)
        process = pexpect.spawn(command, logfile=logfile)
        #process.logfile=sys.stdout
        #if logfile != None:
        #process.logfile=logfile
        
            
        time.sleep(waiting_delay) # seconds
        if (process.isalive() == False):
            print "Error starting client '%s': not alive yet :-(" % command
            process.kill(9)
        else:
            return process
    except pexpect.ExceptionPexpect, e:
        print "Error starting client: " + str(e)



def kill_process(process):
    """
    Kills a pexpect.spawn object
    
    See kill -l for flags
    """
    if process != None:
        try:
            if (process.isalive() == True):
                process.kill(15)
                time.sleep(1)
                if (process.isalive() == True):
                    process.kill(9)
        except Exception, e:
            print "Error killing process", e
        



# ---------------------------------------------------------------------
# System test classes
class Nelson(unittest.TestCase):
    
    def __init__(self, *argvs):
        unittest.TestCase.__init__(self, *argvs )
        self.tx_telnet = None
        self.tx_server = None
        self.rx_telnet = None
        self.rx_server = None
        
        self.rx_server_log = None
        self.tx_server_log = None
        self.rx_telnet_log = None
        self.tx_telnet_log = None
        
        msc_string = ""
    
    def _get_start_command(self, codec, port, address, 
                           bitrate=None, 
                           source =None, 
                           channels=None,
                           audio_buffer_usec=None):
         s = ' codec="%s" port=%d address="%s"' %  (codec, port, address)
         if bitrate:
             s += ' bitrate=%d' % bitrate
         if source:
             s += ' source="%s"' % source
         if channels:
             s += ' channels=%d' % channels    
         return s
    

   
    def stream_duration(self, delay):
        """
        Adds a line to the sequence diagram
        """ 
        time.sleep(delay)
        #self.msc += 'test:>test [label="waiting"];\n' % str(delay)
        self.msc += '---  [ label = "streaming data for %s seconds"]; \n' % str(delay)     
        
    """
    Telnet system test case parent class
    """
    def setUp(self):
        self.tx_telnet = None
        self.tx_server = None
        self.tx_telnet_log = None
        self.tx_server_log= None
        
        self.rx_telnet = None
        self.rx_server = None    
        self.rx_telnet_log = None
        self.rx_server_log= None
        
        self.msc = """
msc
{        
    test, telnet_tx, telnet_rx, gst_tx, gst_rx;
    
"""
        
        
    
    def _save_string_to_file(self, string_to_save, short_file_name):
        filepath = os.path.realpath(short_file_name)
        f = open(filepath , 'w')
        try:    
            f.write(string_to_save)
        finally:
            f.close()
            verb(  filepath + " saved...")
        
    def tearDown(self):
        """
        Destructor for each test. 
        """
        try:
            self._pump_up_the_files()
        except:
            pass
            
        self.msc += '\n}'
        short_dia_file_name = self._testMethodName+".msc"
        self._save_string_to_file(self.msc,short_dia_file_name)
        index = generate_html(self.__class__)
        self._save_string_to_file(index, 'index.html')
        
        
        command = "quit:"
                      
        if self.tx_telnet != None:
            self.tx_telnet.sendline(command)
            kill_process(self.tx_telnet)
             
        if self.tx_server != None:
            kill_process(self.tx_server)
                      
        if self.rx_telnet != None:
            self.rx_telnet.sendline(command)
            kill_process(self.rx_telnet)
             
        if self.rx_server != None:
            kill_process(self.tx_server)  
        
        if self.rx_server_log != None:
            self.rx_server_log.close()

        if self.tx_server_log != None:
            self.tx_server_log.close()
        
        if self.rx_telnet_log != None:
            self.rx_telnet_log.close()
            
        if self.tx_telnet_log != None:
            self.tx_telnet_log.close()

    def _evalTest(self, index, message):
        """
        Fails a test, displaying a message, if the provided index resulting 
        from expect() matches some of the indices provided by expectTest()
        """
        self.assertEqual(index, 0, message)
        self.failIfEqual(index, 1, 'Problem : Unexpected EOF')
        self.failIfEqual(index, 2, 'Problem : Time out.')

    def _expectTest(self, client, expected, message):
        """
        Fails a test if the expected regex value is not read from the client output.
        
        The expected value can be a string that is compiled to a regular expression (re)
        or the name of a Exception class.
        
        Succeeds otherwise.
        """
        # other listed expectations are child classes of Exception
        index = client.expect([expected, pexpect.EOF, pexpect.TIMEOUT], timeout=2) # 2 seconds max
        self._evalTest(index, message)
    
    def _start_propulseart(self, mode, port, serv_name, tel_name, server_log, telnet_log):
        global EXECUTABLE 
        server_command = "%s -%s --serverport %d" % (EXECUTABLE,  mode, port) 
        client_command = 'telnet localhost %d' % port
        server = start_process(server_command, server_log)
        time.sleep(1.)
        telnet = start_process(client_command,telnet_log )
        self.msc += 'test:>%s [label="$> %s"];\n' % (serv_name, server_command)
        self.msc += 'test:>%s [label="$> %s"];\n' % (tel_name, client_command)
        return (server, telnet)
        
    def start_propulseart_rx(self, port):
        server_logfilename = os.path.realpath(self._testMethodName + "_rx.txt")
        self.rx_server_log = open(server_logfilename , 'w')
        telnet_logfilename = os.path.realpath(self._testMethodName + "_telnet_rx.txt")
        self.rx_telnet_log = open(telnet_logfilename , 'w')
        
        (server, telnet) = self._start_propulseart('r', port, 'gst_rx', 'telnet_rx', self.rx_server_log, self.rx_telnet_log)
        self.rx_telnet = telnet
        self.rx_server = server
        
    def start_propulseart_tx(self, port):
        logfilename = os.path.realpath(self._testMethodName + "_tx.txt")
        self.tx_server_log = open(logfilename , 'w')
        telnet_logfilename = os.path.realpath(self._testMethodName + "_telnet_tx.txt")
        self.tx_telnet_log = open(telnet_logfilename , 'w')       
        
        (server, telnet) = self._start_propulseart('s', port, 'gst_tx', 'telnet_tx', self.tx_server_log, self.tx_telnet_log)
        self.tx_telnet = telnet
        self.tx_server = server
    
    def _add_sequence(self, source, destination, message):
        ping = message.replace('"','\\"')
        self.msc += '%s=>%s [label="%s"];\n' % (source, destination, ping)
   
    def _add_sequence_response(self, source, destination, message):
        ping = message.replace('"','\\"')
        self.msc += '%s>>%s [label="%s"];\n' % (source, destination, ping)
    
    def _pump_up_the_files(self):
        def pump_me(what):
            if what != None:
                #what.read_nonblocking(size=10000, timeout=1)
                what.readline()
                pass
            
        pump_me(self.tx_server)
        pump_me(self.rx_server)
        #pump_me(self.tx_telnet)
        #pump_me(self.rx_telnet)    
            
         
            
            
    def tst_tx(self,command, expected, errorMsg = None):
        self.assertNotEqual(self.tx_telnet, None, "Sender telnet process is None!")
        self._tst(self.tx_telnet,command, expected, errorMsg)
        #self._add_sequence('test', 'telnet_tx', command)
        self._add_sequence('telnet_tx', 'gst_tx', command)
        self._add_sequence_response('gst_tx', 'telnet_tx', expected)
        
          
    def tst_rx(self,command, expected, errorMsg = None):
        self.assertNotEqual(self.rx_telnet, None, "Receiver telnet process is None!")
        self._tst(self.rx_telnet, command, expected, errorMsg)
        #self._add_sequence('test', 'telnet_rx', command)
        self._add_sequence('telnet_rx', 'gst_rx', command)
        self._add_sequence_response('gst_rx', 'telnet_rx', expected)
        
         
    def _tst(self, client, command, expected, errorMsg = None):
        #self._pump_up_the_files()
        client.sendline(command)
        time.sleep(0.025)
        self._pump_up_the_files()
        err = errorMsg or 'The command did not return: "%s" as expected' % expected
        self._expectTest(client, expected, err)
        
    
    def verb(self,msg=''):
        verb(msg)

def prompt(str, prompt = ">"):
    return prompt + str + ''

# ---------------------------------------------------------------------




  
