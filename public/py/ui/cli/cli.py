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
            self.write_prompt()
            
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
        # build a dict of all semi-private methods
        self.callbacks = find_callbacks(self)
        self.shortcuts = {'c': 'contacts',
                          'a': 'audio'
                          }
        
    def parse(self, data):
        self.core = self.factory.subject.api
        data = to_utf(data)
        data = data.split()
        cmd = data[0]
        if len(cmd) == 1:
            cmd = self.shortcuts[cmd]
#        del data[0]
        if cmd in self.callbacks:
            self.callbacks[cmd](data)
        else:
            self.write("command not found")
            self.write_prompt()
            
    def connectionLost(self, reason=protocol.connectionDone):
        del self.view.callbacks # delete this first to remove the reference to self
        del self.view

    def _contacts(self, data):
        cp = CliParser(self, prog=data[0], description="Manage the address book.")
        cp.add_option("-l", "--list", action='store_true', help="List all the contacts")
        cp.add_option("-a", "--add", type="string", help="Add a contact")
        cp.add_option("-d", "--delete", type="string", help="Delete a contact")
        cp.add_option("-m", "--modify", type="string", help="Modify a contact")
        cp.add_option("-s", "--select", help="Select a contact")
        
        (options, args) = cp.parse_args(data)
        
        if options.list:
            self.core.get_contacts(self)
        elif options.add:
            if len(args) > 1:
                self.core.add_contact(self, options.add, args[1])
            else:
                self.write('You need to give an address.', True)
        elif options.delete:
            self.core.delete_contact(self, options.delete)
        elif options.modify:
            if len(args) > 2:
                self.core.modify_contact(self, options.modify, args[1], args[2])
            else:
                self.write('You need to give the current name, the new name and the address.', True)
        elif options.select:
            self.core.select_contact(self, options.select)
        else:
            cp.print_help()
            
    def _audio(self, data):
        cp = CliParser(self, prog=data[0], description="Manage the audio stream.")
        cp.add_option("-l", "--list", action='store_true', help="List all the audio settings")
        cp.add_option("-t", "--container", "--tank", "--type", type="string", help="Set/get the container")
        cp.add_option("-c", "--codec", type="string", help="Set/get the codec")
        cp.add_option("-s", "--settings", type="string", help="Set/get the codec settings (set1:val,set2:val)")
        cp.add_option("-d", "--bitdepth", type="int", help="Set/get the bitdepth of the audio (default: 16 bit)")
        cp.add_option("-r", "--samplerate", type="int", help="Set/get the samplerate of the audio (default: 48000 Hz")
        cp.add_option("-v", "--channels", "--voices", type="int", help="Set/get the number of audio channels (from 1 to 8)")
        cp.add_option("-p", "--port", type="int", help="Set/get the network port (5020-5030)")
        cp.add_option("-b", "--buffer", type="int", help="Set/get the latency buffer (in millisec)")

        (options, args) = cp.parse_args(data)
        
        if options.list:
            self.core.audio_status(self)
        elif [opt for opt in options.__dict__.values() if opt]:
            if options.container:
                self.core.audio_container(self, options.container)
            if options.codec:
                self.core.audio_codec(self, options.codec)
            if options.bitdepth:
                self.core.audio_bitdepth(self, options.bitdepth)
            if options.samplerate:
                self.core.audio_samplerate(self, options.samplerate)
            if options.channels:
                self.core.audio_channels(self, options.channels)
            if options.port:
                self.core.audio_port(self, options.port)
            if options.buffer:
                self.core.audio_buffer(self, options.buffer)
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
        
    def write(self, msg):
        self.controller.write(msg)
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


def start(subject):
    """This runs the protocol on port 14444"""
    factory = CliFactory()
    factory.protocol = CliController
    factory.subject = subject
    factory.view = CliView
    reactor.listenTCP(14444, factory)


# this only runs if the module was *not* imported
if __name__ == '__main__':
    from utils import Subject
    from utils import log as logging
    logging.start()
    start(Subject())
    reactor.run()
