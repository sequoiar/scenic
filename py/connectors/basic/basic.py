# -*- coding: utf-8 -*-
"""
The BasicServer and BasicClient protocols are used to negociate the connection 
between 2 running miville software. (with a contact)

It is the session initialization routine. It negociates the settings for the streams. 

The client takes the initiative. (ASK) The server receives it and responds with ACCEPT or REFUSE.

The connector.basic package is good an example of a simple way to communicate between two miville software. 
"""
# Sropulpof
# # Copyright (C) 2008 Société des arts technoligiques (SAT)
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
import sys
import traceback # TODO remove

# Twisted imports
from twisted.internet import reactor, protocol, error
from twisted.protocols.basic import LineReceiver
#from twisted.spread import jelly

# App imports
from utils import log
from protocols import com_chan
import streams.stream
from connectors import Connection
from connectors.states import *
from errors import ConnectionError

def track(msg):
    print ""
    print "###########", msg, "############"
    traceback.print_stack()

log = log.start('debug', 1, 0, 'basic')

PORT = 2222

# SERVER STATES
IDLE = 0
WAITING = 1

class BasicServer(LineReceiver):
    """
    The session initialization protocol for miville. - server side
    """
    def __init__(self):
        self.api = None
        self.state = IDLE
        self.client_port = None
        self.client_name = None

    def lineReceived(self, line):
        log.debug('Line received from %s:%s: %s' % (self.addr.host, self.addr.port, line))
        if line[0:3] == "ASK":
            self.set_port(line)
            self.state = WAITING
            self.api.notify(self, (self.addr.host, self), 'ask')
        elif line[0:4] == "STOP":
            self.api.stop_streams(self)
            self.set_port(line)
            contact = self.api.find_contact(self.addr.host, self.client_port, 'basic')
            self.transport.loseConnection()
            if contact:
                contact.connection.cleanup()
                if contact.auto_created:
                    self.api.delete_contact(self, contact.name)
#            self.state = IDLE
            self.api.notify(self, 'Connection was stop by the other side (%s)' % self.addr.host, 'answer')
        else:
            log.info('Bad command receive from %s.' % self.addr.host)

    def set_port(self, line):
        atoms = line.split()
        if len(atoms) > 1 and atoms[1].isdigit():
            self.client_port = int(atoms[1])

    def connectionMade(self):
        if not self.api:
            self.api = self.factory.api
        self.client_name = '%s:%s' % (self.addr.host, self.client_port)
        log.info('Client connected from %s:%s.' % (self.addr.host, self.addr.port))

    def connectionLost(self, reason=protocol.connectionDone):
        if self.state == WAITING:
            self.api.notify(self, 'You didn\'t answer soon enough. Connection close.', 'ask_timeout')
#            self.state = IDLE
        log.info('Client %s:%s disconnected. Reason: %s' % (self.addr.host, self.addr.port, reason.value))

    def refuse(self):
        self.state = IDLE
        self.sendLine('REFUSE')
        self.transport.loseConnection()

    def accept(self):
        self.state = IDLE
        contact = self.api.client_contact(self.addr.host, self.client_port)
        contact.connection = ConnectionBasic(contact, self.api)
        contact.state = CONNECTING
        self.sendLine('ACCEPT')
        self.transport.loseConnection()



class BasicServerFactory(protocol.ServerFactory):

    protocol = BasicServer

    def buildProtocol(self, addr):
        """
        Create an instance of a subclass of Protocol.

        The returned instance will handle input on an incoming server
        connection, and an attribute \"factory\" pointing to the creating
        factory.

        :param addr: an object implementing twisted.internet.interfaces.IAddress
        """
        p = self.protocol()
        p.factory = self
        p.addr = addr
        p.api = self.api
        return p


class BasicClient(LineReceiver):
    """
    The session initialization protocol for miville. - client side
    """
    def __init__(self):
        self.callbacks = {}

    def add_callback(self, name, cmd):
        if name not in self.callbacks:
            self.callbacks[name] = cmd
        else:
            log.debug('A callback with this name %s is already registered.' % name)
        log.debug('Callback list: ' + repr(self.callbacks))

    def del_callback(self, name):
        if name in self.callbacks:
            del self.callbacks[name]
        else:
            log.debug('Cannot delete the callback %s because is not registered.' % name)
        log.debug('Callback list: ' + repr(self.callbacks))

    def lineReceived(self, line):
        log.debug('Line received from %s:%s: %s' % (self.transport.realAddress[0], self.transport.realAddress[1], line))

        if line in self.callbacks:
            self.callbacks[line]()
        else:
            log.info('Bad command receive from remote')


class ConnectionBasic(Connection):
    """
    Connection that uses the "connectors.basic" protocol.
    """
    def __init__(self, contact, api):
        Connection.__init__(self, contact, api)
        self._state = DISCONNECTED
        self._timeout = None

    def _create_connection(self):
        #track("create connection") # TODO
        self._state = CONNECTING
        port = self.contact.port
        if port == None:
            port = PORT
        client_creator = protocol.ClientCreator(reactor, BasicClient)
        deferred = client_creator.connectTCP(self.contact.address, port, timeout=2)
        deferred.addCallback(self._connection_ready)
        deferred.addErrback(self._connection_failed)
    
    # Class attribtues, wich are aliases for some method names.
    _start = _create_connection

    _stop = _create_connection
    
    stop_connecting = Connection.stop

    def _connection_ready(self, connection):
        #track("connection_ready") # TODO
        self._state = CONNECTED
        self.connection = connection    # Maybe private ???
        self.connection.connectionLost = self._connection_lost

        if self.contact.state == ASKING:
            self.connection.add_callback('ACCEPT', self.accepted)
            self.connection.add_callback('REFUSE', self.refused)
            self.connection.sendLine('ASK %s' % PORT)
            self._timeout = reactor.callLater(10, self.timeout)
        elif self.contact.state == DISCONNECTING:
            self.connection.sendLine('STOP %s' % PORT)
            self.cleanup()
            if self.contact.auto_created:
                self.api.delete_contact(self, self.contact.name)

    def _connection_failed(self, conn):
        self._state = DISCONNECTED
        log.debug('Connection failed. Address: %s | Port: %s' % (self.contact.address, self.contact.port))
        self.connection_failed(conn.value)

    def _connection_lost(self, reason=protocol.connectionDone):
        self._state = DISCONNECTED
        log.info('Lost the connection. Reason:\n%s' % reason)

    def _close_connection(self):
        try:
            self._timeout.cancel()
        except (error.AlreadyCalled, error.AlreadyCancelled), err:
            log.debug(err)
        self.connection.transport.loseConnection()
        self.connection = None

    def _accepted(self):
        track("accepted") # TODO
        self.local_name = '%s:%s' % (self.connection.transport.getHost().host, PORT)
        self._close_connection()
#        Connection.accepted(self)
#        self.send_settings()

    def com_chan_started_client(self, action="media"):
        track("com_chan_started_client") # TODO
        if action == "media":
            self.send_settings()
        elif action == "network_test":
            self.api.network_test.client_started(self) 

    def com_chan_started_server(self, action="media"):
        if action == "media":
            self.com_chan.add(self.settings)
        elif action == "network_test":
            self.api.network_test.server_started(self) 
    
    def settings(self, settings):
        """
        Receives the settings for the streams.

        :param settings: dict kind => dict "stream" ... whose keys are "name" and "engine"
        """
        self.com_chan.delete(self.settings)
        self.api.select_streams(self, 'receive')
        for kind, stream in settings.items():
            name = stream.pop('name') + '.rem'
            engine = stream.pop('engine')
            self.api.add_stream(self, name, kind, engine)
            for attr, value in stream.items():
                self.api.set_stream(self, name, kind, attr, value)

        self.api.start_streams(self, None, self.com_chan)

    def send_settings(self):
        """
        Sends the settings for the streams. 
        
        Sends each stream set in the API. The setting method in this very 
        same class handles its result on the receiving end.
        """
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

        self.com_chan.callRemote('ConnectionBasic.settings', settings)
        self.api.start_streams(self, self.contact.address, self.com_chan)

    _refused = _close_connection

    def timeout(self):
        self._close_connection()
        Connection.timeout(self)

    def stop_asking(self):
        self._close_connection()
        self.cleanup()
        return 'Connection with %s stopped.' % self.contact.name



def start(api):
    """
    Starts the Basic connector protocol for miville. 
    
    :param api: miville's api
    """
    global PORT
    if len(sys.argv) > 1:
        PORT += 1
    server_factory = BasicServerFactory()
    server_factory.api = api
    reactor.listenTCP(PORT, server_factory)

