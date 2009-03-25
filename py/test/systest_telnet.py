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

pexpect expected strings are regular expression. See the re module.
""" 
import unittest
import pexpect
import os
import time
import sys
import commands
import tempfile

# ---------------------------------------------------------------------
# config 
server_port = "14444"
server_command = os.path.expanduser("./miville.py")
#server_exec = os.path.expanduser("nc -l -p server_port")
client_command = 'telnet localhost %s' % server_port
waiting_delay = 1.0 # seconds before starting client after server start

#VERBOSE_CLIENT = False
VERBOSE_CLIENT = True

#VERBOSE_SERVER = False
VERBOSE_SERVER = True


TMP_NAME = tempfile.mktemp()

if len(sys.argv) == 3:
    global server_port
    address = sys.argv[2]
    client_command = 'telnet  %s %s ' %  (address, server_port)
    print "CLIENT COMMAND: ", client_command


def bash_it(cmd):
    status = commands.getstatusoutput(cmd)
    output = status[1]
    print
    print
    print cmd
    lines = output.split("\n")
    for line in lines:
        if line.find(cmd) == -1:
            print line
    print
    print

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
        #sys.stdout.write(getColor('CYAN'))
        for s in self.buffer:
            sys.stdout.write(s)
        #sys.stdout.write(getColor())
        sys.stdout.flush()
        self.buffer = []

# ---------------------------------------------------------------------
# functions
def println(s, endl=True):
    """
    Prints a line to standard output with a prefix.
    """
    print ">>>>", s, # note the comma (",") at end of line

def start_process(command, isVerbose=False, logPrefix=''):
    """
    Command is a string to execute
    
    Returns a pexpect.spawn object
    """
    try:
        directory = os.getcwd()
        println('\nCurrent working dir: ' + directory)
                
        if isVerbose:
            bash_it( "ps aux |grep miville")
            println('\nStarting \"%s\"' % command)
            process = pexpect.spawn(command, logfile=ProcessOutputLogger(logPrefix))
        else:
            process = pexpect.spawn(command)
            time.sleep(waiting_delay) # seconds
        if (is_running(process) == False):
            die()
        else:
            return process
    except pexpect.ExceptionPexpect, e:
        println("Error starting client: " + str(e))
        die()

def is_running(process):
    """
    Process is a pexpect.spawn object
    
    Returns boolean
    """
    if (process.isalive() == False):
        status = "no status available" 
        if client:
            status = client.status
        println("Error starting server: %s" % status)
        return False
    else:
        return process

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
    #return "\x1b[" + s
    return ""

def kill_process(process):
    """
    Kills a pexpect.spawn object
    
    See kill -l for flags
    """
    try:
        if process != None:
            if (is_running(process) == True):
                process.kill(15)
                time.sleep(2)
                if (is_running(process) == True):
                    process.kill(9)
    except Exception, e:
        print "Error killing process", e
    
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
    os.environ['HOME'] = '/var/tmp/%s' % (TMP_NAME)
    os.remove('/var/tmp/%s/.sropulpof/sropulpof.adb' % (TMP_NAME))
except Exception, e:
    println("Warning removing old sropulpof.adb or setting HOME to /var/tmp/%s : %s" % (TMP_NAME, str(e)))

# TODO: Fix the process.logfile not getting to sys.stdout
# TODO: If the test fails, check if client and server are still running.

server = start_process(server_command, VERBOSE_SERVER, "SERVER> ")
time.sleep(2)
client = start_process(client_command, VERBOSE_CLIENT, "CLIENT> ")

# ---------------------------------------------------------------------
# System test classes
class TelnetBaseTest(unittest.TestCase):
    """
    Telnet system test case parent class
    """
    def setUp(self):
        """
        Starts a Telnet client for tests.
        """
        global client
        self.client = client
        self.sleep()
        try:
            client.logfile.real_flush()
            server.logfile.real_flush()
        except:
            pass
    def tearDown(self):
        """
        Destructor for each test. 
        """
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

    def expectTest(self, expected, message, timeout=2):
        """
        Fails a test if the expected regex value is not read from the client output.
        
        The expected value can be a string that is compiled to a regular expression (re)
        or the name of a Exception class.
        
        Succeeds otherwise.
        """
        # other listed expectations are child classes of Exception
        index = self.client.expect([expected, pexpect.EOF, pexpect.TIMEOUT], timeout=2) # 2 seconds max
        self.evalTest(index, message)



# ---------------------------------------------------------------------
# test classes
class Test_0_cli(TelnetBaseTest):
    """
    General tests for the CLI
    """
    def test_01_unknown_command(self):
        self.sleep()
        self.client.sendline('qwetyqrwetyqwertye')
        self.sleep()
        self.expectTest('command not found', 'Unknown command didn\'t give error.')
        

class Test_1_AddressBook(TelnetBaseTest):
    """
    Systems tests for the Address Book
    """
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
        self.expectTest('Contact deleted', 'Error while trying to erase contact')

    def test_99_delete_invalid_contact(self):
        self.client.sendline("c -e some_invalid_name")
        self.sleep()
        self.expectTest('Could not delete', 'There should be no contact with that name.')

class Test_2_Audiostream(TelnetBaseTest):
    """
    System Tests for Audiostream
    """
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

#    def test_11_change_input(self):
#        self.client.sendline("a -i jackaudiosrc audio")
#        self.sleep()
#        self.expectTest('source of audio stream audio is set to jackaudiosrc.', 'Unable to specify audio input.')
#    
#    def test_12_change_codec_of_invalid_stream(self):
#        self.client.sendline("a -c vorbis audiostream")
#        self.sleep()
#        self.expectTest('There\'s no audio stream with the name audiostream', 'IP Port of audio stream audio was not set.')
    
    def test_13_set_samplerate_to_invalid_value(self):
        self.client.sendline("a -r Juliette audio")
        self.sleep()
        self.expectTest('option -r: invalid integer value: u\'Juliette\'', 'Invalid value accepted as sample rate')
        #self.expectTest(AssertionError,"OK")
        
    def test_14_invalid_audio_stream_and_codec_order(self):
        """
        pof: a -c toto qweqweqwe (set codec to toto, but qweqweqwe is not a valid audio stream)
        There's no audio stream with the name qweqweqwe
        """
        self.client.sendline("a -c audio qweqweqwe")
        self.sleep()
        self.expectTest('There\'s no audio stream with the name qweqweqwe', 'There should be no audio stream with that name.')
        
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
    
    
        
class Test_3_Videostream(TelnetBaseTest):
    """
    System Tests for Videostream
    """
    def test_00_default_prompt(self):
        self.expectTest('pof: ', 'The default prompt is not appearing.')

    def test_01_add_videostream(self):
        self.sleep()
        self.client.sendline("v -a videostream")
        self.sleep()
        self.expectTest('Video stream videostream created.', 'Video stream video cannot be created.')

    def test_02_rename_videostream(self):
        self.client.sendline("v -m video videostream")
        self.sleep()
        self.expectTest('Video stream videostream rename to video.', 'Video stream videostream cannot be renamed to video.')

    def test_03_change_container(self):
        self.client.sendline("v -t mpegts video")
        self.sleep()
        self.expectTest('container of video stream video is set to mpegts.', 'Container of video stream video was not set to mpegts.')

    def test_04_change_codec(self):
        self.client.sendline("v -c vorbis video")
        self.sleep()
        self.expectTest('codec of video stream video is set to vorbis.', 'Codec of video stream video was not set to vorbis.')

    def test_05_change_settings(self):
        self.client.sendline("v -s value1:key1,value2:key2 video")
        self.sleep()
        self.expectTest('codec_settings of video stream video is set to value1:key1,value2:key2.', 'Codec settings of video stream video was not set to new values.')

    def test_06_change_width(self):
        self.client.sendline("v -w 640 video")
        self.sleep()
        self.expectTest('width of video stream video is set to 640.', 'Bitdepth of video stream video was not set to 24.')

    def test_07_change_height(self):
        self.client.sendline("v -r 480 video")
        self.sleep()
        self.expectTest('height of video stream video is set to 480.', 'Sample rate of video stream video was not set to 44100.')

    def test_09_change_port(self):
        self.client.sendline("v -p 5050 video")
        self.sleep()
        self.expectTest('port of video stream video is set to 5050.', 'IP Port of video stream video was not set.')

    def test_10_change_buffer(self):
        self.client.sendline("v -b 100 video")
        self.sleep()
        self.expectTest('buffer of video stream video is set to 100.', 'Buffer time of video stream video was not set to 44100.')

    def test_11_change_input(self):
        self.client.sendline("v -i v4l2src:location=/dev/video0 video")
        self.sleep()
        self.expectTest('source of video stream video is set to v4l2src:location="/dev/video0".', 'Unable to specify video input.')
    
#    def test_12_change_codec_of_invalid_stream(self):
#        self.client.sendline("v -c vorbis videostream")
#        self.sleep()
#        self.expectTest('There\'s no video stream with the name videostream', 'IP Port of video stream video was not set.')

    # NOTE: -z flag is not for changing description !
    #def test_13_change_description(self):
    #    self.client.sendline("v -z 'A Description' video")
    #    self.sleep()
    #    self.expectTest('source of video stream video is set to jackvideosrc.', 'Unable to specify video input.')
    
    def test_14_set_samplerate_to_invalid_value(self):
        self.client.sendline("v -r Juliette video")
        self.sleep()
        self.expectTest('option -r: invalid integer value: u\'Juliette\'', 'Invalid value accepted as sample rate')
        
    def test_50_list_videostreams(self):
        self.client.sendline("v -l")
        self.sleep()
        self.expectTest('video', 'The list of video streams cannot be obained.')
    
    def test_98_delete_videostream(self):
        self.client.sendline("v -e video")
        self.sleep()
        self.expectTest('Video stream video deleted.', 'Video stream video cannot be deleted.')

    def test_99_delete_invalid_videostream(self):
        self.client.sendline("v -e videostream")
        self.sleep()
        self.expectTest('There\'s no video stream with the name videostream', 'There should be no video stream with that name.')
        
class Test_004_Settings(TelnetBaseTest):
    """
    System Tests for presets
    """
    
    
    def tst(self, command, expected, errorMsg = None):
        self.client.sendline(command)
        self.sleep()
        err = errorMsg or 'The command did not return: "%s" as expected' % expected
        self.expectTest(expected, err)
        
    def test_00_yes(self):
        self.expectTest('pof: ', 'The default prompt is not appearing.')
    
    
    
    def test_01_GlobalPresets(self):
        """
        test that preset file exist.
        look for a known preset
        check that you can't delete a preset
        
        Assign a preset to a contact
        """
        
        # os.path.expanduser("~")+"/.sropulpof/sropulpof.preset"
        path = "/usr/share/sropulpof/globalSettings.presets"
        self.assertTrue(os.path.exists(path), 'Global Preset file "%s" is missing' % path)

    def test_02_MediaPresets(self):    
        """
        test that preset file exist.
        look for a known preset
        check that you can't delete a preset
        """
        path = "/usr/share/sropulpof/mediaSettings.presets"
        self.assertTrue(os.path.exists(path), 'Global Preset file "%s" is missing' % path)
        
        
    def test_02_addSettings(self):
        
        self.tst('list settings', 'There are 5 preset settings', 'The preset files are missing.')
 
class Test_5_Devices(TelnetBaseTest):
    """
    System Tests for devices.

    devices -k video -l
    devices -k video -t v4l2 -d /dev/video0 -a
    devices -k video -t v4l2 -d /dev/video0 -m norm pal
    devices -k video -t v4l2 -d /dev/video0 -a
    devices -k video -t v4l2 -d /dev/video0 -m norm ntsc
    devices -k video -t v4l2 -d /dev/video0 -a
    """
    def test_01_list_v4l2_devices(self):
        self.sleep()
        self.client.sendline("devices -k video -l")
        self.sleep()
        self.expectTest('/dev/video0.', 'Warning: no v4l2 device appearing in the CLI.')

    def test_02_modify_v4l2_attribute(self):
        self.client.sendline("devices -k video -t v4l2 -d /dev/video0 -a")
        self.sleep()
        self.expectTest('norm', 'v4l2 device doesn\'t have a norm attribute.')
       
        self.client.sendline("devices -k video -t v4l2 -d /dev/video0 -m norm pal")
        self.sleep()
        self.client.sendline("devices -k video -t v4l2 -d /dev/video0 -a")
        self.sleep()
        self.expectTest('PAL', 'Could not modify device attribute.')
       
        self.client.sendline("devices -k video -t v4l2 -d /dev/video0 -m norm ntsc")
        self.sleep()
        self.client.sendline("devices -k video -t v4l2 -d /dev/video0 -a")
        self.sleep()
        self.expectTest('NTSC', 'Could not modify device attribute.')


   
