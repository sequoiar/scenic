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

Usage: trial test/systest_telnet.py
"""
import unittest
import pexpect
import os
import time
import sys
# ---------------------------------------------------------------------
# config 
server_port = "14444"
server_command = os.path.expanduser("./miville.py")
#server_exec = os.path.expanduser("nc -l -p server_port")
client_command = 'telnet localhost %s' % server_port
waiting_delay = 1.0 # seconds
#BE_VERBOSE = True 
BE_VERBOSE = False
# ---------------------------------------------------------------------
# functions
def println(s,endl=True):
    """
    Prints a line to standard output
    """
    if endl:
        print ">>>>",s
    else:
        print ">>>>",s, # note the comma (",") at end of line

def start_process(command):
    """
    command is a string to execute
    a pexpect object will be returned
    """
    try:
        println('Starting \"%s\"' % command )
        process = pexpect.spawn(command)
        if BE_VERBOSE:
            process.logfile = sys.stdout
        time.sleep(waiting_delay) # seconds
        if ( is_running(process) == False ):
            die()
        else:
            return process

    except pexpect.ExceptionPexpect,e:
        println("Error starting client: heh"+e)
        die()

def is_running(process):
    """
    process is a pexpect.spawn object
    """
    if ( process.isalive() == False ):
        println("Error starting server: %s" % client.status)
        return False
    else:
        return process

def kill_process(process):
    try:
        if (is_running(process) == True ):
            process.kill(15)
            sleep(2)
            if (is_running(process) == True ):
                process.kill(9)
    except Exception,e:
        print "Error killing process",e

def die():
    """
    Ends the programs with error flag.
    """
    global server, client
    println("EXITING")
    kill_process(client)
    kill_process(server)
    sys.exit(1)

# ---------------------------------------------------------------------
# startup poutine
# global variables for the telnet client and server child process
client = None
server = None

try:
    #delete ~/.sropulpof/sropulpof.adb
    #orig_home = os.environ['HOME']
    os.environ['HOME'] = '/var/tmp'
    os.remove('/var/tmp/.sropulpof/sropulpof.adb')
except Exception,e:
    println("Error removing old sropulpof.adb or setting HOME to /var/tmp."+str(e))

# TODO: Fix the process.logfile not getting to sys.stdout
# TODO: If the test fails, check if client and server are still running.


server = start_process(server_command)
client = start_process(client_command)

# ---------------------------------------------------------------------
# classes
class TelnetBaseTest(unittest.TestCase):
    """
    Telnet test case parent class
    """
    messages = {
        'prompt':"pof: ",
        'greeting':"Welcome to Sropulpof!",
        'not found':"command not found"
    }
    def setUp(self):
        """
        Starts a Telnet client for tests.
        """
        global client
        self.client = client
        self.sleep()


    def tearDown(self):
        """
        Destructor for each test. Nothing to do.
        """
        pass

    def sleep(sleep):
        """Waits a bit between each command."""
        time.sleep(0.025)

    def prepareTest(self, server, client):
        if ( is_running(server) == False ):
            server = start_process(server_command)
        if ( is_running(client) == False ):
            client = start_process(client_command)
        return(server, client)

    def evalTest(self, index, message):
        self.assertEqual(index, 0, message)
        self.failIfEqual(index, 1, 'Problem : Unexpected EOF')
        self.failIfEqual(index, 2, 'Problem : Time out.')

    def expectTest(self, expected, message):
        index = self.client.expect([expected, pexpect.EOF, pexpect.TIMEOUT], timeout=2)
        self.evalTest(index, message)

# ---------------------------------------------------------------------
# test classes
class Test_1_AddressBook(TelnetBaseTest):
    def test_01_default_prompt(self):
        self.expectTest('pof: ', 'The default prompt is not appearing.')

    def test_02_add_contact(self):
        # adding a contact
        # c -a name ip [port]
        self.client.sendline("c -a Juliette 154.123.2.3")
        self.sleep()
        self.expectTest('Contact added', 'Contact not added')
    def test_03_list(self):
        self.client.sendline("c -l") 
        self.sleep()
        self.expectTest('Juliette:', 'The contact that has just been added is not appearing.')

    def test_04_add_duplicate(self):
        self.client.sendline("c -a Juliette 192.168.20.20")
        self.sleep()
        self.expectTest('Could not add contact.', 'Double entry shouldn\'t have been made.')

    def test_05_select(self):
        self.client.sendline("c -s Juliette")
        self.sleep()
        self.expectTest('Contact selected', 'Contact couln\'t be selected')

    def test_06_duplicate_selected(self):
        self.client.sendline("c -d Henriette")
        self.sleep()
        self.expectTest('Contact duplicated', 'Selected contact cannot be duplicated')

    def test_07_duplicate_named(self):
        self.client.sendline("c -d Mariette Juliette")
        self.sleep()
        self.expectTest('Contact duplicated', 'Specified contact cannot be duplicated')

    def test_08_modify_selected_name(self):
        self.client.sendline("c -s Mariette")
        self.sleep()
        self.client.sendline("c -m Luciette")
        self.sleep()
        self.expectTest('Contact modified', 'Selected contact name cannot be modified')

    def test_08_modify_selected_ip(self):
        self.client.sendline("c -m address=172.16.20.30")
        self.sleep()
        self.expectTest('Contact modified', 'Selected contact name cannot be modified')

    def test_09_modify_selected_name_and_ip(self):
        self.client.sendline("c -m Annette 192.168.30.30")
        self.sleep()
        self.expectTest('Contact modified', 'Selected contact name cannot be modified')

    def test_98_erase_contact(self):
        self.client.sendline("c -e Juliette")
        self.sleep()
        self.expectTest('Contact deleted','Error while trying to erase contact')

    def test_99_delete_invalid_contact(self):
        self.client.sendline("c -e some_invalid_name")
        self.sleep()
        self.expectTest('Could not delete', 'There should be no contact with that name.')

class Test_2_Audiostream(TelnetBaseTest):
    def test_00_default_prompt(self):
        self.expectTest('pof: ', 'The default prompt is not appearing.')

    def test_01_add_audiostream(self):
        self.sleep()
        self.client.sendline("a -a audiostream")
        self.sleep()
        self.expectTest('Audio stream audiostream created.', 'Audio stream audio cannot be created.')

    def test_02_rename_audiostream(self):
        self.client.sendline("a -m audio audiostream")
        self.sleep()
        self.expectTest('Audio stream audiostream rename to audio.', 'Audio stream audiostream cannot be renamed to audio.')

    def test_03_change_container(self):
        self.client.sendline("a -t mpegts audio")
        self.sleep()
        self.expectTest('container of audio stream audio is set to mpegts.', 'Container of audio stream audio was not set to mpegts.')

    def test_04_change_codec(self):
        self.client.sendline("a -c vorbis audio")
        self.sleep()
        self.expectTest('codec of audio stream audio is set to vorbis.', 'Codec of audio stream audio was not set to vorbis.')

    def test_05_change_settings(self):
        self.client.sendline("a -s value1:key1,value2:key2 audio")
        self.sleep()
        self.expectTest('codec_settings of audio stream audio is set to value1:key1,value2:key2.', 'Codec settings of audio stream audio was not set to new values.')

    def test_06_change_bitdepth(self):
        self.client.sendline("a -d 24 audio")
        self.sleep()
        self.expectTest('bitdepth of audio stream audio is set to 24.', 'Bitdepth of audio stream audio was not set to 24.')

    def test_07_change_samplerate(self):
        self.client.sendline("a -r 44100 audio")
        self.sleep()
        self.expectTest('sample_rate of audio stream audio is set to 44100.', 'Sample rate of audio stream audio was not set to 44100.')

    def test_08_change_channels(self):
        self.client.sendline("a -v 8 audio")
        self.sleep()
        self.expectTest('channels of audio stream audio is set to 8.', 'Number of channels of audio stream audio was not set.')

    def test_09_change_port(self):
        self.client.sendline("a -p 5050 audio")
        self.sleep()
        self.expectTest('port of audio stream audio is set to 5050.', 'IP Port of audio stream audio was not set.')
    def test_10_change_buffer(self):
        self.client.sendline("a -b 100 audio")
        self.sleep()
        self.expectTest('buffer of audio stream audio is set to 100.', 'Buffer time of audio stream audio was not set to 44100.')
    def test_11_change_input(self):
        self.client.sendline("a -i jackaudiosrc audio")
        self.sleep()
        self.expectTest('source of audio stream audio is set to jackaudiosrc.', 'Unable to specify audio input.')
#    def test_12_change_codec_of_invalid_stream(self):
#        self.client.sendline("a -c vorbis audiostream")
#        self.sleep()
#        self.expectTest('There\'s no audio stream with the name audiostream', 'IP Port of audio stream audio was not set.')

    def test_13_set_samplerate_to_invalid_value(self):
        self.client.sendline("a -r Juliette audio")
        self.sleep()
        self.expectTest('Invalid value for sample rate. Sample rate should be integers.', 'Invalid value accepted as sample rate')
    def test_50_list_audiostreams(self):
        self.client.sendline("a -l")
        self.sleep()
        self.expectTest('audio', 'The list of audio streams cannot be obained.')
    def test_98_delete_audiostream(self):
        self.client.sendline("a -e audio")
        self.sleep()
        self.expectTest('Audio stream audio deleted.', 'Audio stream audio cannot be deleted.')

    def test_99_delete_invalid_audiostream(self):
        self.client.sendline("a -e audiostream")
        self.sleep()
        self.expectTest('There\'s no audio stream with the name audiostream', 'There should be no audio stream with that name.')
