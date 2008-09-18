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
#from twisted.spread import jelly

# App imports
from utils import log
from connections import com_chan
import streams.stream

log = log.start('info', 1, 0, 'basic')



class BasicServer(LineReceiver):
    def __init__(self):
        self.notify = None
        self.api = None
        self.chan = None

    def lineReceived(self, line):
        log.debug('Line received from %s:%s: %s' % (self.addr.host, self.addr.port, line))
        if line == "ASK":
            self.notify(self, (self.addr, self), 'ask')
#        elif line == "STOP":
#        elif line == "ACCEPT":
#        elif line == "REFUSE":
        else:
            log.info('Bad command receive from %s:%s.' % self.transport.client)
            
    def connectionMade(self):
        if not self.notify:
            self.notify = self.factory.notify
        log.info('Client connected from %s:%s.' % self.transport.client)
    
    def connectionLost(self, reason=protocol.connectionDone):
        log.info('Client %s:%s disconnected. Reason: %s' % (self.transport.client[0], self.transport.client[1], reason.value))

    def refuse(self):
        self.sendLine('REFUSE')
        self.transport.loseConnection()
        
    def accept(self):
        self.chan = com_chan.listen(78347)
        self.chan.add(self.settings)
        self.sendLine('ACCEPT')
        self.transport.loseConnection()
        
    def settings(self, data):
        print data
        self.api.select_streams(self, 'receive')
        for kind, stream in data.items():
            name = stream.pop('name') + '.rem'
            engine = stream.pop('engine')
            self.api.add_stream(self, name, kind, engine)
            for attr, value in stream.items():
                print attr, value
                self.api.set_stream(self, name, kind, attr, value)
        


class BasicClient(LineReceiver):
    def __init__(self):
        self.notify = None
        self.api = None
        self.chan = None
        
    def lineReceived(self, line):
        log.debug('Line received from %s:%s: %s' % (self.addr.host, self.addr.port, line))
        if line == "ASK":
            self.notify(self, self.addr, 'ask')
        #        elif line == "STOP":
        elif line == "ACCEPT":
            self.notify(self, '\nThe invitation to %s:%s was accepted.' % (self.addr.host, self.addr.port), 'info')
            self.chan = com_chan.connect(self.addr.host, 78347)
            self.send_settings()
            
        elif line == "REFUSE":
            self.notify(self, '\nThe invitation to %s:%s was refuse.' % (self.addr.host, self.addr.port), 'info')
        else:
            log.info('Bad command receive from remote')

    def send_settings(self):
        if not self.chan.remote:
            reactor.callLater(0.000001, self.send_settings)
        else:
            settings = {}
            for name, stream in self.api.streams.streams.items():
                kind = self.api.streams.get_kind(stream)
                if not kind:
                    continue
                
                engine = stream.__module__.rpartition('.')[2]
                params = {'name':name.partition('_')[2], 'engine':engine}
                for key, value in stream.__dict__.items():
                    if (key[0] != '_' and value):
                        params[key] = value
                        
                settings[kind] = params
                    
            self.chan.callRemote('BasicServer.settings', settings)
            
    def connectionMade(self):
        if not self.api:
            self.api = self.factory.api
            self.notify = self.factory.notify
        log.info('Connection made to %s:%s.' % self.transport.addr)
        self.sendLine('ASK')
        self.notify(self, ('Connection made to %s:%s.' % self.transport.addr, self.transport.addr))
    
    def connectionLost(self, reason=protocol.connectionDone):
        log.debug('Lost the server connection. Reason: %s' % reason.value)


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





class BasicServerFactory(protocol.ServerFactory):
    
    notify = None
    protocol = BasicServer
    
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
        p.api = self.api
        p.notify = self.notify
        return p


class BasicClientFactory(protocol.ClientFactory):

    api = None
    notify = None
    protocol = BasicClient

    def startedConnecting(self, connector):
        log.info('Started to connect.')
    
    def buildProtocol(self, addr):
        log.debug('Connected to %s.' % addr)
        p = self.protocol()
        p.factory = self
        p.addr = addr
        p.api = self.api
        p.notify = self.notify
        return p
    
    def clientConnectionLost(self, connector, reason):
        log.info('Lost the server connection. Reason: %s' % reason.value)
        self.notify(self, '\nLost the connection. Reason: %s' % reason.value, 'connection_msg')
    
    def clientConnectionFailed(self, connector, reason):
        log.info('Connection failed. Reason: %s' % reason.value)
        self.notify(self, 'Connection failed. Reason: %s' % reason.value, 'connection_msg')
        


def connect(api, host, port):
    factory = BasicClientFactory()
    factory.api = api
    factory.notify = api.notify
    conn = reactor.connectTCP(host, port, factory, timeout=2)
    return conn
    
def start(api, port=2222):
    factory = BasicServerFactory()
    factory.api = api
    factory.notify = api.notify
    reactor.listenTCP(port, factory)


if __name__ == '__main__':
    import sys
    
    if len(sys.argv) > 1:
        transport = connect(sys.argv[1], 2222)
        reactor.callLater(1, transport.write, 'Allo\r\n')
    else:
        start(None, 2223)
        
    reactor.run()
    