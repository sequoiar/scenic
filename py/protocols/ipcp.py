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

import re
from types import NoneType, FunctionType, InstanceType

# twisted imports
from twisted.internet import reactor, protocol, defer
from twisted.protocols.basic import LineReceiver

# App imports
from utils import log

log = log.start('debug', 1, 0, 'ipcp')


class IPCP(LineReceiver):
    def __init__(self):
        self.r = re.compile(r'("([^"\\]|\\.)*"|[^ ]+)')
        self.callbacks = {}

    def add_callback(self, cmd, name=None):
        if not name:
            name = cmd.im_class.__name__ + '/' + cmd.__name__
        if name not in self.callbacks:
            self.callbacks[name] = cmd
        else:
            log.debug('A callback with this name %s is already registered.' % name)
        log.debug('Callback list: ' + repr(self.callbacks))
            
    def del_callback(self, name):
        if isinstance(name, InstanceType):
            name = cmd.im_class.__name__ + '/' + cmd.__name__
        if name in self.callbacks:
            del self.callbacks[name]
        else:
            log.debug('Cannot delete the callback %s because is not registered.' % name)
        log.debug('Callback list: ' + repr(self.callbacks))
              
    def connectionMade(self):
        log.info('Connection made to the server.')
    
    def lineReceived(self, line):
        log.debug("Line received: %s" % line)
        cmd, sep, args = line.partition(':')
#        tokens = self.r.findall(line)
#        cmd = tokens[0][0][:-1]
        if cmd not in self.callbacks:
            log.info('Command %s not in callback list.' % cmd)
        else:
            data = args.strip() + " "   #TODO: use the parse function instead
            args = {}
            attr_s = 0
            start = 0
            while True:
                end = data.find('=', start)
                if end == -1:
                    break
                attr = data[start:end]
                start = end + 1
                is_string = False
                if data[start] == '"':
                    is_string = True
                    start += 1
                    temp_s = start
                    out = False
                    while True:
                        temp_e = data.find('"', temp_s)
                        if temp_e == -1:
                            out = True
                            break
                        if data[temp_e - 1] != "\\":
                            end = temp_e
                            break
                        temp_s = temp_e + 1
                    if out:
                        break
                    
                else:
                    end = data.find(' ', start)
                    if end == -1:
                        break
                value = data[start:end]
                if is_string:
                    value = value.replace('\\\\', '\\') \
                                 .replace('\\=', '=') \
                                 .replace('\\"', '"')

                else:
                    if value.isdigit():
                        value = int(value)
                    else:
                        try:
                            value = float(value)
                        except:
                            log.info('Invalid type for received argument %s=%s.' % (attr, value))
                args[attr] = value
                start = end + 1
            log.debug("Received: " + cmd + repr(args))
            self.callbacks[cmd](**args)
    
    def send_cmd(self, cmd, *args):
        line = []
        line.append(cmd + ":")
        for arg in args:
            if isinstance(arg, tuple):
                parg = self._process_arg(arg[1])
                if parg:
                    line.append(arg[0] + '=' + parg)
            else:
                parg = self._process_arg(arg)
                if parg:
                    line.append(parg)
        line = ' '.join(line)
        self.sendLine(line)
        log.debug('Sending: ' + line)

    def _process_arg(self, arg):
        if isinstance(arg, int) or isinstance(arg, float):
            return str(arg)
        elif isinstance(arg, unicode):
            return '"%s"' % arg.encode('ascii', 'backslashreplace') \
                                    .replace('\\', '\\\\') \
                                    .replace("'", "\\'") \
                                    .replace('"', '\\"')
        elif isinstance(arg, str):
            return '"%s"' % arg.replace('\\', '\\\\') \
                                    .replace("'", "\\'") \
                                    .replace('"', '\\"')
        elif not isinstance(arg, NoneType):
            log.debug('Invalid type (%s) for argument %s to send.' % (type(arg), arg))
        return None

    def connectionLost(self, reason=protocol.connectionDone):
        log.info('Lost the server connection.')


def parse(args):
    data = args.strip() + " "
    args = {}
    attr_s = 0
    start = 0
    while True:
        end = data.find('=', start)
        if end == -1:
            break
        attr = data[start:end]
        start = end + 1
        is_string = False
        if data[start] == '"':
            is_string = True
            start += 1
            temp_s = start
            out = False
            while True:
                temp_e = data.find('"', temp_s)
                if temp_e == -1:
                    out = True
                    break
                if data[temp_e - 1] != "\\":
                    end = temp_e
                    break
                temp_s = temp_e + 1
            if out:
                break
            
        else:
            end = data.find(' ', start)
            if end == -1:
                break
        value = data[start:end]
        if is_string:
            value = value.replace('\\\\', '\\') \
                         .replace('\\=', '=') \
                         .replace('\\"', '"')

        else:
            if value.isdigit():
                value = int(value)
            else:
                try:
                    value = float(value)
                except:
                    log.info('Invalid type for received argument %s=%s.' % (attr, value))
        args[attr] = value
        start = end + 1
    return args

def find_equal(data):
    data = data.strip() + " "
    args = {}
    attr_s = 0
    start = 0
    while True:
        try:
            end = data.index('=', start)
        except:
            break
        attr = data[start:end]
        start = end + 1
        if data[start] == '"':
            try:
                temp_s = start + 1
                while True:
                    temp_e = data.index('"', temp_s)
                    if data[temp_e - 1] != "\\":
                        end = temp_e + 1
                        break
                    else:
                        temp_s = temp_e + 1
            except:
                break
            
        else:
            try:
                end = data.index(' ', start)
            except:
                break
        args[attr] = data[start:end]
        start = end + 1
    return args
        
      
def connect(addr, port, timeout=2, bindAddress=None):
    client_creator = protocol.ClientCreator(reactor, IPCP)
    deferred = client_creator.connectTCP(addr, port, timeout, bindAddress)
    return deferred

def connection_failed(protocol):
    log.warning("Connection failed! %s" % protocol.getErrorMessage())        
        


def grrr(test):
    print test
    print "yes!"

def test(arg1, arg2, arg3):
    print "perte"
    print arg1, arg2, arg3

# When connected, send a line
def connectionReady(protocol):
    protocol.add_callback('You', test)
    protocol.connectionLost = grrr
    protocol.sendLine('Hey there')
    protocol.send_cmd('test', 23.3, 34, 'gros bouton', 1)
    reactor.callLater(10, protocol.sendLine, 'Coco')
    reactor.callLater(20, reactor.stop)


if __name__ == "__main__":
    # Client example
    # Create creator and connect
    deferred = connect('127.0.0.1', 22222)
    deferred.addCallback(connectionReady)
    deferred.addErrback(connection_failed)
    reactor.run()
