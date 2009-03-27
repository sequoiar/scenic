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
System test for the GST telnet UI.

Usage: trial test/systest_telnet.py

pexpect expected strings are regular expression. See the re module.
""" 
import unittest
import pexpect
import os
import time
import sys
import inspect

import repr

# global constants

EXECUTABLE = "milhouse" 
# used to be "propulseart"
#video_src = "videotestsrc"
video_src = "videotestsrc"

audio_src = 'audiotestsrc'
#audio_src = 'alsrc'

video_init_ok = 'video_init: ack="ok"'
audio_init_ok = 'audio_init: ack="ok"'
start_ok = 'start: ack="ok"'
stop_ok =  'stop: ack="ok"'
stop_but_not_ok =  ''


# ---------------------------------------------------------------------
# config 

waiting_delay = 1.0 # seconds before starting client after server start

VERBOSE_CLIENT = False
#VERBOSE_CLIENT = True

VERBOSE_SERVER = False
#VERBOSE_SERVER = True

def verb(to_print=''):
    #if VERBOSE_CLIENT:
    print to_print

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

def start_process(command, isVerbose=False, logPrefix=''):
    """
    Command is a string to execute
    
    Returns a pexpect.spawn object
    """
    try:
        directory = os.getcwd()
	display = os.environ['DISPLAY']
	# print 'Current DISPLAY: ' + display
        #print 'Current working dir: ' + directory
        if isVerbose:
            print
            print 'starting process "$>%s"' % command
            print
            process = pexpect.spawn(command, logfile=ProcessOutputLogger(logPrefix))
        else:
            process = pexpect.spawn(command)
        time.sleep(waiting_delay) # seconds
        if (process.isalive() == False):
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
                time.sleep(2)
                if (process.isalive() == True):
                    process.kill(9)
        except Exception, e:
            print "Error killing process", e
        



# ---------------------------------------------------------------------
# System test classes
class TelnetBaseTest(unittest.TestCase):
    
    def __init__(self, *argvs):
        unittest.TestCase.__init__(self, *argvs )
        self.tx_telnet = None
        self.tx_server = None
        self.rx_telnet = None
        self.rx_server = None
        
        msc_string = ""
         
        
    """
    Telnet system test case parent class
    """
    def setUp(self):
        
        self.msc = """
msc
{        
    test, telnet_tx, telnet_rx, gst_tx, gst_rx;
    
"""
        
    
    def tearDown(self):
        """
        Destructor for each test. 
        """
        self.msc += '\n}'
        
        filepath = os.path.realpath(self._testMethodName+".msc")
        print "Writing diagram " + filepath
        f = open(filepath , 'w')
        try:    
            f.write(self.msc)
        finally:
            f.close()
        
        
        command = "quit:"
        
        if self.tx_telnet != None:
            self.tx_telnet.sendline(command)
            time.sleep(0.5)
            tmp = self.tx_telnet
            self.tx_telnet = None
            kill_process(tmp)
             
        if self.tx_server != None:
            tmp = self.tx_server
            self.tx_server = None
            kill_process(tmp)  
                      
        if self.rx_telnet != None:
            self.rx_telnet.sendline(command)
            time.sleep(0.5)            
            tmp = self.rx_telnet
            self.rx_telnet = None
            kill_process(tmp)
             
        if self.rx_server != None:
            tmp = self.tx_server
            self.rx_server = None
            kill_process(tmp)  




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
    
    def _start_propulseart(self, mode, port, serv_name, tel_name):
        global EXECUTABLE 
        server_command = "%s -%s --serverport %d" % (EXECUTABLE,  mode, port) 
        client_command = 'telnet localhost %d' % port
        server = start_process(server_command, VERBOSE_SERVER, "SERVER> ")
        time.sleep(2)
        telnet = start_process(client_command, VERBOSE_CLIENT, "CLIENT> ")
        self.msc += 'test:>%s [label="$> %s"];\n' % (serv_name, server_command)
        self.msc += 'test:>%s [label="$> %s"];\n' % (tel_name, client_command)
        return (server, telnet)
        
    def start_propulseart_rx(self, port):
        (server, telnet) = self._start_propulseart('r', port, 'gst_rx', 'telnet_rx')
        self.rx_telnet = telnet
        self.rx_server = server
        
    def start_propulseart_tx(self, port):
        (server, telnet) = self._start_propulseart('s', port, 'gst_tx', 'telnet_tx')
        self.tx_telnet = telnet
        self.tx_server = server
    
    def _add_sequence(self, source, destination, message):
        ping = message.replace('"','\\"')
        self.msc += '%s=>%s [label="%s"];\n' % (source, destination, ping)
   
    def _add_sequence_response(self, source, destination, message):
        ping = message.replace('"','\\"')
        self.msc += '%s>>%s [label="%s"];\n' % (source, destination, ping)
            
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
        client.sendline(command)
        time.sleep(0.025)
        err = errorMsg or 'The command did not return: "%s" as expected' % expected
        self._expectTest(client, expected, err)
    

def prompt(str, prompt = ">"):
    return prompt + str + ''



# ---------------------------------------------------------------------



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
                    """ % (member_name, description, image)  
  
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
  

    
class Test_telnet_milhouse(TelnetBaseTest):
    """
    System test telnet propulseart:
    Runs a series of streaming tests with propulseart.
    The tests starts and stop various propulseart processes in sender and receiver mode,
    and controls them via a telnet protocol.  
    """
    
        
    def _get_start_command(self, codec, port, address, bitrate=None, source =None, channels=None):
         s = ' codec="%s" port=%d address="%s"' %  (codec, port, address)
         if bitrate:
             s += ' bitrate=%d' % bitrate
         if source:
             s += ' source="%s"' % source
         if channels:
             s += ' channels=%d' % channels    
         return s
    
#    def test_01_generate_html_index(self):
#        generate_html(self)
#    
    def test_02_simple_video_sender(self):
        """
        create a video sender process that waits for connections. The 
        process is controlled from port 1220 and its videostream is set on port
        12020 
        The port numbers are printed for debuggind convenience.
        The process is then stopped without any streaming.
        
        This tests that the process exists and responds to basic commands.
        """
        
        telnet_tx_port = 1220
        stream_port = 12020 
        bitrate = 3000000
        tx_init_msg = "video_init:" + self._get_start_command('h264', stream_port, "127.0.0.1", bitrate=bitrate, source=video_src)
        verb('')
        verb( tx_init_msg) 
        # start process and a telnet on port 1200 for control
        self.start_propulseart_tx(telnet_tx_port) 
        # start a sender on port 12000
        self.tst_tx(tx_init_msg, video_init_ok)
        self.tst_tx("start:", start_ok) # start streaming
        time.sleep(1.)
        self.tst_tx("stop:", stop_ok)
        
    def test_03_simple_video_receiver(self):
        """
        Same as test2, but this time the process is started in receiver mode
        """
        telnet_rx_port = 1230
        stream_port = 12030 
        rx_init_msg = "video_init:" + self._get_start_command('h264', stream_port, "127.0.0.1")
        verb('')
        verb(rx_init_msg)
        self.start_propulseart_rx(telnet_rx_port)
        
        self.tst_rx(rx_init_msg, video_init_ok)
        self.tst_rx("start:", start_ok) # start streaming
        
        time.sleep(2.)
        
        self.tst_rx("stop:", stop_ok)
    
    def _stream_dream(self, delay):
        """
        Adds a line to thje sequence diagram
        """ 
        time.sleep(delay)
        #self.msc += 'test:>test [label="waiting"];\n' % str(delay)
        self.msc += '---  [ label = "streaming data for %s seconds"]; \n' % str(delay)
    
    def test_04_video_sender_receiver(self):
        """
        Finally, we get some streaming going: a receiver and a sender are started and stopped.
        During the 5 second sleep, the video stream should appear on the screen.
        
        This test should check for actual transfered data when the command is available.
        """
        verb('')
        
        
        telnet_rx_port = 1240
        rx_init_command = "video_init:" + self._get_start_command('h264', 12040, "127.0.0.1")
        verb("RX> " + rx_init_command)
        
        bitrate = 3000000
        telnet_tx_port = 1340
        tx_init_command = "video_init:" + self._get_start_command('h264', 12040, "127.0.0.1", bitrate=bitrate,source=video_src)  
        verb( "TX> " + tx_init_command)
        
        self.start_propulseart_rx(telnet_rx_port)
        self.start_propulseart_tx(telnet_tx_port)
        
        self.tst_rx( rx_init_command, video_init_ok )
        self.tst_tx( tx_init_command, video_init_ok )
        
        self.tst_rx("start:", start_ok)
        self.tst_tx("start:", start_ok)
        self._stream_dream(5.)
        # check for transfered data
        # should be well above 0 bytes at this point
        
        self.tst_tx("stop:", stop_ok)
        self.tst_rx("stop:", stop_but_not_ok)
        
    def test_05_audio_send_receiv(self):
        """
        Streaming audio test betweeen 2 instances of propulseart.
        """
        
        verb('')
        telnet_rx_port = 1250
        telnet_tx_port = 1350
        
        rx_init_cmd = "audio_init:" + self._get_start_command('vorbis', 12005, "127.0.0.1")
        verb( "RX:>" + rx_init_cmd)
        tx_init_cmd =  "audio_init:" + self._get_start_command ('vorbis', 12005, "127.0.0.1", source=audio_src, channels=2)
        verb( "TX:>" + tx_init_cmd)
        self.start_propulseart_rx(telnet_rx_port)
        self.start_propulseart_tx(telnet_tx_port)
        # self.tst_rx( self.get_audio_start(12000) , 'audio_start audio_start()' )
        self.tst_rx(rx_init_cmd, audio_init_ok )
        self.tst_tx(tx_init_cmd, audio_init_ok)
        # make it so
        self.tst_rx("start:", start_ok)
        self.tst_tx("start:", start_ok)
        # check for transfered data
        # should be well above 0 bytes at this point
        
        verb("DATA is flowing... or is it?")
        self._stream_dream(5.)
        # cleanup and go home
        verb("stopping the streaming...")
 
 
        self.tst_tx("stop:", stop_but_not_ok) # CARAMBA!
        self.tst_rx("stop:", stop_ok)
        
 
        # check for data
        
    def test_audio_video_synchronized(self): 
        """
        This test is a streaming test where audio and video are streamed 
        simultaneously with synchronisation.
        The commands for this test are not implemented yet.
        """
        verb('')
        telnet_rx_port = 1270
        telnet_tx_port = 1370
        
        rx_video_init_cmd = "video_init:" + self._get_start_command('mpeg4',   12007, "127.0.0.1")
        rx_audio_init_cmd = "audio_init:" + self._get_start_command('raw', 12107, "127.0.0.1", channels =2)        
        verb( 'RX video: ' + prompt(rx_video_init_cmd) )
        verb( 'RX audio: ' + prompt(rx_audio_init_cmd) )
        
        bitrate = 3000000
        tx_video_init_cmd = "video_init:" + self._get_start_command('mpeg4',   12007, "127.0.0.1", bitrate=bitrate, source=video_src)
        tx_audio_init_cmd = "audio_init:" + self._get_start_command('raw', 12107, "127.0.0.1", source=audio_src, channels =2)   
        verb(  'TX video: ' + prompt(tx_video_init_cmd))
        verb( 'TX audio: ' + prompt(tx_audio_init_cmd))
                 
        self.start_propulseart_rx(telnet_rx_port)
        self.start_propulseart_tx(telnet_tx_port)
        

        self.tst_rx(rx_video_init_cmd , video_init_ok )
        self.tst_rx(rx_audio_init_cmd , audio_init_ok  )
        
        
        self.tst_tx(tx_video_init_cmd, video_init_ok)
        self.tst_tx(tx_audio_init_cmd , audio_init_ok)
                
        self.tst_tx( 'start:', start_ok )
        time.sleep(1)
        self.tst_rx( 'start:', start_ok )
       
        self._stream_dream(5.)
        # check number of packets TBD
        self.tst_rx("stop:", stop_but_not_ok) # CARAMBA!!!
        self.tst_tx("stop:", stop_ok)
       
       
#index = generate_html(Test_telnet_milhouse)

#print index
#index_file = os.path.realpath('index.html')
#f = open(index_file,'w')
#f.write(index)
#f.close() 
#verb(  "Index.html saved to> " + index_file)        
