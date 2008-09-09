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


# Twisted imports
from twisted.internet import reactor
from twisted.internet import protocol
from twisted.protocols.basic import LineReceiver

# App imports
from utils import log

log = log.start('debug', 1, 0, 'basic')



class Basic(LineReceiver):
    def __init__(self):
        self.core = None

    def lineReceived(self, line):
        if not self.core:
            self.core = self.factory.subject
        if line == "ASK":
            self.core.notify(None, self.addr, 'ask')
            print "ask"
#        elif line == "STOP":
#        elif line == "ACCEPT":
#        elif line == "REFUSE":
        else:
            log.info('Bad command receive from remote')
            
    def connectionMade(self):
        log.info('Connection made to the server.')
    
    def connectionLost(self, reason=protocol.connectionDone):
        log.info('Lost the server connection.')


    def connect(self, address, port):
        """function connect
        
        address: string
        port: int
        
        returns 
        """
        return None # should raise NotImplementedError()
    
    def disconnect(self):
        """function disconnect
        
        returns 
        """
        return None # should raise NotImplementedError()
    
    def accept(self):
        """function accept
        
        returns 
        """
        return None # should raise NotImplementedError()
    
    def refuse(self, reason):
        """function refuse
        
        reason: 
        
        returns 
        """
        return None # should raise NotImplementedError()


class BasicFactory(protocol.ServerFactory):
    
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
        p.subject = self.subject
        return p
    
def start(port, subject):
    factory = BasicFactory()
    factory.protocol = Basic
    factory.subject = subject
    reactor.listenTCP(port, factory)

    