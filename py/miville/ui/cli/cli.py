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
Command line user interface. 

Implements some view and controller of the MVC design pattern.

Each method of the CliController class that begins with "_" has a CLI 
command parser with some options. It parses what the user types. 
(the "controller")
It then calls a method in ui/api.py (the "model")
This finally answer by calling each of its Observer that listen to it. 
In this case, the "view" is the CliView class.

For example:

What happens when the user types "c -l":

 * CliController._get_contacts() is called with the args from user
 * it calls ControllerApi.get_contacts() from ui/api.py
 * which finally calls CliView._get_contacts() with a dict of contacts 
   as an argument. (the key are the contact names)
   It is the contacts attributes of the AddressBook instance.
   
"""

# System imports
import optparse
import re
import os
import socket

# Twisted imports
from twisted.internet import reactor, protocol
#from twisted.conch import telnet
try:
    from twisted.conch import recvline
    from twisted.conch.insults import insults
    from twisted.conch.telnet import TelnetTransport, TelnetBootstrapProtocol
except ImportError:
    raise ImportError, 'If you want to use the Telnet interface, you need to install twisted.conch.'

#App imports
from miville.utils import Observer, log # also imports Observer and Subject from miville.utils/observer.py

from miville.utils.i18n import to_utf
from miville.utils.common import find_callbacks
# from streams.stream import AudioStream, VideoStream, DataStream


log = log.start('info', 1, 0, 'cli')

ESC = chr(27)


class TelnetServer(recvline.HistoricRecvLine):
    """A Telnet server to control the application from a command line interface."""

    def __init__(self):
        self.prompt = "pof: "
        self.ps = (self.prompt,)
        self.shortcuts = None
        self.history_file = None

    def next_line(self):
        """
        This method overwrite the Twisted one because there's was a bug
        (Gnome-terminal was not scrolling at the bottom of the page).
        Write a '\\\\n' instead of an ESC character.
        """
        # It's really should be a '\n' in the docstring above but sphinx barf on it.
        self.terminal.cursorPos.x = 0
        self.terminal.cursorPos.y = min(self.terminal.cursorPos.y + 1, self.terminal.termSize.y - 1)
        self.terminal.write('\n')
        

    def write(self, msg, prompt=False, endl=True):
        msg2 = str(msg) # Why this change ???
        self.terminal.write("%s" % (msg2.encode('utf-8')))
        if endl:
            self.terminal.nextLine()
        if prompt:
            self.write_prompt()

    def connectionMade(self):
        self.addr = self.terminal.transport.getPeer()
        # overwrite twisted terminal.nextLine method with our own to fix a bug 
        self.terminal.nextLine = self.next_line
        recvline.HistoricRecvLine.connectionMade(self)
        try:
            self.history_file = open('%s/.sropulpof/.cli_history_%s-%s' % 
                                    (os.environ['HOME'],
                                     self.addr.host,
                                     self.terminal.transport.getHost().port),
                                     'r')
        except IOError:
            #self.history_file = None
            log.info('Could not read .cli_history file.')
        else:
#            self.history_file.seek(0)
            self.historyLines = [line.strip() for line in self.history_file]
            self.history_file.close()
            self.historyPosition = len(self.historyLines)
        t = self.terminal
        self.keyHandlers.update({'\x01': self.handle_HOME,
                                 '\x05': self.handle_END,
                                 t.TAB: self.completion})

        log.info("%s is connecting in Telnet on port %s" % (self.addr.host, self.addr.port))
        self.write("Welcome to Sropulpof!")
        hostname = socket.gethostname()
        msg = ']2;telnet miville to %s' % hostname
        self.terminal.write("%s" % (msg.encode('utf-8')))
        self.write_prompt()

    def handle_RETURN(self):
        if self.lineBuffer:
            line = ''.join(self.lineBuffer)
            if line in self.historyLines:
                self.historyLines.remove(line)
            self.historyLines.append(line)
            try:
                #TODO: create a general manager for managing the verification/creation of the prefs directory
                self.history_file = open('%s/.sropulpof/.cli_history_%s-%s' % 
                            (os.environ['HOME'],
                             self.addr.host,
                             self.terminal.transport.getHost().port),
                             'w')
            except IOError:
                log.info('Could not write .cli_history file.')
            else:
                if len(self.historyLines) > 50:
                    self.history_file.write('\n'.join(self.historyLines[-50:]))
                else:
                    try:
                        self.history_file.write('\n'.join(self.historyLines))
                    except TypeError, e:
                        log.error(e.message)
                self.history_file.close()
        self.historyPosition = len(self.historyLines)
        
        recvline.RecvLine.handle_RETURN(self)
    
    def completion(self):
        # rudimentary command completion
        begin = "".join(self.lineBuffer)
        length = len(begin)
        words = self.shortcuts.values()
        self.matches = [word for word in words if word[:length] == begin]
        if self.matches:
            if len(self.matches) == 1:
                end = self.matches[0][length:]
                self.lineBuffer.extend(list(end))
                self.lineBufferIndex = len(self.lineBuffer)
                self.terminal.write(end)
                self.matches = None
            else:
                # TODO: implement when there's more then one match
                pass
        else:
            # ring the bell
            self.terminal.write('\x07')

    def lineReceived(self, line):
        line = line.strip()
        if line:
            if line.lower() in ("exit", "quit"):
                if isinstance(self.history_file, file):
                    self.history_file.close()
                log.info("Telnet client at %s:%s has disconnected" % (self.addr.host, self.addr.port))
                self.write("Good Bye")
                self.terminal.loseConnection()
            else:
                self.parse(line)
        else:
            self.parse(line)
#            self.write_prompt()

    def parse(self, line):
        """Method to be overidden when subclassing."""
        self.write("command not found")
        self.write_prompt()

    def write_prompt(self):
        self.terminal.write(self.prompt)


class CliController(TelnetServer):
    """ 
    Command line parsers for each command
    
    TODO: we need to call each method in order to print the infos. 
    Each one instaciates a CliParser object the is ephemerous. It is 
    Very hard to retrieve information on each command this way.
    """
    def __init__(self, subject):
        TelnetServer.__init__(self)
        self.view = CliView(subject, self)
        self.core = subject.api
        self.block = False
        self.remote = None
        # build a dict of all semi-private methods
        self.callbacks = find_callbacks(self)
        self.shortcuts = {'c': 'contacts',
                          #'a': 'audio',
                          #'v': 'video',
                          's': 'settings',
                          'z': 'streams',
                          'j': 'join',
                          'e': 'exit',
                          'q': 'quit',
                          'n': 'network',
                          'd': 'devices',
                          'h': 'help',
                          'p': 'ping'
                          }

    def parse(self, line):
        """
        Parses something entered by the user.
        """
        line = to_utf(line)
        if callable(self.block):
            self.block(line)
        elif line:
            line = line.split()
            cmd = line[0]
            if len(cmd) == 1 and self.shortcuts.has_key(cmd):
                cmd = self.shortcuts[cmd]
            if cmd in self.callbacks:
                self.callbacks[cmd](line)
            else:
                self.write("command not found")
                self.write_prompt()
        else:
            self.write_prompt()

    def connectionLost(self, reason=protocol.connectionDone):
        del self.view.callbacks # delete this first to remove the reference to self
        del self.view

    def _ask(self, line=None):
        if not line or line.lower() == "y":
            self.core.accept_connection(self, self.remote)
            self.block = False
        elif line.lower() == "n":
            self.core.refuse_connection(self, self.remote)
            self.block = False
            self.remote = None
        elif self.block:
            self.write('This is not a valid answer.\n[Y/n]:')

    def _pyt(self, line):
        s = str(line)
        log.info("pyt: " + s)
        
    def _settings(self, line):
        
        cp = CliParser(self, prog=line[0], description="Manages the settings.")
        cp.add_option("-l", "--list", action='store_true', help="List all the settings")
        cp.add_option("-a", "--add", type="string", help="Add a setting")
        cp.add_option("-t", "--type", type="string", help="Type of setting (global, streamsubgroup, stream or media )")
        cp.add_option("-e", "--erase", "--remove", "--delete", type="string", help="Erase a setting")
        cp.add_option("-m", "--modify", type="string", help="Modify a setting")
        cp.add_option("-d", "--duplicate", action="store_true", help="Duplicate a setting")
        cp.add_option("-s", "--select", help="Select a setting")
        cp.add_option("-k", "--save", action="store_true", help="Save user settings")
        cp.add_option("-o", "--load", action="store_true", help="Load presets and user settings (overwrites current settings)")
       
        # options to select global setting to use when manipulating streamgroups and streams
        cp.add_option("-g", "--globalsetting", type="string", help="The global setting name to use when editing global settings")
        cp.add_option("-p", "--subgroup", type="string", help="The stream subgroup name to use when editing media streams")
        cp.add_option("-n", "--mediasetting", type="string", help="The media setting name to use when editing media settings")
        cp.add_option("-i", "--mediastream", type="string", help="The media stream name to use when editing media streams")
        
        cp.add_option("-x", "--xlist", action='store_true', help="List settings hierarchy")      
        cp.add_option("-z", "--description", action='store_true', help="Display description")
        (options, args) = cp.parse_args(line)
        
        if options.xlist:
            self.core.pretty_list_settings(self)
        elif options.save:
            self.core.save_settings(self)
        elif options.load:
            self.core.load_settings(self)
        elif options.description:
            cp.print_description() 
        elif options.type:
            if options.modify:
                tokens  = options.modify.split("=")
                attribute = tokens[0]
                new_value = tokens[1]
            if options.type == 'global':   
                if options.list:
                    self.core.list_global_setting(self)
                elif options.add:
                    self.core.add_global_setting(self, options.add) # options.type
                elif options.erase:
                    self.core.erase_global_setting(self,options.erase)
                elif options.modify:
                    self.core.modify_global_setting(self, options.globalsetting, attribute, new_value)
                elif options.duplicate:
                    self.core.duplicate_global_setting(self,None)
                elif options.select:
                    self.core.select_global_setting(self,options.select)
                
            elif options.type == 'media':
                if options.list:
                    self.core.list_media_setting(self)
                elif options.add:
                    self.core.add_media_setting(self, options.add) # options.type
                elif options.erase:
                    self.core.erase_media_setting(self,options.erase)
                elif options.modify:
                    self.core.modify_media_setting(self,options.mediasetting, attribute, new_value)
                elif options.duplicate:
                    self.core.duplicate_media_setting(self,None)
                elif options.select:
                    self.core.select_media_setting(self,options.select)
            elif options.type == 'streamsubgroup':
                if options.list:
                    self.core.list_stream_subgroup(self, options.globalsetting)
                elif options.add:
                    self.core.add_stream_subgroup(self, options.add, options.globalsetting) # options.type
                elif options.erase:
                    self.core.erase_stream_subgroup(self,options.erase, options.globalsetting)
                elif options.modify:
                    self.core.modify_stream_subgroup(self, options.globalsetting, options.subgroup, attribute, new_value)
                elif options.duplicate:
                    self.core.duplicate_stream_subgroup(self,None, options.globalsetting)
                elif options.select:
                    self.core.select_stream_subgroup(self,options.select, options.globalsetting)
            elif options.type == 'stream':
                if options.list:
                    self.core.list_media_stream(self, options.globalsetting, options.subgroup)
                elif options.add:
                    self.core.add_media_stream(self, options.add, options.globalsetting, options.subgroup) # options.type
                elif options.erase:
                    self.core.erase_media_stream(self, options.globalsetting, options.subgroup, options.erase)
                elif options.modify:
                    self.core.modify_media_stream(self, options.globalsetting, options.subgroup, options.mediastream, attribute, new_value )
                elif options.duplicate:
                    self.core.duplicate_media_stream(self, options.globalsetting, options.subgroup)
                elif options.select:
                    self.core.select_media_stream(self,options.select, options.globalsetting, options.subgroup) 
        else:    
            cp.print_help()
                
    
    def _contacts(self, line):
        cp = CliParser(self, prog=line[0], description="Manages the address book.")
        cp.add_option("-l", "--list", action='store_true', help="List all the contacts")
        cp.add_option("-a", "--add", type="string", help="Add a contact")
        cp.add_option("-e", "--erase", "--remove", "--delete", action='store_true', help="Erase a contact")
        cp.add_option("-m", "--modify", action="store_true", help="Modify a contact")
        cp.add_option("-d", "--duplicate", action="store_true", help="Duplicate a contact")
        cp.add_option("-s", "--select", help="Select a contact")
        cp.add_option("-k", "--keep", "--save", action="store_true", help="Keep (save) an auto-created contact (a caller)")
        cp.add_option("-z", "--description", action='store_true', help="Display description")
        (options, args) = cp.parse_args(line)

        if options.list:
            self.core.get_contacts(self)
        elif options.description:
            cp.print_description()
        elif options.add:
            if len(args) == 2:
                self.core.add_contact(self, options.add, args[1])
            elif len(args) > 2:
                if args[2].isdigit():
                    self.core.add_contact(self, options.add, args[1], args[2])
                else:
                    self.write('The port is not valid.', True)
            else:
                self.write('You need to give at least a name and an address.', True)
        elif options.erase:
            if len(args) > 1:
                self.core.delete_contact(self, args[1])
            else:
                self.core.delete_contact(self)
        elif options.modify:
            # parses key=val arguments:
            if len(args) > 1:
                name = None
                new_name = None
                address = None
                port = None
                setting = None
                auto_answer = None
                for i, arg in enumerate(args):
                    if i > 0:
                        if '=' in arg:
                            key, sep, value = arg.partition('=')
                            if key == 'name':
                                name = value
                            elif key == 'new_name':
                                new_name = value
                            elif key == 'address':
                                address = value
                            elif key == 'port':
                                port = value
                            elif key == 'setting':
                                setting = value
                            elif key == 'auto_answer':
                                auto_answer = value
                            else:
                                self.write("Unable to change %s: unexisting property" % (key) , False)
                        elif new_name == None: #if no key=value pair, resolution order:
                            new_name = arg
                        elif address == None:
                            address = arg
                        elif port == None:
                            port = arg
                        elif name == None:
                            name = arg
                        elif auto_answer == None:
                            auto_answer = arg

                self.core.modify_contact(self, name, new_name, address, port, auto_answer=auto_answer, setting=setting)
            else:
                self.write('You need to give at least one argument.', True)
        elif options.duplicate:
            name = None
            new_name = None
            for i, arg in enumerate(args):
                if i > 0:
                    if '=' in arg:
                        key, sep, value = arg.partition('=')
                        if key == 'name':
                            name = value
                        elif key == 'new_name':
                            new_name = value
                    elif new_name == None:
                        new_name = arg
                    elif name == None:
                        name = arg

            self.core.duplicate_contact(self, name, new_name)

        elif options.select:
            self.core.select_contact(self, options.select)
        elif options.keep:
            print args
            if len(args) == 1:
                self.core.save_client_contact(self)
            elif len(args) == 2:
                self.core.save_client_contact(self, args[1])
            elif len(args) > 2:
                self.core.save_client_contact(self, args[1], args[2])
        else:
            cp.print_help()

    

    def _streams(self, line):
        cp = CliParser(self, prog=line[0], description="Manage the wrapper stream.")
        cp.add_option("-s", "--start", type="string", help="Start streaming with specified contact")
        cp.add_option("-i", "--stop", "--interrupt", action='store_true', help="Stop a stream of playing")
        cp.add_option("-z", "--description", action='store_true', help="Display description")
        
        (options, args) = cp.parse_args(line)
        kind = 'streams'
        if options.start:
            contact_name = options.start
            self.core.start_streams(self, contact_name)
        elif options.stop:
            self.core.stop_streams(self)
        elif options.description:
            cp.print_description()
        else:
            cp.print_help()

    def _join(self, line):
        cp = CliParser(self, prog=line[0], description="Start and stop a connection.")
        cp.add_option("-s", "--start", action='store_true', help="Start a connection with the default contact")
        cp.add_option("-i", "--stop", "--interrupt", action='store_true', help="Stop the connection")
        cp.add_option("-z", "--description", action='store_true', help="Display description")
        
        (options, args) = cp.parse_args(line)

        if options.start:
            self.core.start_connection(self)
        elif options.description:
            cp.print_description()
        elif options.stop:
            self.core.stop_connection(self)
        else:
            cp.print_help()
    
    def print_all_commands(self):
        """
        Prints the description of each available command
        """
        for cmd in self.callbacks.keys():
            if cmd == 'ask' or cmd == 'help':
                # crash or infinite recursion...
                pass
            else:
                self.write("%9s:   " % cmd, False, False) # no prompt, no endl
                data = [cmd, '--description']
                self.callbacks[cmd](data)
        self.write("exit:   Quits the client.", False)
        self.write("quit:   Quits the client.", False)
        self.write_prompt()
        #print_usage
        
    def _help(self, line):
        """
        Displays list of commands
        """
        
        cp = CliParser(self, prog=line[0], description="Prints descriptions of all commands.")
        #cp.add_option("-l", "--list", help="List all commands in Sropulpof.")
        #cp.add_option("-z", "--description", action='store_true', help="Display description")
        
        (options, args) = cp.parse_args(line)
        
        #if len(args) > 1 or len(options) > 1 :
        #    if options.list:
        #        self.print_all_commands()
        #elif options.usage:
        #    cp.print_usage()
        #else:
        self.print_all_commands()

    def _norm(self, line):                                                                       
        """                                                                                      
        Easily sets the video standard. (norm)                                                   
                                                                                                 
        Valid string values are "ntsc", "secam" and "pal".                                       
        If value is None, sets it according to the time zone.                                    
        """                                                                                      
                                                                                                 
        cp = CliParser(self, prog=line[0], description="Easily sets the video standard.")        
        # strings options                                                                        
        cp.add_option("-n", "--norm", type='string', default="ntsc", help="Specifies the norm such as 'ntsc', 'pal' or 'secam'")
        (options, args) = cp.parse_args(line)                                                    
        
        if options.norm:                                                                         
            value = self.core.set_video_standard(self, options.norm)
            self.write("\nSet video standard to %s\n" % (value))
        else:
            cp.print_help()    

    def _devices(self, line):
        """
        Manages v4l2 and ALSA devices.
        
        Usage:
        devices -l : list drivers and devices of each type
        devices -t v4l2 -d /dev/video0 -a : list attributes for a device
        devices -t v4l2 -d /dev/video0 -m norm ntsc  : modifies an attribute for a device
        """
        cp = CliParser(self, prog=line[0], description="Manages the audio/video/data devices.")
        
        # booleans (action)
        cp.add_option("-l", "--list", action='store_true', help="Lists all the drivers and devices: devices -l")
        cp.add_option("-a", "--attributes", action='store_true', help="Lists attributes for a device: devices -t v4l2 -d /dev/video0 -a")
        cp.add_option("-z", "--description", action='store_true', help="Displays description")
       
        # strings options
        cp.add_option("-k", "--kind", type='string', help="Specifies a kind of devices, such as 'video', 'audio' or 'data'")
        cp.add_option("-t", "--driver", type="string", help="Specifies a driver.")
        cp.add_option("-d", "--device", type="string", help="Specifies a device.")
        
        # modifiers
        cp.add_option("-m", "--modify", type="string", help="Modifies the value of an attribute: devices -t v4l2 -d /dev/video0 -m norm ntsc")
        
        (options, args) = cp.parse_args(line)
        # USE CASES:
        # 1) devices_list
        if options.list:
            if options.kind:
                # devices_list(self, caller, driver_kind)
                self.core.devices_list(self, options.kind)
            else:
                self.write("Please specify a driver kind such as 'video' or 'audio'")
                cp.print_help()

        # 2) device_list_attributes
        elif options.attributes:
            if options.driver and options.device and options.kind:
                # device_list_attributes(self, caller, driver_kind, driver_name, device_name)
                self.core.device_list_attributes(self, options.kind, options.driver, options.device)
            else:
                self.write("Please specify a driver kind, a device and a driver.")
                cp.print_help()

        # 3) device_modify_attribute
        elif options.modify:
            if options.driver and options.device and options.kind and len(args) == 2:
                value = args[1]
                # device_modify_attribute(self, caller, driver_kind, driver_name, device_name, attribute_name, value)
                self.core.device_modify_attribute(self, options.kind, options.driver, options.device, options.modify, value) 
                # TODO: should return success.
            else:
                self.write("Please specify a driver, a driver kind and a device")
                cp.print_help()
        
        # HELP:
        elif options.description:
            cp.print_description()
        else:
            cp.print_help()
        self.write_prompt()


    def _network(self, line):
        """
        Starts (or stop) a network test.
        
        Usage:
        network -b 30 -d 10 -k unidirectional
        """
        # TODO: add contact (-c) argument
        cp = CliParser(self, prog=line[0], description="Manages the audio/video/data devices.")
        
        # booleans (action)
        cp.add_option("-z", "--description", action='store_true', help="Displays description") # TODO: add examples
        cp.add_option("-s", "--start", action='store_true', help="Starts a test")
        cp.add_option("-q", "--stop", action='store_true', help="Stops the current test")
        #cp.add_option("-h", "--help", action='store_true', help="Displays help")
        
        # strings options
        cp.add_option("-k", "--kind", type='string', help="Kind of network test. (locaotoremote | remotetolocal tradeoff | dualtest)")
        #cp.add_option("-c", "--contact", type='string', help="Specifies a contact to test with")
        
        # int options
        cp.add_option("-b", "--bandwidth", type="int", help="Bandwidth in megabits. (default:1)")
        cp.add_option("-t", "--time", type="int", help="Duration in seconds. (default:10)")
        (options, args) = cp.parse_args(line)
        
        # default values : 
        bandwidth = 1
        duration = 10 
        kind = "localtoremote"
        caller = self

        if options.description:
            cp.print_description()
        elif options.stop:
            self.core.network_test_stop(caller)
        elif  options.start:
            if options.kind:
                kind = options.kind
            if options.bandwidth:
                bandwidth = options.bandwidth
            if options.time:
                duration = options.time
            self.core.network_test_start(caller, bandwidth, duration, kind)
        else: # options.help
            cp.print_help()
        self.write_prompt()

    def _ping(self, line):
        """
        Starts a pinger test.
        """
        self.core.pinger_start(self) # arg should be caller.
        self.write_prompt()

    def _firereset(self, line):
        """
        Resets the firewire bus.
        """
        self.core.reset_firewire_bus(self)
        self.write_prompt()

class CliParser(optparse.OptionParser):
    """
    Base class for each CLI command 
    """
    def __init__(self,
                 output,
                 usage=None,
                 option_list=None,
                 option_class=optparse.Option,
                 version=None,
                 conflict_handler="error",
                 description=None,
                 formatter=None,
                 add_help_option=True,
                 prog=None,
                 epilog=None):
        optparse.OptionParser.__init__(self, usage, option_list, option_class, version, conflict_handler, description, formatter, add_help_option, prog, epilog)
        self.output = output

    def print_description(self, with_prompt=False):
        # TODO: Should display the prompt if entered by the user
        #       and not automagically by the help command.
        # TODO: Add name of the command
        self.output.write(self.description)
        if with_prompt:
            self.output.write_prompt()
    
    def error(self, msg):
        if msg:
            self.output.write(msg, True)
        else:
            self.print_usage()

    def exit(self, status=0, msg=None):
        if msg:
            self.output.write(msg)
            self.output.write_prompt()

    def print_usage(self, file=None):
        """print_usage(file : file = stdout)

        Print the usage message for the current program (self.usage) to
        'file' (default stdout).  Any occurence of the string "%prog" in
        self.usage is replaced with the name of the current program
        (basename of sys.argv[0]).  Does nothing if self.usage is empty
        or not defined.
        """
        if self.usage:
            self.output.write(self.get_usage())
            self.output.write_prompt()

    def print_version(self, file=None):
        """print_version(file : file = stdout)

        Print the version message for this program (self.version) to
        'file' (default stdout).  As with print_usage(), any occurence
        of "%prog" in self.version is replaced by the current program's
        name.  Does nothing if self.version is empty or undefined.
        """
        if self.version:
            self.output.write(self.get_version())
            self.output.write_prompt()

    def print_help(self, file=None):
        """print_help(file : file = stdout)

        Print an extended help message, listing all options and any
        help text provided with them.
        """
        self.output.write(self.format_help(), True)

    def get_prog_name(self):
        return self.prog

    def _process_short_opts(self, rargs, values):
        arg = rargs.pop(0)
        stop = False
        i = 1 # number of chars read ?
        for ch in arg[1:]:
            opt = "-" + ch
            option = self._short_opt.get(opt)
            i += 1                      # we have consumed a character

            if not option:
                raise optparse.BadOptionError(opt)
            if option.takes_value():
                # Any characters left in arg?  Pretend they're the
                # next arg, and stop consuming characters of arg.
                if i < len(arg):
                    rargs.insert(0, arg[i:])
                    stop = True

                nargs = option.nargs
                if len(rargs) < nargs:
                    if nargs == 1:
                        self.error(optparse._("%s option requires an argument") % opt)
                        return
                    else:
                        self.error(optparse._("%s option requires %d arguments")
                                   % (opt, nargs))
                        return

                elif nargs == 1:
                    value = rargs.pop(0)
                else:
                    value = tuple(rargs[0:nargs])
                    del rargs[0:nargs]

            else:                       # option doesn't take a value
                value = None

            option.process(opt, value, values, self)

            if stop:
                break

    def _process_long_opt(self, rargs, values):
        arg = rargs.pop(0)

        # Value explicitly attached to arg?  Pretend it's the next
        # argument.
        if "=" in arg:
            (opt, next_arg) = arg.split("=", 1)
            rargs.insert(0, next_arg)
            had_explicit_value = True
        else:
            opt = arg
            had_explicit_value = False

        opt = self._match_long_opt(opt)
        option = self._long_opt[opt]
        if option.takes_value():
            nargs = option.nargs
            if len(rargs) < nargs:
                if nargs == 1:
                    self.error(optparse._("%s option requires an argument") % opt)
                    return
                else:
                    self.error(optparse._("%s option requires %d arguments")
                               % (opt, nargs))
                    return
            elif nargs == 1:
                value = rargs.pop(0)
            else:
                value = tuple(rargs[0:nargs])
                del rargs[0:nargs]

        elif had_explicit_value:
            self.error(optparse._("%s option does not take a value") % opt)

        else:
            value = None

        option.process(opt, value, values, self)


class CliView(Observer):
    """ 
    Command-line interface results printer. 
    The View (Observer) in the MVC pattern. 
    """
    
    def __init__(self, subject, controller):
        Observer.__init__(self, subject)
        self.controller = controller
        self.callbacks = find_callbacks(self)

    def update(self, origin, key, data):
        """
        Called when a attribute of its Subject watched objects change.
        
        Will call any of the methods of this class that start with "_".
        The keys are their name without the "_".
        For example, calling "get_contacts" will call _get_contacts.
        All of those methods accept two args: origin and data. (a tuple or so)
        """
        if key in self.callbacks:
            self.callbacks[key](origin, data)
        else:
            log.error("update(): Notification not in callbacks: %s" % (key))

    def write(self, msg, prompt=True):
        self.controller.write(msg)
        if prompt:
            self.controller.write_prompt()
            
    ### Settings
    
    def _modify_media_stream(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not modify media stream.\nReason: ' +  str(data))
            else:
                self.write('Stream settings modified')

    
    def _erase_media_stream(self,origin,data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not erase media stream.\nReason: ' +  str(data))
            else:
                self.write('Media stream removed')        
        
    def _add_media_stream(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not add media stream.\nReason: ' +  str(data))
            else:
                self.write('Media stream added')
                    
    
    def _list_media_stream(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not list media streams.\nReason: ' +  str(data))
            else:
                (media_streams, selected_media_stream) = data
                if len(media_streams) == 0:
                    self.write('There are no media streams\n')
                else:
                    txt = ""
                    for media_stream in media_streams:
                        name = media_stream.name
                        setting = str(media_stream.setting)
                        if name == selected_media_stream:
                            name = bold(name + ": <---")
                        port = media_stream.port
                        port = str(port)
                        gain = str(media_stream.gain_levels)
                        sync = str(media_stream.sync_group)
                        enabled = str(media_stream.enabled)
                        txt += """%(name)s:
    enabled       : %(enabled)s
    port          : %(port)s
    gain levels   : %(gain)s
    media setting : %(setting)s
    sync group    : %(sync)s
""" % locals()
                    self.write( txt )
                        
    
    def _list_stream_subgroup(self, origin, data): # list media settings
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not list media settings.\nReason: ' +  str(data))
            else:
                (stream_subgroups, selected_stream_subgroup) = data
                if len(stream_subgroups) == 0:
                    self.write('There are no streamsubgroup settings\n')
                else:
                    txt = ''
                    for k in stream_subgroups.keys():
                        sub_group = stream_subgroups[k]
                        name = sub_group.name
                        if name == selected_stream_subgroup:
                            name = bold(name + ": <---")
                        enabled = str(sub_group.enabled)
                        mode = str(sub_group.mode)
                        port = str(sub_group.port)
                        container = str(sub_group.container)
                        txt += """%(name)s:
    id        : %(k)3d
    enabled   : %(enabled)s
    mode      : %(mode)s
    port      : %(port)s
    container : %(container)s """ % locals()
                        self.write( txt )


    def _add_stream_subgroup(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not add stream  settings.\nReason: ' +  str(data))
            else:
                self.write('Stream subgroup added')


    def _erase_stream_subgroup(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not erase stream  settings.\nReason: ' +  str(data))
            else:
                self.write('Stream subgroup removed')

    def _modify_stream_subgroup(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not modify stream  settings.\nReason: ' +  str(data))
            else:
                self.write('Stream settings modified')

    def _duplicate_stream_subgroup(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not duplicate stream  settings.\nReason: ' +  str(data))
            else:
                self.write('stream settings duplicate')

    def _select_stream_subgroup(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not select stream  settings.\nReason: ' +  str(data))
            else:
                self.write('Stream settings select')
    
    def _list_media_setting(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not list media settings.\nReason: ' +  str(data))
            else:
                (media_settings, selected_media_setting) = data
                if len(media_settings) == 0:
                    self.write('There are no media settings\n')
                else:
                    txt = ''
                    for k in media_settings.keys():
                        name = media_settings[k].name
                        settings_ = "\n" #str(media_settings[k].settings)
                        for setting_name, setting_value in media_settings[k].settings.iteritems():
                            settings_ += "                " + str(setting_name) + ':' + str(setting_value) + "\n"
                        if name == selected_media_setting:
                            name = bold(name + ": <---")
                        others = "None"
                        txt += """%(name)s:
    id      : %(k)3d
    settings: %(settings_)s\n""" % locals()
                    self.write( txt )

    def _add_media_setting(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not add media setting.\nReason: ' +  str(data))
            else:
                self.write('Media setting added')

    def _erase_media_setting(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not erase media setting.\nReason: ' +  str(data))
            else:
                self.write('Media setting removed')

    def _modify_media_setting(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not modify media setting.\nReason: ' +  str(data))
            else:
                self.write('Media setting modified')

    def _duplicate_media_setting(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not duplicate media setting.\nReason: ' +  str(data))
            else:
                self.write('Media setting duplicated')

    def _select_media_setting(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not select media setting.\nReason: ' +  str(data))
            else:
                self.write('Media setting selected')
    
    def _pretty_list_settings(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not list global settings.\nReason: ' +  str(data))
            else:
                got_some = False
                (global_settings, media_settings) = data
                txt = "\nGLOBAL SETTINGS:\n"
                for k, v in global_settings.iteritems():
                    txt += " [" + str(k) + "] " + v.name + "\n"
                    for gid,group in v.stream_subgroups.iteritems():
                        txt += "  [" + str(gid) + "] " + group.name + "\n"
                        txt += "   enabled: " + str(group.enabled) + "\n"
                        txt += "   mode: " + str(group.mode) + "\n"
                        for stream in group.media_streams:
                            txt += "   " + stream.name + "\n"
                            txt += "    enabled: " + str(stream.enabled) + "\n"
                            txt += "    sync: " + stream.sync_group + "\n"
                            txt += "    port: %s\n" % str(stream.port) + "\n"
                            txt += "    media setting: %s\n" % str(stream.setting)

                        
                txt += "\nMEDIA SETTINGS...\n"
                for k, v in media_settings.iteritems():
                    txt += " [" + str(k) + "] " + v.name + "\n"
                    for key, value in v.settings.iteritems():
                        txt += "  %s : %s\n" % (key, str(value))
                self.write( txt )
                    
    
    def _list_global_setting(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not list global settings.\nReason: ' +  str(data))
            else:
                (global_settings, selected_global_setting) = data
                if len(global_settings) == 0:
                    self.write('There are no global settings\n')
                else:
                    txt = ''
                    for k in global_settings.keys():
                        comm = global_settings[k].communication
                        name = global_settings[k].name
                        if name == selected_global_setting:
                            name = bold(name + ": <---")
                        others = "None"
                        txt += """%(name)s:
    id:            %(k)3d
    communication: %(comm)s
    others:        %(others)s\n""" % locals()
                    self.write( txt )

    def _add_global_setting(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not add settings.\nReason: ' +  str(data) )
            else:
                self.write('Global setting added')

    def _erase_global_setting(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not erase settings.\nReason: ' +  str(data))
            else:
                self.write('Global setting removed')

    def _modify_global_setting(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not modify global settings.\nReason: ' +  str(data))
            else:
                self.write('Global setting modified')

    def _duplicate_global_setting(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not duplicate settings.\nReason: ' +  str(data))
            else:
                self.write('Global setting duplicated')

    def _select_global_setting(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not select settings.\nReason: ' +  str(data))
            else:
                self.write('Global setting selected')

    def _load_settings(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not load settings.\nReason: ' +  str(data))
            else:
                self.write('Settings loaded')

    def _save_settings(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not save settings.\nReason: ' +  str(data))
            else:
                self.write('Settings saved')

#    def _description_global_setting(self, origin, data):
#        if origin is self.controller:
#            if isinstance(data, Exception):
#                self.write('Could not describe settings.\nReason: ' +  str(data))
#            else:
#                self.write('Settings added')
    
    ### Contacts
    
    def _get_contacts(self, origin, data):
        """
        called by api.py:
        data: dict 'contact':'contact object'.
        """
        if origin is self.controller:
            msg = []
            contacts = data[0].items()
            contacts.sort()
            selected = data[1]
            for name, contact in contacts:
                if name == selected:
                    msg.append(bold(contact.name + ": <---"))
                elif contact.auto_created:
                    msg.append(italic(bold(contact.name + ":")))
                else:
                    msg.append(contact.name + ":")
                msg.append("\taddress: %s" % contact.address)
                #if contact.port:
                msg.append("\tport   : %s" % str(contact.port))
                #if contact.kind:
                msg.append("\tkind   : %s" % str(contact.kind))
                msg.append("\tsetting   : %s" % str(contact.setting))
            msg_out = "\n".join(msg)
            self.write(msg_out)

    def _add_contact(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not add contact.\nReason: %s.' % data)
            else:
                self.write('Contact added')

    def _delete_contact(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not delete contact.\nReason: %s' % data)
            else:
                self.write('Contact deleted')

    def _modify_contact(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not modify this contact.\nReason: %s' % data)
            else:
                self.write('Contact modified')

    def _duplicate_contact(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not duplicate contact.\nReason: %s' % data)
            else:
                self.write('Contact duplicated')

    def _save_client_contact(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not keep this contact.\nReason: %s' % data)
            else:
                self.write('Contact saved')

    def _select_contact(self, origin, data):
        if origin is self.controller:
            if isinstance(data, Exception):
                self.write('Could not select this contact.\nReason: %s' % data)
            else:
                self.write('Contact selected')

    def _audio_set(self, origin, ((state, attr, value), name)):
        """
        
        """
        if origin is self.controller:
            if state:
                self.write('%s of audio stream %s is set to %s.' % (attr, name, value))
            else:
                self.write('Unable to set %s of audio stream %s.' % (attr, name))
                
    def _not_found(self, origin, (name, kind)): # _not_found(self, origin, name, kind)
        """
        Displays a command not found error.
        
        Now with two args like everything else.
        """
        if origin is self.controller:
            self.write('There\'s no %s stream with the name %s' % (kind, name))

    def _audio_settings(self, origin, (data, name)):
        if origin is self.controller:
            if data:
                if data._state == 0:
                    state = 'idle'
                elif data._state == 1:
                    state = 'playing'
                else:
                    state = 'unknown'
                msg = [bold('Audio Stream: ' + name)]
                msg.append('\nstatus: ')
                msg.append(state)
                msg.append('\ncontainer: ')
                msg.append(str(data.container))
                msg.append('\ncodec: ')
                msg.append(str(data.codec))
                msg.append('\ncodec settings: ')
                msg.append(str(data.codec_settings))
                msg.append('\nbitdepth: ')
                msg.append(str(data.bitdepth))
                msg.append('\nsamplerate: ')
                msg.append(str(data.sample_rate))
                msg.append('\nchannels: ')
                msg.append(str(data.channels))
                msg.append('\nbuffer: ')
                msg.append(str(data.buffer))
                msg.append('\nport: ')
                msg.append(str(data.port))
                msg.append('\nsource: ')
                msg.append(str(data.source))
                self.write("".join(msg))
            else:
                self._not_found(origin, (name, 'audio'))
                
            
    def _audio_add(self, origin, (data, name)):
        if origin is self.controller:
            if data == 1:
                self.write('Audio stream %s created.' % name)
            elif data == 0:
                self.write("Cannot create audio stream. Name '%s' already use." % name)
            else:
                self.write('Unable to create the audio stream %s.' % name)

    def _audio_delete(self, origin, (data, name)):
        if origin is self.controller:
            if data == 1:
                self.write('Audio stream %s deleted.' % name)
            elif data == 0:
                self._not_found(origin, (name, 'audio')) 
            else:
                self.write('Unable to delete the audio stream %s.' % name)

    def _audio_rename(self, origin, (data, name, new_name)):
        if origin is self.controller:
            if data == 1:
                self.write('Audio stream %s rename to %s.' % (name, new_name))
            elif data == 0:
                self._not_found(origin, (name, 'audio')) 
            else:
                self.write('Unable to rename the audio stream %s.' % name)


    def _audio_list(self, origin, data):
        if origin is self.controller:
            if data:
                names = [stream[0] for stream in data]
                self.write("\n".join(names))
            else:
                self.write('There is no audio stream.')

    def _audio_sending_started(self, origin, data):
        if data:
            self.write('\nAudio sending started.')
        else:
            self.write('\nUnable to start audio sending.')

    def _audio_sending_stopped(self, origin, data):
        if data:
            self.write('\nAudio sending stopped.')
        else:
            self.write('\nUnable to stop audio sending.')


    def _video_settings(self, origin, (data, name)):
        if origin is self.controller:
            if data:
                if data._state == 0:
                    state = 'idle'
                elif data._state == 1:
                    state = 'playing'
                else:
                    state = 'unknown'
                msg = [bold('Video Stream: ' + name)]
                msg.append('\nstatus: ')
                msg.append(state)
                msg.append('\ncontainer: ')
                msg.append(str(data.container))
                msg.append('\ncodec: ')
                msg.append(str(data.codec))
                msg.append('\ncodec settings: ')
                msg.append(str(data.codec_settings))
                msg.append('\nwidth: ')
                msg.append(str(data.width))
                msg.append('\nheight: ')
                msg.append(str(data.height))
                msg.append('\nbuffer: ')
                msg.append(str(data.buffer))
                msg.append('\nport: ')
                msg.append(str(data.port))
                msg.append('\nsource: ')
                msg.append(str(data.source))
                self.write("".join(msg))
            else:
                self._not_found(origin, (name, 'video'))
            
    def _video_add(self, origin, (data, name)):
        if origin is self.controller:
            if data == 1:
                self.write('Video stream %s created.' % name)
            elif data == 0:
                self.write("Cannot create video stream. Name '%s' already use." % name)
            else:
                self.write('Unable to create the video stream %s.' % name)

    def _video_delete(self, origin, (data, name)):
        if origin is self.controller:
            if data == 1:
                self.write('Video stream %s deleted.' % name)
            elif data == 0:
                self._not_found(origin, (name, 'video'))
            else:
                self.write('Unable to delete the video stream %s.' % name)

    def _video_rename(self, origin, (data, name, new_name)):
        if origin is self.controller:
            if data == 1:
                self.write('Video stream %s rename to %s.' % (name, new_name))
            elif data == 0:
                self._not_found(origin, (name, 'video'))
            else:
                self.write('Unable to rename the video stream %s.' % name)


    def _video_list(self, origin, data):
        if origin is self.controller:
            if data:
                names = [stream[0] for stream in data]
                self.write("\n".join(names))
            else:
                self.write('There is no video stream.')

    def _video_set(self, origin, ((state, attr, value), name)):
        if origin is self.controller:
            if state:
                self.write('%s of video stream %s is set to %s.' % (attr, name, value))
            else:
                self.write('Unable to set %s of video stream %s.' % (attr, name))

    def _video_sending_started(self, origin, data):
        if data:
            self.write('\nVideo sending started.')
        else:
            self.write('\nUnable to start video sending.')

    def _video_sending_stopped(self, origin, data):
        if data:
            self.write('\nVideo sending stopped.')
        else:
            self.write('\nUnable to stop video sending.')


    def _streams_list(self, origin, (streams, data)):
        if origin is self.controller:
            names = []
            for attr, value in data.items():
                if attr != "streams" and attr[0] != '_':
                    names.append(attr + ': ' + str(value))
            if names:
                names.append('------------')
            if streams:
                for stream in streams:
                     kind = '?'
                # TODO ! 
                #     if isinstance(stream[1], AudioStream):
                #         kind = 'audio'
                #     elif isinstance(stream[1], VideoStream):
                #         kind = 'video'
                     names.append(stream[0] + ' (' + kind + ')')
            else:
                names.append('There is no stream.')
            self.write("\n".join(names))

    def _set_streams(self, origin, (state, attr, value)):
        if state:
            self.write('The %s of the master stream is now set to %s' % (attr, value))
        elif origin is self.controller:
            self.write('Unable to set the %s of the master stream' % attr)

    def _select_streams(self, origin, (name, value)):
        if value:
            self.write('The selected master stream is now  %s' % name)
        elif origin is self.controller:
            self.write('Unable to select %s as the master stream, it does\'nt exist.' % name)

    def _start_streams(self, origin, data):
        self.write(data)

    def _stop_streams(self, origin, data):
        self.write(data)

    def _list_streams(self, origin, (data, curr)):
        if origin is self.controller:
            msg = []
            for name, streams in data.items():
                if name == curr:
                    msg.append(bold(name + ": <---"))
                else:
                    msg.append(name + ":")
                for attr, value in streams.__dict__.items():
                    if attr[0] != '_':
                        msg.append("\t%s: %s" % (attr, value))
            self.write("\n".join(msg))

    def _ask(self, origin, data):
        self.write('\n%s is inviting you. Do you accept? [Y/n]: ' % data[0], False)
        self.controller.block = self.controller._ask
        self.controller.remote = data[1]

    def _ask_timeout(self, origin, data):
        self.write('You didn\'t answer soon enough to %s. Connection closed.' % data)
        self.controller.block = False
        self.controller.remote = None

    def _start_connection(self, origin, data):
        """
        Called as an answer to api.start_connection
        
        Data is a dict. 
        Possible keys in this dict are : 
         * name : contact name
         * address : ip
         * msg : string
         * context : string (could be removed)
         * exception : Exception instance
        """
        if origin is self:
            if data.has_key('exception'):
                self.write('%s with %s. Error: %s' % (data['msg'],
                                                      data['name'],
                                                      data['exception']))
            elif not data.has_key('name'):
                self.write(data['msg'])
        if not data.has_key('exception') and data.has_key('name'):
            self.write('%s with %s...' % (data['msg'], data['name']))

    def _stop_connection(self, origin, data):
        if origin is self and data.has_key('exception'):
            if data.has_key('name'):
                self.write('%s with %s. Reason: %s.' % (data['msg'],
                                                             data['name'],
                                                             data['exception']))
            else:
                self.write('%s. Reason: %s.' % (data['msg'],
                                                     data['exception']))
        if not data.has_key('exception'):
            self.write('%s with %s...' % (data['msg'], data['name']))
        
    def _info(self, origin, data):
        if isinstance(data, dict):
            if data.has_key('context'):
                if data['context'] == 'auto-answer':
                    self.write('%s with %s.' % (data['msg'], data['name']))
                if data['context'] == 'connection_closed':
                    if data.has_key('name'):
                        contact = data['name']
                    else:
                        contact = data['address']
                    self.write('%s by %s.' % (data['msg'], contact))
        else:
            self.write(data)
    
    def _connection_failed(self, origin, data):
        self.write('%s for %s:%s %s.'
                   % (data['msg'], data['address'], data['port'], data['exception']))
    
    def _error(self, origin, data):
        """
        Similar to the "info" key, but for error messages to the users.
        
        There are 2 ways to use this notify key. 
         * with a string argument
         * with a dict argument. Mandatory keys are: 'msg' and 'context'
        
        Example ::
        self.api.notify(
            caller, 
            {
            'address':self.contact.address, 
            'port':self.contact.port,
            'exception':'%s' % err,
            'msg':'Connection failed',
            'context':'connection'
            }, 
            "error")
        """
        if isinstance(data, dict):
            msg = "Error: \n"
            # mandatory arguments
            for k in data.keys():
                msg += "  %s\n" % (data[k])
            self.write(msg)
        else:
            self.write(data)

    def _answer(self, origin, data):
        self.write('\n%s by %s.' %  (data['msg'], data['name']))

    def _connectionMade(self, origin, data):
        self.write(data[0])
   
    def _device_attributes_changed(self, origin, data):
        """
        :data: list of attributes
        They can be for different devices, but will be for the same driver.
        """
        msg = []
        if origin is self.controller: # and len(data) >= 1:
            msg .append(bold('Warning : some attributes changed.')) #TODO: imporoved message
        else:
            msg .append(bold('Some attributes changed.'))
        for attribute in data:
            name = attribute.name
            value = attribute.get_value()
            msg.append("\t%s : %s" % (name, value))
            device_name = attribute.device.name
            driver_name = attribute.device.driver.name
            msg.append("Attribute %s of device %s (driver %s) changed to %s." % (name, bold(device_name), driver_name, value))
        self.write("\n".join(msg), True)

    def _device_modify_attribute(self, origin, data):
        # TODO: improve this.
        self.write("Successfully changed attribute." + str(data))

    def _device_list_attributes(self, origin, data):
        """
        :data: list of attributes
        They are all for the same device and driver.
        """
        if origin is self.controller and len(data) >= 1:
            msg = []
            device_name = data.values()[0].device.name
            driver_name = data.values()[0].device.driver.name
            msg.append("Attributes of device %s using driver %s :" % (bold(device_name), driver_name))
            for attribute in data.values():
                name = attribute.name
                value = attribute.get_value()
                msg.append("\t%s : %s" % (name, value))
            self.write("\n".join(msg), True)

    def _devices_removed(self, origin, data):
        """
        :data: dict of devices.
        All from the same driver.
        """
        msg = []
        for device in data.values():
            msg.append("Device %s:%s has been removed." % (device.driver.name, device.name))
        self.write("\n".join(msg), True)
    
    def _devices_added(self, origin, data):
        """
        :data: dict of devices
        all from the same driver.
        """
        msg = []
        for device in data.values():
            msg.append("New device : %s:%s." % (device.driver.name, device.name))
        self.write("\n".join(msg), True)

    def _devices_list(self, origin, data):
        """
        :data: list of devices
        they all belong to the same driver.
        """
        msg = []
        if origin is self.controller:
            if len(data) == 0:
                msg.append("No devices to list.")
            else:    
                msg.append("Devices for driver %s :" % (bold(data[0].driver.name)))
                for device in data:
                    msg.append("\t%s" % (device.name))
        self.write("\n".join(msg), True)
    
    def _network_test_done(self, origin, data):
        """
        Results of a network test. 
        See network.py
        :param data: a dict with iperf statistics
        """
        #print "UI/CLI/CLI: " 
        #print data
        contact_name = data['contact'].name
        txt = "\n" + bold("Network test results with %s" % contact_name) +" :\n"
        for host_name in ['local', 'remote']:
            if data.has_key(host_name):
                if host_name == "local":
                    txt += bold("From local to remote") + "\n"
                else: # remote
                    txt += bold("From remote to local") + "\n"
                host_data = data[host_name]
                for k in host_data:
                    txt += "\t%s: %s\n" % (k, str(host_data[k]))
        self.write(txt, True)
        

def bold(msg):
    return "%s[1m%s%s[0m" % (ESC, msg, ESC)

def italic(msg):
    return "%s[33m%s%s[0m" % (ESC, msg, ESC)

def add_quotes(input):
    if ':' in input:
        input, sep, args = input.partition(':')
        args = args.split(',')
        new_args = []
        for arg in args:
            key, sep, value = arg.partition('=')
            if not value.isdigit() and not re.match('^\d+\.\d*$', value) and not (value[0] == '"' and value[-1] == '"'):
                value = '"%s"' % value
            new_args.append(key + sep + value)
        input += ':' + ','.join(new_args)
    return input


def start(subject, port=14444, interfaces=''):
    """
    This runs the telnet server on specifed port 
    """
    factory = protocol.ServerFactory()
    factory.protocol = lambda: TelnetTransport(TelnetBootstrapProtocol,
                                               insults.ServerProtocol,
                                               CliController, subject)
    # subject is the core...
    subject.api.listen_tcp(port, factory, interfaces) # subject.config.ui_network_interfaces)
    #reactor.listenTCP(port, factory)


# this only runs if the module was *not* imported
if __name__ == '__main__':
    from miville.utils import Subject
    from miville.utils import log as logging
    logging.start()
    start(Subject()) # will not work, since some attributes are missing. (core) 
    reactor.run()

