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
from types import InstanceType, FunctionType

# twisted imports
from twisted.internet import reactor, protocol, defer
from twisted.protocols.basic import LineReceiver


class TelnetTester(LineReceiver):
    def __init__(self):
        self.callbacks = {}

    def add_callback(self, cmd, name=None):
        if not name:
            name = cmd.im_class.__name__ + '/' + cmd.__name__
        if name not in self.callbacks:
            self.callbacks[name] = cmd
        else:
            print 'A callback with this name %s is already registered.' % name
            
    def del_callback(self, name):
        if isinstance(name, InstanceType):
            name = cmd.im_class.__name__ + '/' + cmd.__name__
        elif isinstance(name, FunctionType):
            name = cmd.__name__
        if name in self.callbacks:
            del self.callbacks[name]
        else:
            print 'Cannot delete the callback %s because is not registered.' % name
    
    def connectionMade(self):
        print 'Connection made to the server.'
    
    def lineReceived(self, line):
        print line
        
    def connectionLost(self, reason=protocol.connectionDone):
        print 'Lost the server connection.'
        
    def sendLine(self, line):
        LineReceiver.sendLine(self, line)
        
    def write(self, data):
        self.transport.write(data)
    
    def output(self, callback):
        self.lineReceived = callback


def connect(addr, port, timeout=10, bindAddress=None):
    client_creator = protocol.ClientCreator(reactor, TelnetTester)
    deferred = client_creator.connectTCP(addr, port, timeout, bindAddress)
    return deferred

def connection_failed(protocol):
    print "Connection failed! %s" % protocol.getErrorMessage()
    reactor.stop()       

# When connected, send a line
def connection_ready(client):
    client.output(line_received)
    client.sendLine('Hey there')
    reactor.callLater(10, client.sendLine, 'Coco')
    reactor.callLater(20, reactor.stop)

def line_received(line):
    print 'allo'
    print line

if __name__ == "__main__":
    # Client example
    # Create creator and connect
    deferred = connect('127.0.0.1', 8000)
    deferred.addCallback(connection_ready)
    deferred.addErrback(connection_failed)
    reactor.run()
        
        
        
        
        
        
        
        
        
        
        