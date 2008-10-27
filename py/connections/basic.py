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

log = log.start('debug', 1, 0, 'basic')


PORT = 2222

class BasicServer(LineReceiver):
    def __init__(self):
        self.notify = None
        self.api = None
        self.chan = None
        self.state = 'idle'

    def lineReceived(self, line):
        log.debug('Line received from %s:%s: %s' % (self.addr.host, self.addr.port, line))
        if line == "ASK":
            self.notify(self, (self.addr, self), 'ask')
            self.state = 'waiting'
        elif line == "STOP":
            self.state = 'idle'
            com_chan.disconnect()
#            self.transport.loseConnection()
            self.api.stop_streams(self)
            self.notify(self, 'Connection was stop by the other side (%s:%s)' % self.transport.client, 'info')
#        elif line == "ACCEPT":
#        elif line == "REFUSE":
        else:
            log.info('Bad command receive from %s:%s.' % self.transport.client)
            
    def connectionMade(self):
        if not self.notify:
            self.notify = self.factory.notify
        log.info('Client connected from %s:%s.' % self.transport.client)
    
    def connectionLost(self, reason=protocol.connectionDone):
        if self.state == 'waiting':
            self.notify(self, 'You didn\'t answer soon enough. Connection close.', 'ask_timeout')
            self.state = 'idle'
        log.info('Client %s:%s disconnected. Reason: %s' % (self.transport.client[0], self.transport.client[1], reason.value))

    def refuse(self):
        self.state = 'idle'
        self.sendLine('REFUSE')
        self.transport.loseConnection()
        
    def accept(self):
        self.state = 'idle'
        self.chan = com_chan.listen(37054) #TODO: Set a port by connection
        self.chan.add(self.remote_info)
        self.chan.add(self.settings)
        self.sendLine('ACCEPT')
        self.transport.loseConnection()
        
    def remote_info(self, address, port, connector):
        self.api.set_connection(address, port, connector)
        
    
    def settings(self, data):
        self.api.select_streams(self, 'receive')
        for kind, stream in data.items():
            name = stream.pop('name') + '.rem'
            engine = stream.pop('engine')
            self.api.add_stream(self, name, kind, engine)
            for attr, value in stream.items():
                self.api.set_stream(self, name, kind, attr, value)
                
        self.api.start_streams(self, None, self.chan)
        


class BasicClient(LineReceiver):
    def __init__(self):
        self.notify = None
        self.api = None
        self.chan = None
        self.mode = None
        
    def lineReceived(self, line):
        log.debug('Line received from %s:%s: %s' % (self.addr.host, self.addr.port, line))

        if line == "ASK":
            self.notify(self, self.addr, 'ask')
        #        elif line == "STOP":
        
        elif line == "ACCEPT":
            self.host = self.transport.getHost()
            self.timeout.cancel()
            self.notify(self, '\nThe invitation to %s:%s was accepted.' % (self.addr.host, self.addr.port), 'info')
            self.chan = com_chan.connect(self.addr.host, 37054) #TODO: Set a port by connection
            self.send_settings()
#            self.api.start_streams(self, self.addr.host, self.chan)
            
        elif line == "REFUSE":
            self.notify(self, '\nThe invitation to %s:%s was refuse.' % (self.addr.host, self.addr.port), 'info')

        elif line == "STOP":
            self.notify(self, '\n%s:%s stop the connection.' % (self.addr.host, self.addr.port), 'info')

        else:
            log.info('Bad command receive from remote')

    def send_settings(self):
        if not self.chan.remote:
            reactor.callLater(0.001, self.send_settings)
        else:
            print self.host.host
            self.chan.callRemote('BasicServer.remote_info', self.host.host, PORT, 'ip')
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
            self.api.start_streams(self, self.addr.host, self.chan)
            
    def connectionMade(self):
        if not self.api:
            self.api = self.factory.api
            self.notify = self.factory.notify
            self.mode = self.factory.mode
        log.info('Connection made to %s:%s.' % self.transport.addr)
        if self.mode == 'connect':
            self.sendLine('ASK')
            self.timeout = reactor.callLater(10, self.ask_timeout)
            self.notify(self, ('Connection made to %s:%s.' % self.transport.addr, self.transport.addr))
        elif self.mode == 'disconnect':
            self.sendLine('STOP')
            com_chan.disconnect()
            self.transport.loseConnection()
#            self.api.stop_streams(self)
            self.notify(self, ('Stop info sended to %s:%s.' % self.transport.addr, self.transport.addr))

    def ask_timeout(self):
        self.transport.loseConnection()
        self.notify(self, 'Timeout, the other side didn\'t respond.', 'info')
    
    def connectionLost(self, reason=protocol.connectionDone):
        log.debug('Lost the server connection. Reason: %s' % reason.value)


    def connect(self, address, port):
        """function connect
        
        address: string
        port: int
        
        returns 
        """
        return None # should raise NotImplementedError()
    
    def stop(self):
        """function disconnect
        """
        self.sendLine('STOP')
        com_chan.disconnect()
        self.api.stop_streams(self)
    
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
    mode = None
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
        p.mode = self.mode
        return p
    
    def clientConnectionLost(self, connector, reason):
        log.info('Lost the server connection. Reason: %s' % reason.value)
        self.notify(self, '\nLost the connection. Reason: %s' % reason.value, 'connection_msg')
    
    def clientConnectionFailed(self, connector, reason):
        log.info('Connection failed. Reason: %s' % reason.value)
        self.notify(self, 'Connection failed. Reason: %s' % reason.value, 'connection_msg')
        


def connect(api, host, port):
    return send_cmd(api, host, port, 'connect')

def disconnect(api, host, port):
    return send_cmd(api, host, port, 'disconnect')

def send_cmd(api, host, port, mode):
    factory = BasicClientFactory()
    factory.mode = mode
    factory.api = api
    factory.notify = api.notify
    conn = reactor.connectTCP(host, port, factory, timeout=2)
    return conn
    
def start(api, port=0):
#    port += PORT
    factory = BasicServerFactory()
    factory.api = api
    factory.notify = api.notify
    reactor.listenTCP(PORT, factory)


if __name__ == '__main__':
    import sys
    
    if len(sys.argv) > 1:
        transport = connect(sys.argv[1], 2222)
        reactor.callLater(1, transport.write, 'Allo\r\n')
    else:
        start(None, 2223)
        
    reactor.run()
    