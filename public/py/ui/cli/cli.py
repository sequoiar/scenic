#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technologiques (SAT)
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


# System imports
import optparse

# Twisted imports
from twisted.internet import reactor, protocol
from twisted.conch import telnet

#App imports
from utils import Observer, log
from utils.i18n import to_utf
import ui
from ui.common import find_callbacks
from streams import audio, video, data
from streams.stream import AudioStream, VideoStream, DataStream


log = log.start('info', 1, 0, 'cli')

ESC = chr(27)


class TelnetServer(protocol.Protocol):
    """A Telnet server to control the application from a command line interface."""

    def __init__(self):
        self.prompt = "pof: "
        self.history = []
        
    def write(self, msg, prompt=False):
        self.transport.write("%s\n" % (msg.encode('utf-8')))
        if prompt:
            self.write_prompt()
        
    def connectionMade(self):
        log.info("%s is connecting in Telnet on port %s" % (self.addr.host, self.addr.port))
        self.write("Welcome to Sropulpof!")
        self.write_prompt()

    def dataReceived(self, data):
        data = data.strip()
        if data:
            self.history.append(data)
            if data.lower() in ("exit", "quit"):
                log.info("Telnet client at %s:%s has disconnected" % (self.addr.host, self.addr.port))
                self.write("Good Bye")
                self.transport.loseConnection()
            else:
                self.parse(data)
        else:
            self.parse(data)
#            self.write_prompt()
            
    def parse(self, data):
        """Method to be overidden when subclassing."""
        self.write("command not found")
        self.write_prompt()            
            
    def write_prompt(self):
        self.transport.write(self.prompt)


class CliController(TelnetServer):
    """ """
        
    def __init__(self):
        TelnetServer.__init__(self)
        self.core = None
        self.block = False
        self.remote = None
        # build a dict of all semi-private methods
        self.callbacks = find_callbacks(self)
        self.shortcuts = {'c': 'contacts',
                          'a': 'audio',
                          'v': 'video',
                          's': 'streams',
                          'j': 'join'
                          }
        
    def parse(self, data):
        if not self.core:
            self.core = self.factory.subject.api
        data = to_utf(data)
        if self.block:
            self.block(data)
        elif data:
            data = data.split()
            cmd = data[0]
            if len(cmd) == 1 and self.shortcuts.has_key(cmd):
                cmd = self.shortcuts[cmd]
            if cmd in self.callbacks:
                self.callbacks[cmd](data)
            else:
                self.write("command not found")
                self.write_prompt()
        else:
            self.write_prompt()
            
    def connectionLost(self, reason=protocol.connectionDone):
        del self.view.callbacks # delete this first to remove the reference to self
        del self.view

    def _ask(self, data=None):
        if not data or data.lower() == "y":
            self.core.accept_connection(self, self.remote)
            self.block = False
        elif data.lower() == "n":
            self.core.refuse_connection(self, self.remote)
            self.block = False
            self.remote = None
        else:
            self.write('This is not a valid answer.\n[Y/n]:')

    def _contacts(self, data):
        cp = CliParser(self, prog=data[0], description="Manage the address book.")
        cp.add_option("-l", "--list", action='store_true', help="List all the contacts")
        cp.add_option("-a", "--add", type="string", help="Add a contact")
        cp.add_option("-e", "--erase", "-d", "--delete", type="string", help="Erase a contact")
        cp.add_option("-m", "--modify", type="string", help="Modify a contact")
        cp.add_option("-s", "--select", help="Select a contact")
        
        (options, args) = cp.parse_args(data)
        
        if options.list:
            self.core.get_contacts(self)
        elif options.add:
            if len(args) == 2:
                self.core.add_contact(self, options.add, args[1])
            elif len(args) > 2:
                if args[2].isdigit():
                    self.core.add_contact(self, options.add, args[1], args[2])
                else:
                    self.write('The port is not valid.', True)
            else:
                self.write('You need to give an address.', True)
        elif options.erase:
            self.core.delete_contact(self, options.erase)
        elif options.modify:
            if len(args) == 3:
                self.core.modify_contact(self, options.modify, args[1], args[2])
            elif len(args) > 3:
                self.core.modify_contact(self, options.modify, args[1], args[2], args[3])
            else:
                self.write('You need to give the current name, the new name and the address.', True)
        elif options.select:
            self.core.select_contact(self, options.select)
        else:
            cp.print_help()
            
    def _audio(self, data):
        kind = 'audio'
        cp = CliParser(self, prog=data[0], description="Manage the audio streams.")
        cp.add_option("-l", "--list", action='store_true', help="List all the audio streams or settings if stream is specified")

        cp.add_option("-a", "--add", type="string", help="Add an audio stream")
        cp.add_option("-e", "--erase", type="string", help="Erase an audio stream")
        cp.add_option("-m", "--modify", type="string", help="Modify the name of an audio stream")
        
        cp.add_option("-t", "--container", "--tank", "--type", type="string", help="Set the container")
        cp.add_option("-c", "--codec", type="string", help="Set the codec")
        cp.add_option("-s", "--settings", type="string", help="Set the codec settings (set1:val,set2:val)")
        cp.add_option("-d", "--bitdepth", type="int", help="Set the bitdepth of the audio (default: 16 bit)")
        cp.add_option("-r", "--samplerate", type="int", help="Set the samplerate of the audio (default: 48000 Hz")
        cp.add_option("-v", "--channels", "--voices", type="int", help="Set the number of audio channels (from 1 to 8)")
        cp.add_option("-p", "--port", type="int", help="Set the network port (5020-5030)")
        cp.add_option("-b", "--buffer", type="int", help="Set the latency buffer (in millisec)")

        (options, args) = cp.parse_args(data)
                
        if len(args) > 1:
            name = args[1]
            if options.list:
                self.core.settings_stream(self, name, kind)
            elif options.modify:
                self.core.rename_stream(self, name, options.modify, kind)
            elif [opt for opt in options.__dict__.values() if opt]:
                if options.container:
                    self.core.set_stream(self, name, kind, 'container', options.container)
                if options.codec:
                    self.core.set_stream(self, name, kind, 'codec', options.codec)
                if options.settings:
                    self.core.set_stream(self, name, kind, 'codec_settings', options.settings)
                if options.bitdepth:
                    self.core.set_stream(self, name, kind, 'bitdepth', options.bitdepth)
                if options.samplerate:
                    self.core.set_stream(self, name, kind, 'sample_rate', options.samplerate)
                if options.channels:
                    self.core.set_stream(self, name, kind, 'channels', options.channels)
                if options.port:
                    self.core.set_stream(self, name, kind, 'port', options.port)
                if options.buffer:
                    self.core.set_stream(self, name, kind, 'buffer', options.buffer)
        elif options.list:
            self.core.list_stream(self, kind)
        elif options.add:
            self.core.add_stream(self, options.add, kind, 'gst')
        elif options.erase:
            self.core.delete_stream(self, options.erase, kind)
        else:
            cp.print_help()
            
    def _video(self, data):
        kind = 'video'
        cp = CliParser(self, prog=data[0], description="Manage the video streams.")
        cp.add_option("-l", "--list", action='store_true', help="List all the video streams or settings if stream is specified")

        cp.add_option("-a", "--add", type="string", help="Add an video stream")
        cp.add_option("-e", "--erase", type="string", help="Erase an video stream")
        cp.add_option("-m", "--modify", type="string", help="Modify the name of an video stream")
        
        cp.add_option("-t", "--container", "--tank", "--type", type="string", help="Set the container")
        cp.add_option("-c", "--codec", type="string", help="Set the codec")
        cp.add_option("-s", "--settings", type="string", help="Set the codec settings (set1:val,set2:val)")
        cp.add_option("-w", "--width", type="int", help="Set the width of the video in pixels (default: 640 px)")
        cp.add_option("-r", "--height", "--rise", type="int", help="Set the height of the video in pixels (default: 480 px")
        cp.add_option("-p", "--port", type="int", help="Set the network port (5020-5030)")
        cp.add_option("-b", "--buffer", type="int", help="Set the latency buffer (in millisec)")
        cp.add_option("-i", "--input", "--source", type="string", help="Set the video source (input).")

        (options, args) = cp.parse_args(data)
                
        if len(args) > 1:
            name = args[1]
            if options.list:
                self.core.settings_stream(self, name, kind)
            elif options.modify:
                self.core.rename_stream(self, name, options.modify, kind)
            elif [opt for opt in options.__dict__.values() if opt]:
                if options.container:
                    self.core.set_stream(self, name, kind, 'container', options.container)
                if options.codec:
                    self.core.set_stream(self, name, kind, 'codec', options.codec)
                if options.settings:
                    self.core.set_stream(self, name, kind, 'codec_settings', options.settings)
                if options.width:
                    self.core.set_stream(self, name, kind, 'width', options.width)
                if options.height:
                    self.core.set_stream(self, name, kind, 'height', options.height)
                if options.port:
                    self.core.set_stream(self, name, kind, 'port', options.port)
                if options.buffer:
                    self.core.set_stream(self, name, kind, 'buffer', options.buffer)
                if options.input:
                    self.core.set_stream(self, name, kind, 'input', options.input)
        elif options.list:
            self.core.list_stream(self, kind)
        elif options.add:
            self.core.add_stream(self, options.add, video.gst.VideoGst(10000, '127.0.0.1', self.factory.subject), kind)
        elif options.erase:
            self.core.delete_stream(self, options.erase, kind)
        else:
            cp.print_help()
            
    def _streams(self, data):
        cp = CliParser(self, prog=data[0], description="Manage the wrapper stream.")
        cp.add_option("-l", "--list", action='store_true', help="List all the streams included")

#        cp.add_option("-a", "--add", type="string", help="Add a stream")
#        cp.add_option("-e", "--erase", type="string", help="Erase a stream")
#        cp.add_option("-m", "--modify", type="string", help="Modify the name of a stream")

        cp.add_option("-c", "--select", "--choose", type="string", help="Select the current stream")
        
        cp.add_option("-s", "--start", type="string", help="Start a stream of playing")
        cp.add_option("-i", "--stop", "--interrupt", action='store_true', help="Stop a stream of playing")

        cp.add_option("-t", "--container", "--tank", "--type", type="string", help="Set the container")
        cp.add_option("-p", "--port", type="int", help="Set the network port (5020-5030)")
        cp.add_option("-o", "--send", "--out", action='store_true', dest="mode", help="Set the mode to 'send'")
        cp.add_option("-r", "--receive", "--in", action='store_false', dest="mode", help="Set the mode to 'receive'")

        (options, args) = cp.parse_args(data)
        
        kind = 'streams'

        if options.list:
            if len(args) > 1:
                self.core.list_streams(self)
            else:
                self.core.list_stream(self, kind)
        elif options.stop:
            self.core.stop_streams(self)
        elif options.select:
            self.core.select_streams(self, options.select)
        elif [opt for opt in options.__dict__.values() if opt != None]:
            if options.container:
                self.core.set_streams(self, 'container', options.container)
            if options.port:
                self.core.set_streams(self, 'port', options.port)
            if options.mode == True:
                self.core.set_streams(self, 'mode', 'send')
            elif options.mode == False:
                self.core.set_streams(self, 'mode', 'receive')
            if options.start:
                self.core.start_streams(self, options.start)
        else:
            cp.print_help()
        
    def _join(self, data):
        cp = CliParser(self, prog=data[0], description="Start and stop a connection.")
        cp.add_option("-s", "--start", action='store_true', help="Start a connection with the default contact")
        cp.add_option("-i", "--stop", "--interrupt", action='store_true', help="Stop the connection")

        (options, args) = cp.parse_args(data)
                
        if options.start:
            self.core.start_connection(self)
        elif options.stop:
            self.core.stop_connection(self)
        else:
            cp.print_help()
        


class CliParser(optparse.OptionParser):
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
        help text provided with them, to 'file' (default stdout).
        """
        self.output.write(self.format_help(), True)

    def get_prog_name(self):
        return self.prog

    def _process_short_opts(self, rargs, values):
        arg = rargs.pop(0)
        stop = False
        i = 1
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
    """ """
    
    def __init__(self, subject, controller):
        Observer.__init__(self, subject)
        self.controller = controller
        self.callbacks = find_callbacks(self)
                
    def update(self, origin, key, data):
        if key in self.callbacks:
            self.callbacks[key](origin, data)
        
    def write(self, msg, prompt=True):
        self.controller.write(msg)
        if prompt:
            self.controller.write_prompt()

    def _get_contacts(self, origin, data):
        if origin is self.controller:
            msg = []
            for contact in data:
                if contact != '_selected':
                    contact = data[contact]
                    if contact.name == data['_selected']:
                        msg.append(bold(contact.name + ": <---"))
                    else:  
                        msg.append(contact.name + ":")  
                    msg.append("\t" + contact.address)
                    if contact.port:
                        msg.append("\t%s" % contact.port)
            msg_out = "\n".join(msg)  
            self.write(msg_out)
            
    def _add_contact(self, origin, data):
        if origin is self.controller:
            if data:
                self.write('Contact added')
            else:
                self.write('Could not add contact because this name is already taken.')

    def _delete_contact(self, origin, data):
        if origin is self.controller:
            if data:
                self.write('Contact deleted')
            else:
                self.write('Could not delete contact because this name doesn\'t exist.')

    def _modify_contact(self, origin, data):
        if origin is self.controller:
            if data:
                self.write('Contact modified')
            else:
                self.write('Could not modify this contact because this name doesn\'t exist.')

    def _select_contact(self, origin, data):
        if origin is self.controller:
            if data:
                self.write('Contact selected')
            else:
                self.write('Could not select this contact because this name doesn\'t exist.')

    def _audio_set(self, origin, ((state, attr, value), name)):
        if origin is self.controller:
            if state:
                self.write('%s of audio stream %s is set to %s.' % (attr, name, value))
            else:
                self.write('Unable to set %s of audio stream %s.' % (attr, name))
                
    def _not_found(self, origin, name, kind):
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
                self.write("".join(msg))
            else:
                self._not_found(origin, name, 'audio')
            
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
                self._not_found(origin, name, 'audio')
            else:
                self.write('Unable to delete the audio stream %s.' % name)
        
    def _audio_rename(self, origin, (data, name, new_name)):
        if origin is self.controller:
            if data == 1:
                self.write('Audio stream %s rename to %s.' % (name, new_name))
            elif data == 0:
                self._not_found(origin, name, 'audio')
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
                self.write("".join(msg))
            else:
                self._not_found(origin, name, 'video')
            
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
                self._not_found(origin, name, 'video')
            else:
                self.write('Unable to delete the video stream %s.' % name)
        
    def _video_rename(self, origin, (data, name, new_name)):
        if origin is self.controller:
            if data == 1:
                self.write('Video stream %s rename to %s.' % (name, new_name))
            elif data == 0:
                self._not_found(origin, name, 'video')
            else:
                self.write('Unable to rename the video stream %s.' % name)
        
                
    def _video_list(self, origin, data):
        if origin is self.controller:
            if data:
                names = [stream[0] for stream in data]
                self.write("\n".join(names))
            else:
                self.write('There is no video stream.')
                    
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
                    if isinstance(stream[1], AudioStream):
                        kind = 'audio'
                    elif isinstance(stream[1], VideoStream):
                        kind = 'video'
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
        self.write('\n%s is inviting you. Do you accept?\n[Y/n]: ' % data[0].host, False)
        self.controller.block = self.controller._ask
        self.controller.remote = data[1]
        
    def _start_connection(self, origin, data):
        self.write(data[0], False)
            
    def _info(self, origin, data):
        self.write(data)
 
    def _connectionMade(self, origin, data):
        self.write(data[0])

def bold(msg):
    return "%s[1m%s%s[0m" % (ESC, msg, ESC)
        

class Test(telnet.Telnet):
    
    def __init__(self):
        telnet.Telnet.__init__(self)
        self.prompt = "pof: "
        self.history = []
        self.accept_options = (telnet.LINEMODE,
                               telnet.ECHO,
                               telnet.EC)
        self.buffer = []
        self.pos = 0
        self.max_history = 10
        
    def write(self, msg, prompt=False):
        self.transport.write(msg)
        if prompt:
            self.write_prompt()
        
    def applicationDataReceived(self, data):
        print repr(data)
        if data == "\r":
            cmd = ''.join(self.buffer)
            self.buffer = []
            self.upd_history(cmd)
            self.pos = 0
            if cmd.lower() in ("exit", "quit"):
                log.info("Telnet client at %s:%s has disconnected" % (self.addr.host, self.addr.port))
                self.write("\r\nGood Bye\r\n")
                self.transport.loseConnection()
            else:
                self.write_prompt()
        elif data == ESC + '[A':
            self.pos -= 1
            if self.pos < len(self.history) * -1:
                self.pos += 1
                self.write(telnet.BEL)
            else:
                cmd = self.history[self.pos]
                self.buffer = [cmd]
                self.write('\r' + self.prompt + cmd)
        elif data == ESC + '[B':
            self.pos += 1
            if self.pos == 0:
                self.buffer = []
                self.write('\r' + self.prompt)
            elif self.pos > 0:
                self.pos -= 1
                self.write(telnet.BEL)
            else:
                cmd = self.history[self.pos]
                self.buffer = [cmd]
                self.write('\r' + self.prompt + cmd)
        elif data == chr(127):
            self.will(telnet.EL)
#            self.write('\x08')
            print "BACKSPACE"
        else:
            self.buffer.append(data)
            self.write(data)
    
    def upd_history(self, cmd):
        self.history.append(cmd)
        if len(self.history) > self.max_history:
            del self.history[0]
        
            
    def parse(self, data):
        """Method to be overidden when subclassing."""
        self.write("command not found")
        self.write_prompt()            
            
    def write_prompt(self):
        self.transport.write("\r\n" + self.prompt)
    
    def connectionMade(self):
        log.info("%s is connecting in Telnet on port %s" % (self.addr.host, self.addr.port))
        self.write("Welcome to Sropulpof!", True)
        self.do(telnet.LINEMODE)
        self.will(telnet.ECHO)
        
    def enableRemote(self, option):
        if option in self.accept_options:
            return True
        return False


class CliFactory(protocol.ServerFactory):
    
    view = None
    subject = None
    
    def buildProtocol(self, addr):
        """Create an instance of a subclass of Protocol.

        The returned instance will handle input on an incoming server
        connection, and an attribute \"factory\" pointing to the creating
        factory.

        @param addr: an object implementing L{twisted.internet.interfaces.IAddress}
        """
        p = self.protocol()
        p.factory = self
        p.addr = addr
        p.view = self.view(self.subject, p)
        return p


def start(subject, port=14444):
    """This runs the protocol on port 14444"""
    factory = CliFactory()
    factory.protocol = CliController
    factory.subject = subject
    factory.view = CliView
    reactor.listenTCP(port, factory)


# this only runs if the module was *not* imported
if __name__ == '__main__':
    from utils import Subject
    from utils import log as logging
    logging.start()
    start(Subject())
    reactor.run()
